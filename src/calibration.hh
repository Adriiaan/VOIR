#pragma once

#include <opencv2/opencv.hpp>
#include <rpicamopencv.hpp>

struct CalibrationInfo {
    cv::Mat cameraMatrix, distCoeffs;
    std::vector<std::vector<cv::Point2f>> imagePoints;
};

struct RectifyMaps {
    cv::Mat map1x, map2x, map1y, map2y;
}

struct CalibrationInfo calibrate(rpicamopencv::PiCamStill &cam);
struct RectifyMaps stereoCalibrate(struct CalibrationInfo master, struct CalibrationInfo slave);