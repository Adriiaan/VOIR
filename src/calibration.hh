#pragma once

#include <opencv2/opencv.hpp>
#include <rpicamopencv.hpp>

struct CalibrationInfo {
    cv::Mat cameraMatrix, distCoeffs;
    std::vector<std::vector<cv::Point2f>> imagePoints;
};

struct CalibrationInfo calibrate(rpicamopencv::PiCamStill &cam);
