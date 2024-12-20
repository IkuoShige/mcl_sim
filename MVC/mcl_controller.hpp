#ifndef MCL_CONTROLLER_H
#define MCL_CONTROLLER_H

#pragma once
#include <QObject>
#include <QTimer>
#include "mcl_model.hpp"
#include "mcl_view.hpp"
#include "preprocessor.hpp"

class MCLViewer;

class MCLController : public QObject {
    Q_OBJECT

public:
    MCLController(MCLModel *model, MCLViewer *view, Preprocessor *preprocessor, QObject *parent = nullptr);
    void setWhiteline(const std::vector<cv::Point2f> &whiteline);
    void setWlmap(const std::vector<int> &wlmap, int width, int height);

public slots:
    void start();
    void update();
    void updateWhiteline();
    void handleKeyPress(int key);
    void updateOdometry();

private:
    MCLModel *model;
    MCLViewer *view;
    Preprocessor *preprocessor;
    QTimer timer;
    QTimer whitelineTimer;
    QTimer odometryTimer;
    std::vector<cv::Point2f> whiteline;
    std::vector<int> wlmap;
    int width;
    int height;
    Pose truePose;
    double deltaX;
    double deltaY;
    double deltaTheta;
};

#endif // MCL_CONTROLLER_H