#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#pragma once
#include <QMainWindow>
// #include "ui_helloqtwindow.h"
#include "ui_mainwindow.h"
#include "mcl_properties.h"
#include "mcl_item.h"
#include "mcl.h"
#include <QGraphicsItem>
#include <QShortcut>
#include <QTimer>
#include <thread>

#include <QMainWindow>
#include <QObject>
#include <QDialog>
#include <QTransform>
// #include <QShortcut>
#include <QCheckBox>
#include <QtCore>
// #include <QGraphicsItem>
#include <QKeyEvent>
#include <QFileDialog>
#include <iostream>
// #include <QTimer>
// #include "mcl_properties.h"
// #include "mcl_item.h"
// #include "mcl.h"
// #include "control.h"
#include <chrono>
// #include <thread>
#include <iostream>
#include <cmath>
#include <opencv2/imgproc.hpp>

namespace Ui {
	class MainWindow;
}

class HelloQtWindow : public QMainWindow
{
Q_OBJECT
public:
	HelloQtWindow(QWidget *parent = 0, Qt::WindowFlags = 0);
	~HelloQtWindow();
    void setPosition(double pos_x, double pos_y);
public slots:
    void onWpressed();
    void onApressed();
    void onSpressed();
    void onDpressed();
    void onQpressed();
    void onEpressed();
	void onButtonPushed();
    void updateRobotPose();
    void setLinePoints(std::vector<QPointF> linePoints);
    void setLineSegment(std::vector<LineSegment> lineSegment);
    void setClsPnts(std::vector<QPointF> clspnts);
    void setObsPnts(std::vector<QPointF> obspnts);
    void setParticles(Particles particles, State mean_estimate);
    void setPose(double x, double y, double w);
signals:
    void updateOdometry(double x, double y, double deg);
    void updatePose(double x, double y, double deg);
    void loadConfig(std::string path);
    void resetPose(double x, double y, double w);
private:
	void connectSignals();
	// Ui::HelloMainWindow ui;
	Ui::MainWindow *ui;

    class Field : public QGraphicsItem
    {
    public:
        Field();
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
        QRectF boundingRect() const;

    private:
        QVector<QRect> boxes;
        QVector<QLineF> lines;
        QVector<QLineF> circle_lines;
        QRectF center_circle;
    };
	private:
    Field *field;
    QGraphicsScene *scene;
    RobotItem *robot_item;
    MCL *mcl;
    QTimer *activeTimer;
    MCLItem *mcl_item;
    // Control *control;

    QPointF pos, prev_pos;
    double angle, prev_angle;
    double pos_x, pos_y, pos_theta;
    bool debug_updateRobotPose;
};

#endif // MAINWINDOW_H
