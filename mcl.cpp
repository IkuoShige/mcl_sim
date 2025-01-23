#include "mcl.h"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

MCL::MCL(double x=0.0, double y=0.0, double theta=0.0)
{
    // Constructor
    this->robot_pose_x = x;
    this->robot_pose_y = y;
    this->robot_pose_theta = theta;

    this->odom_noise1 = 1.0;
    this->odom_noise2 = 0.5;
    this->odom_noise3 = 0.5;
    this->odom_noise4 = 2.0;

    this->measurement_variance = 0.1*0.1;
    this->measurement_resolution = 0.1;
    this->z_hit = 0.9;
    this->resample_threshold = 0.5;

    this->particle_num = 100;
    this->plot_size_x = 5.0;
    this->plot_size_y = 5.0;

    this->effective_sample_size = 0.0;
    this->total_weight = 0.0;
}

MCL::~MCL()
{
    // Destructor
}

double MCL::mod_yaw(double yaw)
{
    // Modify yaw to be in the range of -pi to pi
    while (yaw > M_PI)
    {
        yaw -= 2*M_PI;
    }
    while (yaw < -M_PI)
    {
        yaw += 2*M_PI;
    }
    return yaw;
}

void MCL::set_particle_num(int num)
{
    this->particle_num = num;
}

void MCL::set_odom_noises(double noise1, double noise2, double noise3, double noise4)
{
    this->odom_noise1 = noise1;
    this->odom_noise2 = noise2;
    this->odom_noise3 = noise3;
    this->odom_noise4 = noise4;
}

void MCL::set_measurement_variance(double variance)
{
    this->measurement_variance = variance;
}

void MCL::set_measurement_resolution(double resolution)
{
    this->measurement_resolution = resolution;
}

void MCL::set_measurement_model_coefficients(double z_hit, double z_rand)
{
    this->z_hit = z_hit;
    this->z_rand = z_rand;
    if (this->z_hit + this->z_rand != 1.0)
    {
        std::cout << "Error: z_hit + z_rand must be equal to 1.0" << std::endl;
        exit(-1);
    }
}

void MCL::set_resample_threshold(double threshold)
{
    this->resample_threshold = threshold;
}

void MCL::set_plot_sizes(double plot_size_x, double plot_size_y)
{
    this->plot_size_x = plot_size_x;
    this->plot_size_y = plot_size_y;
}

void MCL::add_landmark(double x, double y)
{
    this->landmarks.push_back(std::make_pair(x, y));
}

void MCL::print_estimated_pose()
{
    std::cout << "Estimated pose: " << robot_pose_x << ", " << robot_pose_y << ", " << robot_pose_theta << std::endl;
}

void MCL::print_effective_sample_size_and_total_weight()
{
    std::cout << "particle num: " << particles.size() << ", effective sample size: " << effective_sample_size << ", total weight = " << total_weight << std::endl;
}

double MCL::generate_normal_random(double mean, double stddev)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<double> dist(mean, stddev);
    return dist(gen);
}

void MCL::initialize_particles(double var_x, double var_y, double var_theta)
{
    particles.clear();
    double wo = 1.0 / static_cast<double>(particle_num);
    for (int i = 0; i < particle_num; i++) {
        double x = generate_normal_random(this->robot_pose_x, var_x);  // normal is a function that generates a normal random number
        double y = generate_normal_random(this->robot_pose_y, var_y);
        double theta = generate_normal_random(robot_pose_theta, var_theta);
        theta = mod_yaw(theta);
        particles.push_back(std::make_tuple(x, y, theta, wo));
    }
}

void MCL::update_particles(double delta_dist, double delta_theta)
{
    double delta_dist2 = delta_dist * delta_dist;
    double delta_theta2 = delta_theta * delta_theta;
    for (int i = 0; i < particles.size(); i++) {
        double del_dist = generate_normal_random(delta_dist, odom_noise1 * delta_dist2 + odom_noise2 * delta_theta2);
        double del_theta = generate_normal_random(delta_theta, odom_noise3 * delta_dist2 + odom_noise4 * delta_theta2);
        double x = std::get<0>(particles[i]);
        double y = std::get<1>(particles[i]);
        double theta = std::get<2>(particles[i]);
        // double w = std::get<3>(particles[i]);

        x = x + del_dist * cos(theta);
        y = y + del_dist * sin(theta);
        theta = mod_yaw(theta + del_theta);

        // x_new += generate_normal_random(0.0, odom_noise1 * delta_dist2 + odom_noise2 * delta_theta2);
        // y_new += generate_normal_random(0.0, odom_noise1 * delta_dist2 + odom_noise2 * delta_theta2);
        // theta_new += generate_normal_random(0.0, odom_noise3 * delta_dist2 + odom_noise4 * delta_theta2);
        // theta_new = mod_yaw(theta_new);

        std::get<0>(particles[i]) = x;
        std::get<1>(particles[i]) = y;
        std::get<2>(particles[i]) = theta;
        // std::cout << "Particle " << i << ": " << x_new << ", " << y_new << ", " << theta_new << ", " << w << std::endl;
    }
}

void MCL::calculate_weights(std::vector<std::pair<double, double>> measurements)
{
    total_weight = 0.0;
    double norm_coef = 1.0 / (sqrt(2.0 * M_PI * measurement_variance));
    for (int i = 0; i < particles.size(); i++) {
        double total_log_prob = 0.0;
        for (int j = 0; j < measurements.size(); j++) {
            double mtheta = std::get<2>(particles[i]) + measurements[j].second;
            double mx = std::get<0>(particles[i]) + measurements[j].first * cos(mtheta);
            double my = std::get<1>(particles[i]) + measurements[j].first * sin(mtheta);
            double min_dl = 0.0;
            for (int k = 0; k < landmarks.size(); k++) {
                double dx = landmarks[k].first - mx;
                double dy = landmarks[k].second - my;
                double dl = sqrt(dx * dx + dy * dy);
                if (k == 0) {
                    min_dl = dl;
                } else {
                    if (min_dl > dl)
                        min_dl = dl;
                }
            }
            double prob = z_hit * norm_coef * exp(-0.5 * (min_dl * min_dl) / (2.0 * measurement_variance)) + z_rand * 10e-6;
            prob *= measurement_resolution;
            if (prob > 1.0)
                prob = 1.0;
            total_log_prob += log(prob);
        }
        double prob = exp(total_log_prob);
        double weight = std::get<3>(particles[i]) * prob;
        std::get<3>(particles[i]) = weight;
        total_weight += weight;
    }

    // normalize weights and calculate effective sample size
    this->effective_sample_size = 0.0;
    for (int i = 0; i < particles.size(); i++) {
        std::get<3>(particles[i]) /= total_weight;
        this->effective_sample_size += std::get<3>(particles[i]) * std::get<3>(particles[i]);
    }
    this->effective_sample_size = 1.0 / this->effective_sample_size;
}

void MCL::estimate_robot_pose()
{
    double x = 0.0;
    double y = 0.0;
    double theta = 0.0;
    double tmp_theta = theta;
    for (int i = 0; i < particles.size(); i++) {
        x += std::get<0>(particles[i]) * std::get<3>(particles[i]);
        y += std::get<1>(particles[i]) * std::get<3>(particles[i]);
        double dtheta = tmp_theta - std::get<2>(particles[i]);
        dtheta = mod_yaw(dtheta);
        theta += dtheta * std::get<3>(particles[i]);
    }
    double theta_ = tmp_theta - theta;
    this->robot_pose_x = x;
    this->robot_pose_y = y;
    this->robot_pose_theta = mod_yaw(theta_);
    std::cout << "Estimated pose: " << robot_pose_x << ", " << robot_pose_y << ", " << robot_pose_theta << std::endl;
}

void MCL::resample_particles()
{
    if (this->effective_sample_size > resample_threshold * static_cast<double>(particle_num))
        return;
    std::vector<Particle> tmp_particles(particles);
    double w = 1.0 / static_cast<double>(particle_num);
    std::vector<double> board;
    board.push_back(std::get<3>(tmp_particles[0]));
    for (int i = 0; i < particles.size(); i++)
        board.push_back(board[i-1] + std::get<3>(tmp_particles[i+1]));
    for (int i = 0; i < particles.size(); i++) {
        double darts = generate_normal_random(0.0, w);
        for (int j = 0; j < particles.size(); j++) {
            if (darts < board[j]) {
                particles[i] = tmp_particles[j];
                std::get<3>(particles[i]) = w;
                break;
            }
        }
    }
}

std::vector<std::pair<double,double>> MCL::getMeasurements() {
    return this->measurements;
}

double MCL::getPlotSizeX() {
    return this->plot_size_x;
}

double MCL::getPlotSizeY() {
    return this->plot_size_y;
}

double MCL::getRobotPoseX() {
    return this->robot_pose_x;
}

double MCL::getRobotPoseY() {
    return this->robot_pose_y;
}

double MCL::getRobotPoseTheta() {
    return this->robot_pose_theta;
}

std::vector<Particle> MCL::getParticles() {
    return this->particles;
}

int MCL::getParticleNum() {
    return this->particle_num;
}

std::vector<std::pair<double, double>> MCL::getLandmarks() {
    return this->landmarks;
}

PYBIND11_MODULE(mcl_module, m) {
    py::class_<MCL>(m, "MCL")
        .def(py::init<double, double, double>())
        .def_readwrite("robot_pose_x", &MCL::robot_pose_x)
        .def_readwrite("robot_pose_y", &MCL::robot_pose_y)
        .def_readwrite("robot_pose_theta", &MCL::robot_pose_theta)
        .def_readwrite("plot_size_x", &MCL::plot_size_x)
        .def_readwrite("plot_size_y", &MCL::plot_size_y)
        .def_readwrite("landmarks", &MCL::landmarks)
        .def_readwrite("particles", &MCL::particles)
        .def("set_particle_num", &MCL::set_particle_num)
        .def("set_odom_noises", &MCL::set_odom_noises)
        .def("set_measurement_variance", &MCL::set_measurement_variance)
        .def("set_measurement_resolution", &MCL::set_measurement_resolution)
        .def("set_measurement_model_coefficients", &MCL::set_measurement_model_coefficients)
        .def("set_resample_threshold", &MCL::set_resample_threshold)
        .def("set_plot_sizes", &MCL::set_plot_sizes)
        .def("add_landmark", &MCL::add_landmark)
        .def("print_estimated_pose", &MCL::print_estimated_pose)
        .def("print_effective_sample_size_and_total_weight", &MCL::print_effective_sample_size_and_total_weight)
        .def("initialize_particles", &MCL::initialize_particles)
        .def("update_particles", &MCL::update_particles)
        .def("calculate_weights", &MCL::calculate_weights)
        .def("estimate_robot_pose", &MCL::estimate_robot_pose)
        .def("resample_particles", &MCL::resample_particles)
        .def("getMeasurements", &MCL::getMeasurements)
        .def("getParticles", &MCL::getParticles)
        .def("getPlotSizeX", &MCL::getPlotSizeX)
        .def("getPlotSizeY", &MCL::getPlotSizeY)
        .def("getRobotPoseX", &MCL::getRobotPoseX)
        .def("getRobotPoseY", &MCL::getRobotPoseY)
        .def("getRobotPoseTheta", &MCL::getRobotPoseTheta)
        .def("getParticleNum", &MCL::getParticleNum)
        .def("getLandmarks", &MCL::getLandmarks);
}
