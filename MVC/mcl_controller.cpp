#include "mcl_controller.hpp"
#include "preprocessor.hpp"

MCLController::MCLController(MCLModel *model, MCLViewer *view, Preprocessor *preprocessor, QObject *parent)
    : QObject(parent), model(model), view(view), preprocessor(preprocessor), truePose(0.0, 0.0, 0.0), deltaX(0.0), deltaY(0.0), deltaTheta(0.0) {
    connect(&timer, &QTimer::timeout, this, &MCLController::update);
    connect(&whitelineTimer, &QTimer::timeout, this, &MCLController::updateWhiteline);
    // connect(model, &MCLModel::particlesUpdated, view, &MCLViewer::setWhiteline);
    connect(model, &MCLModel::particlesUpdated, view, &MCLViewer::setParticles);
    connect(&odometryTimer, &QTimer::timeout, this, &MCLController::updateOdometry);
}

void MCLController::setWhiteline(const std::vector<cv::Point2f> &whiteline) {
    this->whiteline = whiteline;
    view->setWhiteline(whiteline);
}

void MCLController::setWlmap(const std::vector<int> &wlmap, int width, int height) {
    this->wlmap = wlmap;
    this->width = width;
    this->height = height;
}

void MCLController::start() {
    model->initialize(100); // パーティクルの初期化
    timer.start(100*5); // 100msごとに更新
    whitelineTimer.start(100); // 1秒ごとに白線の観測データを更新
    odometryTimer.start(100); // 100msごとにオドメトリを更新
}

void MCLController::update() {

    model->update(deltaX, deltaY, deltaTheta);
    model->updateWeights(whiteline, wlmap, width, height);
    model->resample();

    Pose meanParticle = model->calculateMean();
    emit model->particlesUpdated(model->getParticles(), meanParticle); // 修正: ゲッターメソッドを使用
}

void MCLController::updateWhiteline() {
    // 白線の観測データを取得, 更新
    // std::cout << "update whiteline" << std::endl;
    // cv::Mat image = preprocessor->receive_image("127.0.0.1", 5000);

    // 外部からセンサーデータを受け取る場合
    // std::ifstream file("./../data/received_image_data.bin", std::ios::binary);
    // if (!file) {
    //     std::cerr << "Error: Could not read the binary file" << std::endl;
    //     return;
    // }
    // std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(file), {});
    // file.close();
    // cv::Mat image = cv::imdecode(buffer, cv::IMREAD_UNCHANGED);

    // std::vector<cv::Point2f> whiteline = preprocessor->op3_gen_wl(image);
    // Pose meanPose = model->calculateMean();
    // std::cout << "Mean Pose: (" << meanPose.x << ", " << meanPose.y << ", " << meanPose.theta << ")" << std::endl;
    // whiteline = preprocessor->convertWhitelineToFieldCoordinates(whiteline, meanPose);

    Pose meanPose = model->calculateMean();
    preprocessor->load_white_line_points("./points.txt", whiteline); // ダミーデータを読み込む
    // 白線の観測データをシミュレート
    double fov = M_PI / 2; // 視野角90度
    double max_distance = 500.0; // 最大距離1メートル
    whiteline = preprocessor->simulate_white_line(whiteline, meanPose, fov, max_distance);

    // 白線の観測データを更新
    setWhiteline(whiteline);
}

void MCLController::handleKeyPress(int key) {
    // std::cout << "Key pressed: " << key << std::endl;
    double moveStep = 10.0; // 移動ステップ
    double rotateStep = M_PI / 18; // 回転ステップ（10度）

    switch (key) {
        case Qt::Key_W:
            deltaX += moveStep * cos(truePose.theta);
            deltaY += moveStep * sin(truePose.theta);
            break;
        case Qt::Key_S:
            deltaX -= moveStep * cos(truePose.theta);
            deltaY -= moveStep * sin(truePose.theta);
            break;
        case Qt::Key_A:
            deltaTheta -= rotateStep;
            break;
        case Qt::Key_D:
            deltaTheta += rotateStep;
            break;
    }

    // 真値を更新
    truePose.x += deltaX;
    truePose.y += deltaY;
    truePose.theta += deltaTheta;

    // truePoseを更新
    view->setTruePose(truePose);

    // std::cout << "True Pose: (" << truePose.x << ", " << truePose.y << ", " << truePose.theta << ")" << std::endl;
    // std::cout << "Delta: (" << deltaX << ", " << deltaY << ", " << deltaTheta << ")" << std::endl;

}

void MCLController::updateOdometry() {
    // オドメトリの更新を行う
    model->update(deltaX, deltaY, deltaTheta);
    deltaX = 0.0;
    deltaY = 0.0;
    deltaTheta = 0.0;
}
