#pragma once

#include <rpicamopencv.hpp>

struct FramebufferInfo
{
    int width;
    int height;
    int depth;

};

cv::Mat convertToRGB565(const cv::Mat &image);
cv::Mat take_picture(rpicamopencv::PiCamStill &cam);
struct FramebufferInfo getFramebufferInfo();
