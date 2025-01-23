#include <iostream>
#include <fstream>
#include <random>
#include <cmath>
#include <algorithm>
#include <vector>
#include <string>
#include <math.h>
#include <tuple>
#include <pybind11/pybind11.h>

typedef std::tuple<double, double, double, double> Particle;

class MCL
{
public:
    MCL(double x, double y, double theta);
    ~MCL();
    double mod_yaw(double yaw);
    void set_particle_num(int num);
    void set_odom_noises(double noise1, double noise2, double noise3, double noise4);
    void set_measurement_variance(double variance);
    void set_measurement_resolution(double resolution);
    void set_measurement_model_coefficients(double z_hit, double z_rand);
    void set_resample_threshold(double threshold);
    void set_plot_sizes(double size_x, double size_y);
    void add_landmark(double x, double y);
    void print_estimated_pose();
    void print_effective_sample_size_and_total_weight();
    double generate_normal_random(double mean, double stddev);
    void initialize_particles(double var_x, double var_y, double var_theta);
    void update_particles(double delta_dist, double delta_theta);
    void calculate_weights(std::vector<std::pair<double, double>> measurements);
    void estimate_robot_pose();
    void resample_particles();
    std::vector<std::pair<double, double>> getMeasurements();
    std::vector<Particle> getParticles();
    std::vector<std::pair<double, double>> getLandmarks();
    double getPlotSizeX();
    double getPlotSizeY();
    double getRobotPoseX();
    double getRobotPoseY();
    double getRobotPoseTheta();
    int getParticleNum();
    double robot_pose_x;
    double robot_pose_y;
    double robot_pose_theta;
    double plot_size_x;
    double plot_size_y;
    std::vector<Particle> particles;
    std::vector<std::pair<double, double>> landmarks;

private:
    double odom_noise1;
    double odom_noise2;
    double odom_noise3;
    double odom_noise4;
    double measurement_variance;
    double measurement_resolution;
    double z_hit;
    double z_rand;
    double resample_threshold;
    int particle_num;
    // Particles particles;
    std::vector<std::pair<double, double>> measurements;

    // std::vector<Particle> landmarks;
    double effective_sample_size;
    double total_weight;
};
