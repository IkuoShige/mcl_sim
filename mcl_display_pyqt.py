import sys
import math
from PyQt5.QtWidgets import QApplication, QMainWindow, QPushButton, QGraphicsScene, QGraphicsView, QGraphicsEllipseItem, QGraphicsLineItem, QGraphicsPolygonItem, QGraphicsPathItem, QLabel, QGraphicsPixmapItem
from PyQt5.QtCore import Qt, QTimer, QPointF, QLineF, QRectF
from PyQt5.QtGui import QPen, QColor, QBrush, QPolygonF, QPainterPath, QPixmap
from robot import RobotSimulation
from build import mcl_module

class MCLVisualizer(QMainWindow):
    def __init__(self):
        super().__init__()

        self.setWindowTitle("MCL Visualization")
        self.setGeometry(100, 100, 800, 600)

        # Create RobotSim instance
        self.robot = RobotSimulation()

        # Create graphics scene and view
        self.scene = QGraphicsScene()
        self.view = QGraphicsView(self.scene)
        self.setCentralWidget(self.view)

        # Create a central widget to hold the background and other elements
        central_widget = QLabel(self)
        central_widget.setGeometry(0, 0, self.width(), self.height())
        central_widget.setAlignment(Qt.AlignCenter)

        # Create buttons
        self.reset_button = QPushButton("Restart", self)
        self.reset_button.move(380, 10)
        self.reset_button.clicked.connect(self.start_simulation)

        self.start_simulation()

        # Set up timer for updating visualization
        self.timer = QTimer(self)
        self.timer.timeout.connect(self.update_visualization)
        self.timer.start(int(self.robot.simulation_time_step * 1000))

    def update_visualization(self):
        self.scene.clear()

        self.robot.update_pose(0.4, -0.4)
        delta_dist, delta_yaw = self.robot.simulated_velocity * self.robot.simulation_time_step, self.robot.simulated_angular_velocity * self.robot.simulation_time_step
        measurements = self.robot.get_sensor_measurements()

        self.mcl.update_particles(delta_dist, delta_yaw)
        self.mcl.calculate_weights(measurements)
        self.mcl.estimate_robot_pose()
        self.mcl.resample_particles()

        self.plot_mcl_world(measurements)

    def plot_mcl_world(self, measurements):
        # 角度の正規化
        def normalize_angle(angle):
            return (angle + math.pi) % (2 * math.pi) - math.pi

        canvas_width = self.view.viewport().width()
        canvas_height = self.view.viewport().height()

        # Plot landmarks
        for landmark in self.mcl.landmarks:
            x, y = self.convert_coordinates(landmark[0], landmark[1], canvas_width, canvas_height, 40)
            self.scene.addRect(x - 5*3, y - 5*3, 10*3, 10*3, Qt.black)

        # Plot estimated robot pose with arrow
        x = self.mcl.robot_pose_x
        y = self.mcl.robot_pose_y
        yaw = self.mcl.robot_pose_theta

        arrow_length = 6
        scale = 40

        x2, y2 = self.convert_coordinates((x * scale/arrow_length) + arrow_length * math.cos(yaw), (y * scale/arrow_length) + arrow_length * math.sin(yaw), canvas_width, canvas_height, arrow_length)
        x1, y1 = self.convert_coordinates((x * scale/arrow_length), (y * scale/arrow_length), canvas_width, canvas_height, arrow_length)

        self.plot_arrow(x1, y1, x2, y2, yaw, Qt.blue, arrow_length)

        # Plot particles with arrow
        for particle in self.mcl.particles:
            x, y, yaw, _ = particle
            x = x
            y = y
            yaw = yaw

            arrow_length = 4

            x2, y2 = self.convert_coordinates((x * scale/arrow_length) + arrow_length * math.cos(yaw), (y * scale/arrow_length) + arrow_length * math.sin(yaw), canvas_width, canvas_height, arrow_length)
            x1, y1 = self.convert_coordinates(x * scale/arrow_length, y * scale/arrow_length, canvas_width, canvas_height, arrow_length)

            self.plot_arrow(x1, y1, x2, y2, yaw, Qt.black, arrow_length)

        # Correct sensor measurement drawing
        for measurement in measurements:
            distance, angle = measurement
            angle = normalize_angle(self.mcl.robot_pose_theta + angle)
            mx = distance * math.cos(angle) + self.mcl.robot_pose_x
            my = distance * math.sin(angle) + self.mcl.robot_pose_y
            mx, my = self.convert_coordinates(mx, my, canvas_width, canvas_height, 40)

            mx = max(0, min(mx, canvas_width))
            my = max(0, min(my, canvas_height))

            self.scene.addEllipse(mx - 3*3, my - 3*3, 6*3, 6*3, Qt.red)

    def convert_coordinates(self, x, y, canvas_width, canvas_height, scale_factor=1):
        converted_x = (x * scale_factor + 520)
        converted_y = 740 - (y * scale_factor + 370)
        return converted_x, converted_y
    
    def dist_to_coord_linear(self, xa, xb, ya, yb, yaw, dist):
        # ターゲット座標と現在の位置座標の間の角度を計算
        th = math.atan2(yb - ya, xb - xa)

        # 距離と角度を考慮して新しい座標を計算
        x3 = xa + (dist * math.cos(th))
        y3 = ya + (dist * math.sin(th))

        # 新しい座標を整数に変換して返す
        return x3, y3


    def plot_arrow(self, x1, y1, x2, y2, yaw, color, arrow_length):
        arrow_head_length = 8
        arrow_width = 8  # Width of the arrow line

        # Draw the main arrow line
        line_outer = self.scene.addLine(x1, y1, x2, y2, QPen(QColor(Qt.black), arrow_width + 2 * 1))  # Outer black line
        line_outer.setZValue(10)
        line_inner = self.scene.addLine(x1, y1, x2, y2, QPen(QColor(color), arrow_width)) # Inner colored line
        line_inner.setZValue(11)

        x2_, y2_ = self.dist_to_coord_linear(x1, x2, y1, y2, yaw, (arrow_length+2)*arrow_length)

        # Calculate the points for the arrow head
        arrow_head_points = [
            QPointF(x2_, y2_),
            QPointF(x2_ - arrow_head_length * 2.0 * math.cos(yaw + math.pi / 4), y2_ + arrow_head_length * 2.0 * math.sin(yaw + math.pi / 4)),
            QPointF(x2_ - arrow_head_length * 1.5 * math.cos(yaw), y2_ + arrow_head_length * 1.5 * math.sin(yaw)),
            QPointF(x2_ - arrow_head_length * 2.0 * math.cos(yaw - math.pi / 4), y2_ + arrow_head_length * 2.0 * math.sin(yaw - math.pi / 4))
        ]

        # Create a path for the arrow head
        arrow_path = QPainterPath()
        arrow_path.moveTo(arrow_head_points[0])
        for point in arrow_head_points[1:]:
            arrow_path.lineTo(point)
        arrow_path.lineTo(arrow_head_points[0])

        # Create and add a path item for the arrow head to the scene
        arrow_head_item = QGraphicsPathItem(arrow_path)
        pen = QPen(QColor(Qt.black))
        pen.setWidth(1)  # Set the line width for the outer black line
        arrow_head_item.setPen(pen)
        arrow_head_item.setBrush(QBrush(color))
        arrow_head_item.setZValue(12)
        self.scene.addItem(arrow_head_item)
        


    def start_simulation(self):
        # Implement any initialization or actions needed when the "Start" button is clicked

        # Initialize robot simulation parameters
        start_x = 0.0
        start_y = 0.0
        start_yaw = 0.0 * math.pi / 180.0
        max_measurement_range = 10.0
        measurement_range_variance_sim = 0.2 * 0.2 
        measurement_angle_variance_sim = 3.0 * math.pi / 180.0 * 3.0 * math.pi / 180.0
        landmark_scale = 1.2
        # Initialize the robot simulator
        self.robot = RobotSimulation(start_x, start_y, start_yaw)
        self.robot.add_landmark(3.0*landmark_scale, 2.0*landmark_scale)
        self.robot.add_landmark(3.0*landmark_scale, -2.0*landmark_scale)
        self.robot.add_landmark(-3.0*landmark_scale, -2.0*landmark_scale)
        self.robot.add_landmark(-3.0*landmark_scale, 2.0*landmark_scale)
        # Add more landmarks as needed
        self.robot.set_odometry_noises(0.33, 0.1, 0.1, 0.33)
        self.robot.set_max_sensor_range(max_measurement_range)
        self.robot.set_measurement_variances(measurement_range_variance_sim, measurement_angle_variance_sim)
        self.robot.set_random_measurement_probability(0.05)
        self.robot.set_plot_dimensions(max_measurement_range, max_measurement_range)
        self.robot.set_simulation_time_step(0.1)
        # Initialize MCL parameters
        particle_num = 50
        measurement_variance = 0.3 * 0.3 
        measurement_resolution = 0.1 
        initial_var_x = 0.2 
        initial_var_y = 0.2 
        initial_var_yaw = 2.0 * math.pi / 180.0
        # Initialize the MCL
        self.mcl = mcl_module.MCL(start_x, start_y, start_yaw)
        self.mcl.set_particle_num(particle_num)
        self.mcl.set_odom_noises(20.0, 10.0, 10.0, 30.0)
        self.mcl.add_landmark(3.0*landmark_scale, 2.0*landmark_scale)
        self.mcl.add_landmark(3.0*landmark_scale, -2.0*landmark_scale)
        self.mcl.add_landmark(-3.0*landmark_scale, -2.0*landmark_scale)
        self.mcl.add_landmark(-3.0*landmark_scale, 2.0*landmark_scale)
        # Add more landmarks as needed
        self.mcl.set_measurement_variance(measurement_variance)
        self.mcl.set_measurement_resolution(measurement_resolution)
        self.mcl.set_measurement_model_coefficients(0.9, 0.1)
        self.mcl.set_resample_threshold(0.5)
        self.mcl.set_plot_sizes(5.0, 5.0)
        self.mcl.initialize_particles(initial_var_x, initial_var_y, initial_var_yaw)
    
    def load_background_image(self, image_path):
        pixmap = QPixmap(image_path)
        background_item = QGraphicsPixmapItem(pixmap)
        background_item.setZValue(0)
        self.scene.addItem(background_item)
        self.view.setSceneRect(0, 0, pixmap.width(), pixmap.height())

if __name__ == "__main__":
    app = QApplication(sys.argv)
    app.setStyle("Fusion")
    window = MCLVisualizer()
    window.show()
    sys.exit(app.exec_())

