#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <rpicamopencv/rpicamopencv.hpp>
#include "picam.hh"

#define PORT 1234 // Port for communication

void sendAll(int sockfd, const void* data, size_t size) {
    size_t totalSent = 0;
    const char* dataPtr = static_cast<const char*>(data);

    while (totalSent < size) {
        ssize_t bytesSent = send(sockfd, dataPtr + totalSent, size - totalSent, 0);
        if (bytesSent <= 0) {
            perror("send failed");
            return;
        }
        totalSent += bytesSent;
    }
}

void sendImage(cv::Mat& image, int sockfd) {
    std::vector<uchar> buf;
    cv::imencode(".jpg", image, buf);  // Encode image as JPEG
    uint32_t size = buf.size();

    // Send the size of the image first
    sendAll(sockfd, &size, sizeof(size));

    // Send the image data
    sendAll(sockfd, buf.data(), buf.size());
}

bool handleRequest(rpicamopencv::PiCamStill& cam, int client_socket)
{
    char buffer;

    ssize_t bytesRead = read(client_socket, &buffer, 1);
    if (bytesRead <= 0)
    {
        std::cerr << "Error receiving request or client disconnected\n";
        return false;
    }

    cv::Mat image = take_picture(cam);

    if (image.empty())
    {
        std::cerr << "Error loading image!\n";
        return true;
    }

    // Send the image
    sendImage(image, client_socket);
    return true;
}

int main(int argc, char* argv[])
{
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

    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for connections (allowing multiple clients)
    if (listen(server_fd, 3) < 0)
    {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "Server is ready. Waiting for connections...\n";

connect:
    // Accept the first client connection
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                             (socklen_t *)&addrlen))
        < 0)
    {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "Client connected. Handling requests...\n";

    // Main loop to handle multiple requests from the same client
    while (true)
    {
        if (!handleRequest(cam, new_socket))
            goto connect;
    }

    close(new_socket); // This will never be reached since we're in an infinite
                       // loop
    close(server_fd);
    return 0;
}
