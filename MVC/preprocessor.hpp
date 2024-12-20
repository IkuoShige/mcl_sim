#ifndef PREPROCESSOR_HPP
#define PREPROCESSOR_HPP

#pragma once
#include <string>
#include <tuple>
#include <vector>
#include <opencv2/opencv.hpp>
#include "types.hpp"
#include "item.hpp"

class Preprocessor {
public:
    Preprocessor();
    ~Preprocessor();

    void loadWhiteLineMap(const std::string &filename, std::vector<int> &out_wlmap, int *width, int *height);
    void add_relative_pose(double s, double c, double x, double y, double &xt, double &yt);
    void add_relative_pose(double s, double c, double x, double y, double lx, double ly, double &nx, double &ny); // 修正: シグネチャを一致させる
    std::vector<cv::Point2f> convertWhitelineToFieldCoordinates(const std::vector<cv::Point2f> &whiteline, const Pose &pos);

    // バッファ情報を表示する関数
    void printBufferInfo(const std::vector<unsigned char>& buffer);

    // 画像をデコードする関数
    std::vector<unsigned char> decodeImage(const std::string& filepath);
    std::pair<double, double> undistort(int x, int y, int width, int height);

    // ピクセル座標をワールド座標に変換する関数
    std::tuple<int, double, double, double> pixel_to_world(int xt, int yt, int width, int height);

    // 白線の座標を生成する関数
    std::vector<cv::Point2f> op3_gen_wl(const cv::Mat& dataset_image);

    // 白線のポイントをファイルに保存する関数
    void save_white_line_points(const std::vector<cv::Point2f>& wlpos, const std::string& filename);
    void load_white_line_points(const std::string& filename, std::vector<cv::Point2f>& wlpos);
    std::vector<cv::Point2f> simulate_white_line(const std::vector<cv::Point2f>& wlpos, const Pose &robot_pose, double fov, double max_distance);

    void signal_handler(int signal);
    cv::Mat receive_image(const char* host, int port);

    bool keep_running;
};

#endif // PREPROCESSOR_HPP