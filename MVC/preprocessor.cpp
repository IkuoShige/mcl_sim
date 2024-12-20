#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <iomanip>
#include <random>
#include <cmath>
#include <cassert>
#include <tuple>
#include <opencv2/opencv.hpp>
#include "preprocessor.hpp"
#include "types.hpp"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <csignal>
#include <fstream>

// void logError(const std::string &message, const std::string &filename) {
//     std::cerr << "Error: " << message << " " << filename << std::endl;
// }

// std::vector<double> loadLikelihoodFieldMap(const std::string &filename, std::vector<double> &out_likelihood_map, int *width, int *height) {
//     std::ifstream file(filename);
//     if (!file.is_open()) {
//         logError("Could not open the file", filename);
//         // return false;
//     }

//     std::string line;
//     // Read the PGM header
//     std::getline(file, line);
//     if (line != "P5") {
//         logError("Not a valid PGM file", filename);
//         // return false;
//     }

//     // Skip comments
//     while (std::getline(file, line)) {
//         if (line[0] != '#') {
//             break;
//         }
//     }

//     std::istringstream dimensions(line);
//     dimensions >> *width >> *height;

//     int max_value;
//     file >> max_value;
//     file.ignore(); // Ignore the newline after max_value

//     out_likelihood_map.resize((*width) * (*height));
//     file.read(reinterpret_cast<char*>(out_likelihood_map.data()), out_likelihood_map.size());

//     // Normalize values to [0, 1]
//     for (double &value : out_likelihood_map) {
//         value = static_cast<double>(value) / max_value; // 正規化
//     }

//     // return true;
//     return out_likelihood_map;
// }

Preprocessor::Preprocessor() : keep_running(true) {}

Preprocessor::~Preprocessor() {}

void Preprocessor::loadWhiteLineMap(const std::string &filename, std::vector<int> &out_wlmap, int *width, int *height) {
    cv::Mat mapimg = cv::imread(filename, cv::IMREAD_GRAYSCALE);
	if (mapimg.empty()) {
		// エラーハンドリング: 画像が読み込めなかった場合
		std::cerr << "Error: Could not read the image file." << std::endl;
		return;
	}

	// 画像のサイズを取得
	*width = mapimg.cols;
	*height = mapimg.rows;
    // std::cout << "Width: " << *width << " Height: " << *height << std::endl;

	// out_wlmapのサイズを設定
	out_wlmap.resize(mapimg.rows * mapimg.cols);
	int p = 0;
    for (int y = 0; y < mapimg.rows; y++) {
        for (int x = 0; x < mapimg.cols; x++) {
            unsigned char pixelValue = mapimg.at<unsigned char>(y, x);
            out_wlmap[p++] = pixelValue;
            // if (out_wlmap[p-1] != 0) {
            //     std::cout << "out_wlmap[" << p << "]: " << out_wlmap[p-1] << std::endl;
            // }
        }
    }
}

void Preprocessor::add_relative_pose(double s, double c, double x, double y, double lx, double ly, double &nx, double &ny) {
    nx = x + lx * c - ly * s;
    ny = y + lx * s + ly * c;
}

std::vector<cv::Point2f> Preprocessor::convertWhitelineToFieldCoordinates(const std::vector<cv::Point2f> &whiteline, const Pose &pos) {
    std::vector<cv::Point2f> fieldCoordinates;
    double c = std::cos(pos.theta);
    double s = std::sin(pos.theta);

    for (const auto &point : whiteline) {
        double xt = point.x + 270;
        double yt = point.y;
        double nx, ny;
        add_relative_pose(s, c, pos.x * 10, pos.y * 10, xt, yt, nx, ny);
        fieldCoordinates.emplace_back(nx/10, ny/10);
    }

    return fieldCoordinates;
}

void Preprocessor::printBufferInfo(const std::vector<unsigned char>& buffer) {
    std::cout << "バッファサイズ: " << buffer.size() << " バイト" << std::endl;
    std::cout << "バッファの先頭50バイト: " << std::endl;
    for (int i = 0; i < std::min(50, static_cast<int>(buffer.size())); ++i) {
        std::cout << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(buffer[i]) << " ";
        if ((i + 1) % 16 == 0) std::cout << std::endl;
    }
    std::cout << std::dec << std::endl;
}

std::vector<unsigned char> Preprocessor::decodeImage(const std::string& filepath) {
    // バイナリファイルを読み込む
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        std::cerr << "Cannot open file." << std::endl;
        return {};
    }

    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(file), {});
    file.close();

    cv::Mat image = cv::imdecode(buffer, cv::IMREAD_UNCHANGED);
    if (image.empty()) {
        std::cerr << "faild to decode image." << std::endl;
        return {};
    }
    return image;
    // std::vector<unsigned char> decodeData(image.data, image.data + image.total() * image.elemSize());
    // return decodeData;
}

std::pair<double, double> Preprocessor::undistort(int x, int y, int width, int height) {
    double fov = 2.1642;
    double cx = width / 2.0;
    double cy = height / 2.0;
    double focal = (width / 2.0) / std::tan(fov / 2.0);
    double vfov = fov * height / width;

    double u_s = x - cx;
    double v_s = y - cy;

    double th = u_s / (width / 2.0) * (fov / 2.0);
    double ph = v_s / (height / 2.0) * (vfov / 2.0);

    double u_p = std::tan(th) * focal;
    double v_p = std::tan(ph) * std::hypot(focal, u_p);

    double x_p = u_p / focal;
    double y_p = -v_p / focal;

    return {x_p, y_p};
}

std::tuple<int, double, double, double> Preprocessor::pixel_to_world(int xt, int yt, int width, int height) {
    // カメラのポーズ行列
    cv::Mat camera_pose_matrix = (cv::Mat_<double>(3, 4) << 
        0.7071068, 0.0000000, 0.7071068, -0.24013989038188419,
        0.0000000, 1.0000000, 0.0000000, -4.5207827720772364e-06,
        -0.7071068, 0.0000000, 0.7071068, 0.46477837530781796);

    // ピクセル座標をカメラ座標に変換
    auto camera_coords = undistort(xt, yt, width, height);


    // カメラの位置を取得
    cv::Vec3d camera_position = camera_pose_matrix.col(3).rowRange(0, 3);
    // std::cout << "Camera position: " << camera_position << std::endl;

    // 同次座標を作成
    cv::Mat homogeneous_camera_coords = (cv::Mat_<double>(3, 1) << 1, camera_coords.first, camera_coords.second);
    // std::cout << "Homogeneous camera coords: " << homogeneous_camera_coords << std::endl;

    // カメラ座標をワールド座標に変換
    cv::Mat world_coords = camera_pose_matrix.colRange(0, 3).rowRange(0, 3) * homogeneous_camera_coords;
    // std::cout << "World coords: " << world_coords << std::endl;

    // 地面上のx, y座標を計算
    double x_ground = world_coords.at<double>(0);
    double y_ground = world_coords.at<double>(1);
    double angle;
    int result;

    // z成分が0未満の場合
    if (world_coords.at<double>(2) < 0.0) {
        double distance_to_ground = std::abs(camera_position[2] / world_coords.at<double>(2));
        x_ground = camera_position[0] + distance_to_ground * world_coords.at<double>(0);
        y_ground = camera_position[1] + distance_to_ground * world_coords.at<double>(1);
        angle = std::atan2(y_ground, x_ground);
        result = 0;
    } else {
        // 地面上にない場合
        x_ground = 10000.0;
        y_ground = 10000.0;
        angle = std::atan2(y_ground, x_ground);
        result = -1;
    }

    if (result == 0) {
        // std::cout << "x_ground: " << x_ground << ", y_ground: " << y_ground << std::endl;
    }

    return std::make_tuple(result, x_ground * 1000, -y_ground * 1000, angle);
    // return std::make_tuple(result, x_ground, -y_ground, angle);
}

std::vector<cv::Point2f> Preprocessor::op3_gen_wl(const cv::Mat& dataset_image) {
    int imh = dataset_image.rows;
    int imw = dataset_image.cols;
    // std::cout << "Image size: " << imw << "x" << imh << std::endl;

    std::vector<cv::Point2f> positions;
    positions.reserve(100000); // 予めメモリを確保

    // RGBのしきい値を設定
    const int threshold = 200;

    for (int yt = 0; yt < imh; ++yt) {
        for (int xt = 0; xt < imw; ++xt) {
            cv::Vec3b pixel = dataset_image.at<cv::Vec3b>(yt, xt);
            int b = pixel[0];
            int g = pixel[1];
            int r = pixel[2];

            if (b > threshold && g > threshold && r > threshold) {
                // int result;
                // double rx, ry, rth;
                auto [result, x_ground, y_ground, angle] = pixel_to_world(xt, yt, imw, imh);

                if (result == 0) {
                    // positions.emplace_back(x_ground / 10.0, y_ground / 10.0); // 射影変換した位置を追加
                    positions.emplace_back(x_ground, y_ground); // 射影変換した位置を追加
                }
            }
        }
    }

    return positions; // 射影変換された位置のベクターを返す
}

void Preprocessor::save_white_line_points(const std::vector<cv::Point2f>& wlpos, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file for writing: " << filename << std::endl;
        return;
    }

    for (const auto& point : wlpos) {
        file << point.x << " " << point.y << "\n"; // X Y形式で保存
    }

    file.close();
    std::cout << "White line points saved to " << filename << std::endl;
}

void Preprocessor::load_white_line_points(const std::string& filename, std::vector<cv::Point2f>& wlpos) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file for reading: " << filename << std::endl;
        return;
    }

    wlpos.clear();
    for (std::string line; std::getline(file, line);) {
        std::istringstream iss(line);
        double x, y;
        if (!(iss >> x >> y)) {
            std::cerr << "Error reading line: " << line << std::endl;
            continue;
        }
        wlpos.emplace_back(x, y);
    }
}

void Preprocessor::signal_handler(int signal) {
    if (signal == SIGINT) {
        keep_running = false;
    }
}

// void save_binary_data(const std::vector<unsigned char>& data, const std::string& filename) {
//     std::ofstream file(filename, std::ios::binary);
//     if (!file) {
//         std::cerr << "ファイルを開けませんでした: " << filename << std::endl;
//         return;
//     }
//     file.write(reinterpret_cast<const char*>(data.data()), data.size());
//     file.close();
//     std::cout << "バイナリデータを保存しました: " << filename << std::endl;
// }

cv::Mat Preprocessor::receive_image(const char* host, int port) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // ソケットの作成
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(host);
    address.sin_port = htons(port);

    // バインド
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // リッスン
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    fd_set readfds;
    struct timeval timeout;
    cv::Mat img;

    while (keep_running) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);

        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int activity = select(server_fd + 1, &readfds, NULL, NULL, &timeout);

        if (activity < 0 && errno != EINTR) {
            perror("select error");
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        if (activity > 0 && FD_ISSET(server_fd, &readfds)) {
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
                perror("accept");
                close(server_fd);
                exit(EXIT_FAILURE);
            }

            uint32_t data_length;
            if (read(new_socket, &data_length, sizeof(data_length)) <= 0) {
                close(new_socket);
                continue;
            }
            data_length = ntohl(data_length);

            std::vector<unsigned char> buffer(data_length);
            int bytes_received = 0;
            while (bytes_received < data_length) {
                int result = read(new_socket, buffer.data() + bytes_received, data_length - bytes_received);
                if (result <= 0) {
                    perror("recv");
                    close(new_socket);
                    break;
                }
                bytes_received += result;
            }

            // save_binary_data(buffer, "received_image_data.bin");

            img = cv::imdecode(buffer, cv::IMREAD_COLOR);
            if (img.empty()) {
                std::cerr << "Failed to decode image" << std::endl;
                close(new_socket);
                continue;
            }

            // cv::imshow("Received Image", img);
            // cv::waitKey(1);

            close(new_socket);
        }
    }

    close(server_fd);
    return img;
}

std::vector<cv::Point2f> Preprocessor::simulate_white_line(const std::vector<cv::Point2f>& wlpos, const Pose &robot_pose, double fov, double max_distance) {
    std::vector<cv::Point2f> simulated_points;
    double half_fov = fov / 2.0;

    for (const auto &point : wlpos) {
        double dx = point.x - robot_pose.x;
        double dy = point.y - robot_pose.y;
        double distance = std::sqrt(dx * dx + dy * dy);

        if (distance > max_distance) {
            continue;
        }

        double angle = std::atan2(dy, dx) - robot_pose.theta;
        if (angle > M_PI) {
            angle -= 2 * M_PI;
        } else if (angle < -M_PI) {
            angle += 2 * M_PI;
        }

        if (std::abs(angle) <= half_fov) {
            simulated_points.push_back(point);
        }
    }

    return simulated_points;
}
