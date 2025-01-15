#include "calibration.hh"

#include <iostream>

#include "picam.hh"

struct CalibrationInfo calibrate(rpicamopencv::PiCamStill &cam)
{
    cv::Mat cameraMatrix = cv::Mat::eye(3, 3, CV_64F);
    cv::Mat distCoeffs = cv::Mat::zeros(1, 5, CV_64F);
    std::vector<std::vector<cv::Point3f>>
        objectPoints; // 3D points in the real world
    std::vector<std::vector<cv::Point2f>>
        imagePoints; // 2D points in the image plane
    cv::Size imageSize(640, 480); // The size of the image (width, height)

    std::vector<cv::Mat> rvecs, tvecs;

    int board_height = 9;
    int board_width = 6;

    float square_size = 25.0f;

    std::vector<cv::Point3f> objPoints;
    for (int i = 0; i < board_height; i++)
    {
        for (int j = 0; j < board_width; j++)
        {
            objPoints.push_back(
                cv::Point3f(j * square_size, i * square_size, 0.0f));
        }
    }

    std::cout << "Press enter to start calibrating\n";
    char bite = 0;
    std::cin >> bite;
    (void)bite;

    // Take pictures and get imagePoints
    for (int n = 0; n < 20; n++)
    {
        cv::Mat image;
        cam.captureStillImage(image);

        cv::Mat grayImage;
        cv::cvtColor(image, grayImage, cv::COLOR_BGR2GRAY);
        std::cout << "Converted to grayscale\n";
        std::vector<cv::Point2f> imgPoints;

        bool found = cv::findChessboardCorners(
            grayImage, cv::Size(board_width, board_height), imgPoints);
        std::cout << "chessboard corners\n";
        if (found)
        {
            cv::cornerSubPix(grayImage, imgPoints, cv::Size(11, 11),
                             cv::Size(-1, -1),
                             cv::TermCriteria(cv::TermCriteria::EPS
                                                  | cv::TermCriteria::MAX_ITER,
                                              30, 0.1));

            std::cout << "placing corners\n";
            // Draw for visual confirmation
            cv::drawChessboardCorners(
                image, cv::Size(board_width, board_height), imgPoints, found);
            std::cout << "drawing corners\n";
#if MASTER
            display_to_fb(image);
#endif
            std::cout << "Calibration image n " << n << " displayed.\n";
            std::cin >> bite;
            (void)bite;

            objectPoints.push_back(objPoints);
            imagePoints.push_back(imgPoints);
        }
    }

    std::cin >> bite;
    (void)bite;
    double rms = cv::calibrateCamera(objectPoints, imagePoints, imageSize,
                                     cameraMatrix, distCoeffs, rvecs, tvecs);

    return { .cameraMatrix = cameraMatrix,
             .distCoeffs = distCoeffs,
             .imagePoints = imagePoints };
}

void stereoCalibrate(struct CalibrationInfo master, struct CalibrationInfo slave) {
    std::vector<std::vector<cv::Point3f>> objectPoints;

    int board_height = 9;
    int board_width = 6;

    float square_size = 25.0f;

    std::vector<cv::Point3f> objPoints;
    for (int i = 0; i < board_height; i++) {
        for (int j = 0; j < board_width; j++) {
            objPoints.push_back(cv::Point3f(j * square_size, i * square_size, 0.0f));
        }
    }

    for (int i = 0; i < master.imagePoints.size(); i++) {
        objectPoints.push_back(objPoints);
    }

    cv::Size imageSize(640, 480);

    cv::Mat R, T, E, F;
    
    double rms = cv::stereoCalibrate(objectPoints, master.imagePoints, slave.imagePoints,
                                    master.cameraMatrix, maste.distCoeffs, slave.cameraMatrix,
                                    slave.distCoeffs, imageSize, R, T, E, F, 0,
                                    cv::TermCriteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 100, 1e-6));
}
