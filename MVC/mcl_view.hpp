#ifndef MCL_VIEW_H
#define MCL_VIEW_H
#pragma once

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <vector>
#include "types.hpp"
#include "item.hpp"
#include "field.hpp"
// #include "MVC/mcl_controller.hpp"

class MCLController;

namespace Ui {
class MainWindow;
}

class MCLViewer : public QMainWindow {
    Q_OBJECT

public:
    explicit MCLViewer(QWidget *parent = nullptr);
    ~MCLViewer();
    void setParticles(const std::vector<Particle> &particles, const Pose &meanParticle);
    void setWhiteline(const std::vector<cv::Point2f> &whiteline);
    void setController(MCLController *controller);
    void setTruePose(const Pose &truePose); // truePoseを更新するメソッドを追加

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    Ui::MainWindow *ui;
    QGraphicsScene *scene;
    Field *field;
    SensorData *sensor_data;
    MCLController *controller;
};

#endif // MCL_VIEW_H