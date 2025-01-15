#include "sgbm.hh"
cv::Mat compute_sgbm(cv::Mat &imgL, cv::Mat &imgR)
{
    // Setting parameters for StereoSGBM algorithm
    int minDisparity = 0;
    int numDisparities = 64;
    int blockSize = 8;
    int disp12MaxDiff = 1;
    int uniquenessRatio = 10;
    int speckleWindowSize = 10;
    int speckleRange = 8;

    // Creating an object of StereoSGBM algorithm
    cv::Ptr<cv::StereoSGBM> stereo = cv::StereoSGBM::create(
        minDisparity, numDisparities, blockSize, disp12MaxDiff, uniquenessRatio,
        speckleWindowSize, speckleRange);

    // Calculating disparith using the StereoSGBM algorithm
    cv::Mat disp;
    stereo->compute(imgL, imgR, disp);

    // Normalizing the disparity map for better visualisation
    cv::normalize(disp, disp, 0, 255, cv::NORM_MINMAX, CV_8UC1);

    return disp;
}
