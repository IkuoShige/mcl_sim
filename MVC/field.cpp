#include "field.hpp"
#include <QPainter>
#include "types.hpp"
#include <opencv2/opencv.hpp>

Field::Field()
{
    boxes.push_back(QRect((-FIELD_WIDTH/2), -FIELD_HEIGHT/2, FIELD_WIDTH, FIELD_HEIGHT));
    boxes.push_back(QRect(PENALTY_X1, PENALTY_Y1+100, PENALTY_WIDTH, 300));
    boxes.push_back(QRect(PENALTY_X2, PENALTY_Y2+100, PENALTY_WIDTH, 300));
    boxes.push_back(QRect(PENALTY_X1, PENALTY_Y1, PENALTY_WIDTH*2, PENALTY_HEIGHT));
    boxes.push_back(QRect(PENALTY_X2-100, PENALTY_Y2, PENALTY_WIDTH*2, PENALTY_HEIGHT));
    boxes.push_back(QRect(GOAL_X1, GOAL_Y1, GOAL_WIDTH, GOAL_HEIGHT));
    boxes.push_back(QRect(GOAL_X2, GOAL_Y2, GOAL_WIDTH, GOAL_HEIGHT));
    lines.push_back(QLine(LINE_X1, LINE_Y1, LINE_X2, LINE_Y2));
    lines.push_back(QLine(-10, 0, 10, 0));
    lines.push_back(QLine(-10+300, 0, 10+300, 0));
    lines.push_back(QLine(-10-300, 0, 10-300, 0));
    lines.push_back(QLine(300, -10, 300, 10));
    lines.push_back(QLine(-300, -10, -300, 10));

    // Convert shapes to points
    std::vector<cv::Point2f> points;
    convertShapesToPoints(boxes, lines, points);

    // Save points to file
    std::ofstream file("points.txt");
    for (const auto &point : points) {
        file << point.x << " " << point.y << std::endl;
    }
    file.close();
}

void Field::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // Draw green fields
    painter->setPen(Qt::transparent);
    painter->setBrush(Qt::green);
    painter->drawRect(boundingRect());

    // Draw the lines using rectangles
    painter->setPen(QPen(Qt::white,5));
    painter->setBrush(Qt::transparent);
    painter->drawRects(boxes);
    painter->drawLines(lines);

    //  Draw the center circle
    painter->drawEllipse(QPoint(0,0), 75, 75);

    // Draw the penalty mark
    // Qt::BrushStyle style = Qt::SolidPattern;
    // painter->setBrush(QBrush(Qt::white, style));
    // painter->drawEllipse(QPoint(0,0), 10, 10);
    // painter->drawEllipse(QPoint(-PENALTY_POINT, 0), 5, 5);
    // painter->drawEllipse(QPoint(PENALTY_POINT, 0), 5, 5);

}

QRectF Field::boundingRect() const
{
    return QRectF(-FIELD_WIDTH/2-BORDER,-FIELD_HEIGHT/2-BORDER,FIELD_WIDTH+2*BORDER,FIELD_HEIGHT+2*BORDER);
}

void Field::convertShapesToPoints(const QVector<QRect> boxes, const QVector<QLineF> lines, std::vector<cv::Point2f> &points) {
    // Convert boxes to points
    for (const auto &box : boxes) {
        // Add points for each corner
        points.push_back(cv::Point2f(box.left(), box.top()));
        points.push_back(cv::Point2f(box.right(), box.top()));
        points.push_back(cv::Point2f(box.left(), box.bottom()));
        points.push_back(cv::Point2f(box.right(), box.bottom()));

        // Add points along each edge
        int numEdgePoints = 30; // Number of points to add along each edge
        for (int i = 1; i < numEdgePoints; ++i) {
            float t = static_cast<float>(i) / numEdgePoints;
            points.push_back(cv::Point2f(box.left() + t * (box.right() - box.left()), box.top()));
            points.push_back(cv::Point2f(box.left() + t * (box.right() - box.left()), box.bottom()));
            points.push_back(cv::Point2f(box.left(), box.top() + t * (box.bottom() - box.top())));
            points.push_back(cv::Point2f(box.right(), box.top() + t * (box.bottom() - box.top())));
        }
    }

    // Convert lines to points
    for (const auto &line : lines) {
        // Add points for each end
        points.push_back(cv::Point2f(line.x1(), line.y1()));
        points.push_back(cv::Point2f(line.x2(), line.y2()));

        // Add points along the line
        int numLinePoints = 30; // Number of points to add along the line
        for (int i = 1; i < numLinePoints; ++i) {
            float t = static_cast<float>(i) / numLinePoints;
            points.push_back(cv::Point2f(line.x1() + t * (line.x2() - line.x1()), line.y1() + t * (line.y2() - line.y1())));
        }
    }

    // Add center circle points
    int centerX = 0;
    int centerY = 0;
    int radius = 75;
    int numPoints = 100/2; // Number of points to approximate the circle
    for (int i = 0; i < numPoints; ++i) {
        float angle = 2 * M_PI * i / numPoints;
        float x = centerX + radius * cos(angle);
        float y = centerY + radius * sin(angle);
        points.push_back(cv::Point2f(x, y));
    }
}
