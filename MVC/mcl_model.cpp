#include "mcl_model.hpp"

MCLModel::MCLModel(QObject *parent) : QObject(parent) {}

void MCLModel::initialize(int numParticles) {
    initializeParticles(particles, numParticles);
    std::cout << "\033[31mInitialized particles. \033[m" << std::endl;
}

void MCLModel::initializeParticles(std::vector<Particle> &particles, int N_Particle) {
    std::random_device x_rd, y_rd, w_rd;
    double known_x = 0, known_y = 0, known_angle = 0;
    std::normal_distribution<double> x_gen(known_x, 10), y_gen(known_y, 10), w_gen(known_angle, 359);
    std::uniform_real_distribution<double> wr_rgen(0, 2 * M_PI);

    for (int i = 0; i < N_Particle; i++) {
        particles.push_back(Particle(x_gen(x_rd), y_gen(y_rd), wr_rgen(w_rd), 1.0 / N_Particle));
        // std::cout << "Particle: (" << particles[i].x << ", " << particles[i].y << "), orientation: " << particles[i].theta << " degrees, weight: " << particles[i].weight << std::endl;
    }
    // 大域的自己位置推定
    // std::random_device x_rd, y_rd, w_rd;
    // std::uniform_real_distribution<double> x_rgen(-FIELD_WIDTH/2,FIELD_WIDTH/2), y_rgen(-FIELD_HEIGHT/2,FIELD_HEIGHT/2), w_rgen(0,359);
    // std::uniform_real_distribution<double> wr_rgen(0, 2 * M_PI);

    // for(int i = 0; i < N_Particle; i++)
    //     particles.push_back(Particle(x_rgen(x_rd), y_rgen(y_rd), wr_rgen(w_rd), 1/N_Particle));

}

void MCLModel::update(double deltaX, double deltaY, double deltaTheta) {
    updateParticles(particles, deltaX, deltaY, deltaTheta);
}

void MCLModel::updateParticles(std::vector<Particle> &particles, double delta_x, double delta_y, double delta_thta) {
    for (auto &particle : particles) {
        particle.x += delta_x;
        particle.y += delta_y;
        particle.theta += delta_thta;

        if (particle.theta > M_PI) {
            particle.theta -= 2 * M_PI;
        } else if (particle.theta < -M_PI) {
            particle.theta += 2 * M_PI;
        }
    }
}

void MCLModel::updateWeights(const std::vector<cv::Point2f> &whiteline, const std::vector<int> &wlmap, int width, int height) {
    updateweights(particles, whiteline, wlmap, width, height);
}

void MCLModel::updateweights(std::vector<Particle> &particles, const std::vector<cv::Point2f> &whiteline_points, const std::vector<int> &wlmap, int width, int height) {
    for (auto &particle : particles) {
        // tyai::TPos pos(particle.x, particle.y, particle.theta);
        Pose pos(particle.x, particle.y, particle.theta);
        double likelihood_from_whiteline = getLikelihood(pos, whiteline_points);
        double likelihood_from_map = calculateLikelihoodFromMap(whiteline_points, wlmap, width, height, pos);
        particle.weight = likelihood_from_whiteline * likelihood_from_map;
        // double likelihood = getLikelihood(pos, whiteline_points);
        // particle.weight = likelihood;
        // std::cout << "Particle: (" << particle.x << ", " << particle.y << "), orientation: " << particle.theta << " degrees, weight: " << particle.weight << std::endl;
    }
}

double MCLModel::getLikelihood(const Pose &pos, const std::vector<cv::Point2f> &whiteline_points) {
    double likelihood = 0.0;
    for (const auto &point : whiteline_points) {
        // ロボットの位置と白線の位置との距離を計算
        double distance = std::hypot(pos.x - point.x, pos.y - point.y);
        // ロボットの位置と白線の位置との距離が近いほど尤度が高くなるように計算
        likelihood += std::exp(-distance * distance / (2 * 1 * 1));
    }
    // std::cout << "likelihood: " << likelihood << std::endl;
    return likelihood;
}

double MCLModel::calculateLikelihoodFromMap(const std::vector<cv::Point2f> &whiteline_points, const std::vector<int> &wlmap, int width, int height, const Pose &pos) {
    #define MAP_PIX_PER_MILLIMETER 10
    double likelihood = 0.0;
    double c = std::cos(pos.theta);
    double s = std::sin(pos.theta);

    for (const auto &point : whiteline_points) {
        // 観測点の位置を地図の座標系に変換
        // double gposx, gposy;
        // add_relative_pose(s, c, pos.x * 1000, pos.y * 1000, point.x + 270, point.y, gposx, gposy); // 270はオフセット
        double gposx = point.x;
        double gposy = point.y;

        int mapx = static_cast<int>(gposx / MAP_PIX_PER_MILLIMETER + width / 2);
        int mapy = static_cast<int>(gposy / MAP_PIX_PER_MILLIMETER + height / 2);

        // 地図の範囲内か確認
        if (mapx < 0 || mapx >= width || mapy < 0 || mapy >= height) {
            continue; // 範囲外の場合はスキップ
        }

        int map_ptr = mapy * width + mapx;

        // 地図上の値を確認し、尤度を加算
        if (wlmap[map_ptr] > 0) { // 白線が存在する場合
            likelihood += 1.0; // 一致度を加算（適宜スケーリング可能）
        }
    }

    // 尤度を正規化（必要に応じて）
    return likelihood / whiteline_points.size(); // 観測点の数で割る
}

void MCLModel::resample() {
    resampleParticles(particles);
}

void MCLModel::resampleParticles(std::vector<Particle> &particles) {
    // std::vector<std::tuple<double, double, double, double>> new_particles;
    std::vector<Particle> new_particles;
    std::vector<double> weights;
    double total_weight = 0.0;
    for (auto &particle : particles) {
        total_weight += particle.weight;
        weights.push_back(total_weight);
    }
    // std::cout << "total_weight: " << total_weight << std::endl;
    std::random_device rd;
    std::uniform_real_distribution<double> dist(0.0, total_weight);
    double randomValue = dist(rd);
    size_t index = 0;
    std::normal_distribution<double> noise_x(0.0, 1.0); // xに加えるノイズ
    std::normal_distribution<double> noise_y(0.0, 1.0); // yに加えるノイズ
    std::normal_distribution<double> noise_theta(0.0, 0.1); // thetaに加えるノイズ（ラジアン）

    for (size_t i = 0; i < particles.size(); i++) {
        while (weights[index] < randomValue) {
            index++;
        }
        Particle selectedParticle = particles[index];
        double new_x = selectedParticle.x + noise_x(rd);
        double new_y = selectedParticle.y + noise_y(rd);
        double new_theta = selectedParticle.theta + noise_theta(rd);

        new_particles.push_back(Particle(new_x, new_y, new_theta, selectedParticle.weight));
        randomValue += total_weight / particles.size(); // 次のランダム値を生成
    }
    particles = new_particles;
}

Pose MCLModel::calculateMean() const {
    return calculateMeanParticle(particles);
}

Pose MCLModel::calculateMeanParticle(const std::vector<Particle> &particles) const {
    double sum_x = 0.0;
    double sum_y = 0.0;
    double sum_theta = 0.0;

    for (const auto &particle : particles) {
        sum_x += particle.x;
        sum_y += particle.y;
        sum_theta += particle.theta;
    }

    int num_particles = particles.size();
    return Pose(sum_x / num_particles, sum_y / num_particles, sum_theta / num_particles);
}

const std::vector<Particle>& MCLModel::getParticles() const { // 追加: particlesへのゲッターメソッド
    return particles;
}
