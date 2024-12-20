#ifndef ITEM_H
#define ITEM_H

#pragma once
#include <QGraphicsItem>
#include <QGraphicsSceneHoverEvent>
#include <QVector>
#include <QObject>
#include <QPainter>
#include <iostream>
#include <QDebug>
#include <opencv2/imgproc.hpp>
#include "types.hpp"

struct Particle {
    double x;
    double y;
    double theta;
    double weight;

    Particle(double x, double y, double theta, double weight) : x(x), y(y), theta(theta), weight(weight) {}
};

struct Pose {
    double x;
    double y;
    double theta;

    Pose() : x(0), y(0), theta(0) {} // デフォルトコンストラクタを追加
    Pose(double x, double y, double theta) : x(x), y(y), theta(theta) {}
};

class SensorData : public QGraphicsItem
{
public:
    SensorData();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QRectF boundingRect() const;
    void setWhiteline(std::vector<cv::Point2f> whiteline_);
    void setParticles(const std::vector<Particle> &particles_, const Pose &meanParticle_);
    void setTruePose(const Pose &truePose_); // truePoseを更新するメソッドを追加

private:
    std::vector<cv::Point2f> whiteline;
    std::vector<Particle> particles;
    Pose meanParticle;
    Pose truePose;
};

#endif // ITEM_H
