#include "mcl_view.hpp"
#include "ui_mainwindow.h"
#include "mcl_controller.hpp"
#include <QKeyEvent>

MCLViewer::MCLViewer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);

    field = new Field();
    sensor_data = new SensorData();

    scene = new QGraphicsScene(this);
    scene->setSceneRect((-FIELD_WIDTH/2)-BORDER, (-FIELD_HEIGHT/2)-BORDER, FIELD_WIDTH + (2*BORDER), FIELD_HEIGHT + (2*BORDER));
    scene->addItem(field);
    scene->addItem(sensor_data);

    ui->graphicsView->setScene(scene);
    ui->graphicsView->scale(0.5, 0.5);
}

MCLViewer::~MCLViewer() {
    delete ui;
}

void MCLViewer::setParticles(const std::vector<Particle> &particles, const Pose &meanParticle) {
    sensor_data->setParticles(particles, meanParticle);
    sensor_data->update();
}

void MCLViewer::setWhiteline(const std::vector<cv::Point2f> &whiteline) {
    sensor_data->setWhiteline(whiteline);
    sensor_data->update();
}

void MCLViewer::setController(MCLController *controller) {
    this->controller = controller;
}

void MCLViewer::setTruePose(const Pose &truePose) {
    sensor_data->setTruePose(truePose);
    sensor_data->update();
}

void MCLViewer::keyPressEvent(QKeyEvent *event) {
    if (controller) {
        controller->handleKeyPress(event->key());
    }
}