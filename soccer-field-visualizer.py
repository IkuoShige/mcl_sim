import sys
import math
from PyQt5.QtWidgets import QApplication, QWidget, QPushButton
from PyQt5.QtCore import Qt, QTimer
from PyQt5.QtGui import QPixmap, QPainter, QPen, QColor, QBrush
from robot import RobotSimulation
from build import mcl_module

class MCLVisualizer(QWidget):
    def __init__(self):
        super().__init__()
        self.field_pixmap = QPixmap("hlfield.png")
        self.setFixedSize(self.field_pixmap.size())

        # Create RobotSim instance
        self.robot = RobotSimulation()
        self.start_simulation()

        # Create a button
        self.reset_button = QPushButton("Restart", self)
        self.reset_button.move(10, 10)
        self.reset_button.clicked.connect(self.start_simulation)

        # Timer for updating
        self.timer = QTimer(self)
        self.timer.timeout.connect(self.update_visualization)
        self.timer.start(int(self.robot.simulation_time_step * 1000))

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.drawPixmap(self.rect(), self.field_pixmap)

        # Draw landmarks
        for lx, ly in self.mcl.landmarks:
            x, y = self.convert_coordinates(lx, ly)
            painter.setPen(QPen(Qt.black, 2))
            painter.setBrush(QBrush(Qt.black))
            painter.drawEllipse(int(x - 5), int(y - 5), 10, 10)

        # Draw particles
        for px, py, pyaw, _ in self.mcl.particles:
            x1, y1 = self.convert_coordinates(px, py)
            length = 10
            x2 = x1 + length * math.cos(pyaw)
            y2 = y1 - length * math.sin(pyaw)
            painter.setPen(QPen(Qt.black, 2))
            painter.drawLine(int(x1), int(y1), int(x2), int(y2))
            painter.drawEllipse(int(x1 - 2), int(y1 - 2), 4, 4)

        # Draw true pose
        rx, ry, ryaw = self.robot.true_x, self.robot.true_y, self.robot.true_yaw
        robot_x, robot_y = self.convert_coordinates(rx, ry)
        painter.setPen(QPen(Qt.red, 3))
        painter.setBrush(QBrush(Qt.yellow))
        painter.drawEllipse(int(robot_x - 6), int(robot_y - 6), 12, 12)
        heading_len = 15
        hx = robot_x + heading_len * math.cos(ryaw)
        hy = robot_y - heading_len * math.sin(ryaw)
        painter.drawLine(int(robot_x), int(robot_y), int(hx), int(hy))

        # Draw estimated robot
        rx, ry, ryaw = self.mcl.robot_pose_x, self.mcl.robot_pose_y, self.mcl.robot_pose_theta
        robot_x, robot_y = self.convert_coordinates(rx, ry)
        painter.setPen(QPen(Qt.red, 3))
        painter.setBrush(QBrush(Qt.blue))
        painter.drawEllipse(int(robot_x - 6), int(robot_y - 6), 12, 12)
        heading_len = 15
        hx = robot_x + heading_len * math.cos(ryaw)
        hy = robot_y - heading_len * math.sin(ryaw)
        painter.drawLine(int(robot_x), int(robot_y), int(hx), int(hy))

        # Draw measurements
        measurements = self.robot.get_sensor_measurements()
        for dist, angle in measurements:
            global_angle = ryaw + angle
            mx = rx + dist * math.cos(global_angle)
            my = ry + dist * math.sin(global_angle)
            mx, my = self.convert_coordinates(mx, my)
            painter.setBrush(QBrush(Qt.red))
            painter.drawEllipse(int(mx - 3), int(my - 3), 6, 6)

    def update_visualization(self):
        # Update robot and MCL
        self.robot.update_pose(10.0, -0.13)
        delta_dist = self.robot.simulated_velocity * self.robot.simulation_time_step
        delta_yaw = self.robot.simulated_angular_velocity * self.robot.simulation_time_step

        measurements = self.robot.get_sensor_measurements()
        self.mcl.update_particles(delta_dist, delta_yaw)
        self.mcl.calculate_weights(measurements)
        self.mcl.estimate_robot_pose()
        self.mcl.resample_particles()
        self.display_estimated_pose()

        self.update()

    def start_simulation(self):
        # Initialize MCL parameters
        start_x = 0.0
        start_y = 75.0
        start_yaw = 0.0 * math.pi / 180.0
        max_measurement_range = 1000.0
        measurement_range_variance_sim = 0.2 * 0.2
        measurement_range_variance_sim = 2.0 * 2.0
        measurement_angle_variance_sim = 3.0 * math.pi / 180.0 * 3.0 * math.pi / 180.0
        particle_num = 100
        measurement_variance = 0.3 * 0.3
        measurement_variance = 3.0 * 3.0
        measurement_resolution = 0.1
        initial_var_x = 2 
        initial_var_y = 2
        initial_var_x = 10
        initial_var_y = 10
        initial_var_yaw = 2.0 * math.pi / 180.0
        
        self.robot = RobotSimulation(start_x, start_y, start_yaw)
        self.robot.add_landmark(-450, 130)
        self.robot.add_landmark(-450, -130)
        self.robot.add_landmark(450, 130)
        self.robot.add_landmark(450, -130)
        self.robot.add_landmark(0, 300)
        self.robot.add_landmark(0, -300)
        self.robot.add_landmark(450, -300)
        self.robot.add_landmark(450, 300)
        self.robot.add_landmark(-450, -300)
        self.robot.add_landmark(-450, 300)
        self.robot.set_odometry_noises(0.33, 0.1, 0.1, 0.33)
        self.robot.set_max_sensor_range(max_measurement_range)
        self.robot.set_measurement_variances(measurement_range_variance_sim, measurement_angle_variance_sim)
        self.robot.set_random_measurement_probability(0.05)
        self.robot.set_plot_dimensions(max_measurement_range, max_measurement_range)
        self.robot.set_simulation_time_step(0.1)

        self.mcl = mcl_module.MCL(start_x, start_y, start_yaw)
        self.mcl.set_particle_num(particle_num)
        self.mcl.set_odom_noises(0.33, 0.1, 0.1, 0.33)
        self.mcl.add_landmark(-450, 130)
        self.mcl.add_landmark(-450, -130)
        self.mcl.add_landmark(450, 130)
        self.mcl.add_landmark(450, -130)
        self.mcl.add_landmark(0, 300)
        self.mcl.add_landmark(0, -300)
        self.mcl.add_landmark(450, -300)
        self.mcl.add_landmark(450, 300)
        self.mcl.add_landmark(-450, -300)
        self.mcl.add_landmark(-450, 300)

        self.mcl.set_measurement_variance(measurement_variance)
        self.mcl.set_measurement_resolution(measurement_resolution)
        self.mcl.set_measurement_model_coefficients(0.9, 0.1)
        self.mcl.set_resample_threshold(0.5)
        self.mcl.set_plot_sizes(5.0, 5.0)
        self.mcl.initialize_particles(initial_var_x, initial_var_y, initial_var_yaw)

        self.update()

    def convert_coordinates(self, x, y):
        scale_x = self.width() / 1040.0
        scale_y = self.height() / 740.0
        cx = self.width() / 2
        cy = self.height() / 2
        screen_x = cx + x * scale_x
        screen_y = cy - y * scale_y
        return screen_x, screen_y
    
    def display_estimated_pose(self):
        print(f"Robot Pose: X = {self.mcl.getRobotPoseX():.2f}, Y = {self.mcl.getRobotPoseY():.2f}, Theta = {self.mcl.getRobotPoseTheta():.3f}")

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MCLVisualizer()
    window.show()
    sys.exit(app.exec_())
