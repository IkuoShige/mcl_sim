#ifndef MCL_MODEL_H
#define MCL_MODEL_H

#pragma once
#include <vector>
#include <QObject>
#include "types.hpp"
#include "item.hpp"

class MCLModel : public QObject {
    Q_OBJECT

public:
    MCLModel(QObject *parent = nullptr);
    void initialize(int numParticles);
    void initializeParticles(std::vector<Particle> &particles, int N_Particle);
    void update(double deltaX, double deltaY, double deltaTheta);
    void updateParticles(std::vector<Particle> &particles, double delta_x, double delta_y, double delta_thta);
    void updateWeights(const std::vector<cv::Point2f> &whiteline, const std::vector<int> &wlmap, int width, int height);
    double getLikelihood(const Pose &pos, const std::vector<cv::Point2f> &whiteline_points);
    double calculateLikelihoodFromMap(const std::vector<cv::Point2f> &whiteline_points, const std::vector<int> &wlmap, int width, int height, const Pose &pos);
    void updateweights(std::vector<Particle> &particles, const std::vector<cv::Point2f> &whiteline_points, const std::vector<int> &wlmap, int width, int height);
    void resample();
    void resampleParticles(std::vector<Particle> &particles);
    Pose calculateMean() const;
    Pose calculateMeanParticle(const std::vector<Particle> &particles) const;
    const std::vector<Particle>& getParticles() const; // 追加: particlesへのゲッターメソッド

signals:
    void particlesUpdated(const std::vector<Particle> &particles, const Pose &meanParticle);

private:
    std::vector<Particle> particles;
};

#endif // MCL_MODEL_H