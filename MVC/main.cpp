#include <QApplication>
#include "mcl_model.hpp"
#include "mcl_view.hpp"
#include "mcl_controller.hpp"
#include "preprocessor.hpp"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    MCLModel model;
    MCLViewer view;
    Preprocessor preprocessor;

    MCLController controller(&model, &view, &preprocessor);
    view.setController(&controller);
    view.show();

    // 白線データと地図データの設定
    std::string filename = "./../map/hlfield.pgm";
    std::vector<int> wlmap;
    int width, height;
    preprocessor.loadWhiteLineMap(filename, wlmap, &width, &height);

    controller.setWlmap(wlmap, width, height);

    controller.start();

    return a.exec();
}