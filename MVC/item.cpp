#include "item.hpp"

SensorData::SensorData() {}

void SensorData::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    painter->setPen(QPen(Qt::red, 5));
    painter->setBrush(Qt::transparent);

    for (const auto &point : whiteline) {
        painter->drawPoint(point.x, point.y);
    }

    
    painter->setBrush(Qt::black);
    for(const auto &particle : particles)
    {

      QRectF particle_(-10, -10, 20, 20);
      double x = particle.x;
      double y = particle.y;
      double w = particle.theta;
      if(abs(x) > (FIELD_WIDTH/2) + GOAL_WIDTH || abs(y) > (FIELD_HEIGHT/2)) continue;
      particle_.translate(x, y);
      double c = cos(w * M_PI/180);
      double s = sin(w * M_PI/180);
      painter->setPen(Qt::black);
      double rot_lx = c*40-s*0;
      double rot_ly = s*40+c*0;
      painter->drawLine(QPointF(x,y), QPointF(x,y) + QPointF(rot_lx, rot_ly));
      painter->drawEllipse(particle_);

    }

    // painter->setPen(QPen(Qt::red));
    painter->setBrush(Qt::red);
    QRectF particle_true_(-10, -10, 20, 20);
    double x = truePose.x;
    double y = truePose.y;
    double w = truePose.theta;
    // std::cout << "True Pose: (" << x << ", " << y << ", " << w << ") on painter" << std::endl;
    particle_true_.translate(x, y);
    double c = cos(w * M_PI/180);
    double s = sin(w * M_PI/180);
    painter->setPen(Qt::red);
    double rot_lx = c*40-s*0;
    double rot_ly = s*40+c*0;
    painter->drawLine(QPointF(x,y), QPointF(x,y) + QPointF(rot_lx, rot_ly));
    painter->drawEllipse(particle_true_);
//     for (const auto &particle : meanParticle) {
//         painter->drawEllipse(QPointF(particle.x, particle.y), 5, 5);
//     }
}

QRectF SensorData::boundingRect() const {
    return QRectF(-FIELD_WIDTH/2-BORDER, -FIELD_HEIGHT/2-BORDER, FIELD_WIDTH+2*BORDER, FIELD_HEIGHT+2*BORDER);
}

void SensorData::setWhiteline(std::vector<cv::Point2f> whiteline_) {
    whiteline = whiteline_;
}

void SensorData::setParticles(const std::vector<Particle> &particles_, const Pose &meanParticle_) {
    particles = particles_;
    meanParticle = meanParticle_;
}

void SensorData::setTruePose(const Pose &truePose_) {
    truePose = truePose_;
}
