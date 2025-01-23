#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import math
from numpy.random import normal, random
import matplotlib.pyplot as plt

class RobotSimulation:
    def __init__(self, pos_x=0.0, pos_y=0.0, orientation=0.0):
        self.true_x = pos_x
        self.true_y = pos_y
        self.true_yaw = orientation
        self.simulated_x = pos_x
        self.simulated_y = pos_y
        self.simulated_yaw = orientation
        self.simulated_velocity = 0.0
        self.simulated_angular_velocity = 0.0

        self.odometry_noise1 = 1.0
        self.odometry_noise2 = 0.5
        self.odometry_noise3 = 0.5
        self.odometry_noise4 = 2.0
        self.simulation_time_step = 0.05
        self.max_sensor_range = 5.0
        self.range_variance = 0.1 * 0.1
        self.angle_variance = 0.01 * 0.01
        self.random_measurement_probability = 1.0
        self.landmark_positions = []

        self.plot_width = 5.0
        self.plot_height = 5.0

        self.PI = 3.14159265359
        self.TWO_PI = 6.28318530718

    def set_odometry_noises(self, noise1, noise2, noise3, noise4):
        self.odometry_noise1 = noise1
        self.odometry_noise2 = noise2
        self.odometry_noise3 = noise3
        self.odometry_noise4 = noise4

    def set_plot_dimensions(self, width, height):
        self.plot_width = width
        self.plot_height = height

    def set_simulation_time_step(self, time_step):
        self.simulation_time_step = time_step

    def set_max_sensor_range(self, max_range):
        self.max_sensor_range = max_range

    def set_measurement_variances(self, range_variance, angle_variance):
        self.range_variance = range_variance
        self.angle_variance = angle_variance

    def set_random_measurement_probability(self, probability):
        self.random_measurement_probability = probability

    def add_landmark(self, x, y):
        self.landmark_positions.append([x, y])

    def display_landmarks(self):
        for i, landmark in enumerate(self.landmark_positions):
            print(f"{i}th landmark: x = {landmark[0]} [m], y = {landmark[1]} [m]")
        print('')

    def normalize_yaw(self, yaw):
        while yaw < -self.PI:
            yaw += self.TWO_PI
        while yaw > self.PI:
            yaw -= self.TWO_PI
        return yaw

    def display_true_pose(self):
        print(f"True pose: x = {self.true_x} [m], y = {self.true_y} [m], yaw = {self.true_yaw * 180.0 / math.pi} [deg]")

    def get_true_pose(self):
        return self.true_x, self.true_y, self.true_yaw

    def display_simulated_pose(self):
        print(f"Simulated pose: x = {self.simulated_x} [m], y = {self.simulated_y} [m], yaw = {self.simulated_yaw * 180.0 / math.pi} [deg]")

    def get_simulated_pose(self):
        return self.simulated_x, self.simulated_y, self.simulated_yaw

    def update_pose(self, velocity, angular_velocity):
        delta_distance = velocity * self.simulation_time_step
        delta_yaw = angular_velocity * self.simulation_time_step
        new_x = self.true_x + delta_distance * math.cos(self.true_yaw)
        new_y = self.true_y + delta_distance * math.sin(self.true_yaw)
        new_yaw = self.true_yaw + delta_yaw
        self.true_x = new_x
        self.true_y = new_y
        self.true_yaw = self.normalize_yaw(new_yaw)

        delta_distance_squared = delta_distance * delta_distance
        delta_yaw_squared = delta_yaw * delta_yaw
        simulated_distance = delta_distance * 0.9 + normal(0.0, self.odometry_noise1 * delta_distance_squared + self.odometry_noise2 * delta_yaw_squared)
        simulated_yaw = delta_yaw * 0.9 + normal(0.0, self.odometry_noise3 * delta_distance_squared + self.odometry_noise4 * delta_yaw_squared)
        new_simulated_x = self.simulated_x + simulated_distance * math.cos(self.simulated_yaw)
        new_simulated_y = self.simulated_x + simulated_distance * math.sin(self.simulated_yaw)
        new_simulated_yaw = self.simulated_yaw + simulated_yaw
        self.simulated_x = new_simulated_x
        self.simulated_y = new_simulated_y
        self.simulated_yaw = self.normalize_yaw(new_simulated_yaw)
        self.simulated_velocity = simulated_distance / self.simulation_time_step
        self.simulated_angular_velocity = simulated_yaw / self.simulation_time_step

    def display_simulated_velocities(self):
        print(f"Velocity = {self.simulated_velocity} [m/sec], Angular velocity = {self.simulated_angular_velocity} [rad/sec]")

    def get_simulated_velocities(self):
        return self.simulated_velocity, self.simulated_angular_velocity

    def get_sensor_measurements(self):
        measurements = []
        for landmark in self.landmark_positions:
            dx = landmark[0] - self.true_x
            dy = landmark[1] - self.true_y
            distance = normal(math.sqrt(dx * dx + dy * dy), self.range_variance)
            if distance <= self.max_sensor_range:
                angle = normal(math.atan2(dy, dx) - self.true_yaw, self.angle_variance)
                angle = self.normalize_yaw(angle)
                if random() < self.random_measurement_probability:
                    distance = random() * self.max_sensor_range
                measurements.append([distance, angle])
        return measurements
