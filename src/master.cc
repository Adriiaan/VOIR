#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <opencv2/opencv.hpp>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "picam.hh"
#include "sgbm.hh"
#include "calibration.hh"
#include <iostream>

#define PORT 1234 // Port for communication

bool recvAll(int sockfd, void* data, size_t size) {
    size_t totalReceived = 0;
    char* dataPtr = static_cast<char*>(data);

    while (totalReceived < size) {
        ssize_t bytesReceived = recv(sockfd, dataPtr + totalReceived, size - totalReceived, 0);
        if (bytesReceived <= 0) {
            perror("recv failed");
            return false;
        }
        totalReceived += bytesReceived;
    }
    return true;
}

cv::Mat receiveImage(int sockfd) {
    uint32_t size;

    // Receive the size of the image
    if (!recvAll(sockfd, &size, sizeof(size))) {
        std::cerr << "Error receiving image size\n";
        return cv::Mat();
    }

    // Receive the image data
    std::vector<uchar> buffer(size);
    if (!recvAll(sockfd, buffer.data(), size)) {
        std::cerr << "Error receiving image data\n";
        return cv::Mat();
    }

    // Decode the image
    return cv::imdecode(buffer, cv::IMREAD_COLOR);
}

int main(int argc, char* argv[]) {
    int sock = 0;
    struct sockaddr_in serv_addr;

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        std::cerr << "Socket creation error\n";
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0)
    {
        std::cerr << "Invalid address\n";
        return -1;
    }

    // Connect to the slave server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        std::cerr << "Connection failed\n";
        return -1;
    }


    cv::Mat image;
    rpicamopencv::PiCamStill cam;

    auto fbInfo = getFramebufferInfo();

    cam.options = cam.GetOptions();
    if (cam.options->Parse(argc, argv)) { //test if options have been found and set
	    if (cam.options->verbose >= 2) {
		    cam.options->Print();  //show options used
	    }
    }
    // Set capture options
    // cam.options->verbose=true;
    // cam.options->width = fbInfo.width;
    // cam.options->height = fbInfo.height;

    cam.options->width = 640;
    cam.options->height = 480;

    cam.startPiCamStill();

    // On recoit le struct CalibrationInfo du slave askip
    struct CalibrationInfo slave;
    struct CalibrationInfo master;

    master = calibrate(cam);
    cv::Mat color_corrected;

    struct RectifyMaps maps;
    maps = stereoCalibrate(master, slave);

    while (true)
    {
        // Send the request to the slave (request for image)
        send(sock, "1", 1, 0);

        // Receive the image
        cv::Mat slave = receiveImage(sock);
	    cv::Mat master = take_picture(cam);

	    auto disp = compute_sgbm(master, slave, maps);


        // Normalize the disparity map to 8-bit before resizing
        cv::Mat disp8;
        disp.convertTo(disp8, CV_8U, 255.0 / (64 * 16.0)); // Scale to [0, 255]

        color_corrected = convertToRGB565(disp8);

        cv::Mat resizedImage;
        cv::resize(color_corrected, resizedImage, cv::Size(fbInfo.width, fbInfo.height));

        display_to_fb(color_corrected);
    }

    close(sock); // This will never be reached because we're in an infinite loop
    return 0;
}
