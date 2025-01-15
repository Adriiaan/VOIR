#pragma once

#include <opencv2/opencv.hpp>

#include "calibration.hh"

cv::Mat compute_sgbm(cv::Mat &imagL, cv::Mat &imagR, struct RectifyMaps maps);
