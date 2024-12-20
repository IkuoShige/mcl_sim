#ifndef FIELD_HPP
#define FIELD_HPP

#pragma once
#include <QGraphicsItem>
#include <QVector>
#include <QRect>
#include <QLineF>
#include <opencv2/opencv.hpp>

class Field : public QGraphicsItem {
public:
    Field();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QRectF boundingRect() const override;
    void convertShapesToPoints(const QVector<QRect> boxes, const QVector<QLineF> lines, std::vector<cv::Point2f> &points);

private:
    QVector<QRect> boxes;
    QVector<QLineF> lines;
    QVector<QLineF> circle_lines;
    QRectF center_circle;
};

#endif // FIELD_HPP