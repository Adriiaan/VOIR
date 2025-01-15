#include "picam.hh"


struct FramebufferInfo getFramebufferInfo()
{
    std::string command = "fbset | grep geometry";
    FILE *pipe = popen(command.c_str(), "r");
    if (!pipe)
    {
        throw std::runtime_error(
            "Error: Could not retrieve framebuffer information. fbset not "
            "found (install it with 'sudo apt install fbset') or failed.");
    }

    char buffer[128];
    FramebufferInfo fbInfo = { 0, 0, 0 }; // Initialize with default values

    if (fgets(buffer, sizeof(buffer), pipe))
    {
        std::istringstream iss(buffer);
        std::string tmp;
        iss >> tmp >> fbInfo.width >> fbInfo.height >> tmp >> tmp
            >> fbInfo.depth;
    }
    pclose(pipe);
    return fbInfo;
}

cv::Mat convertToRGB565(const cv::Mat &image)
{
    cv::Mat image565(image.size(), CV_16UC1);
    for (int y = 0; y < image.rows; y++)
    {
        for (int x = 0; x < image.cols; x++)
        {
            cv::Vec3b color = image.at<cv::Vec3b>(y, x);
            uint16_t r = color[2] >> 3; // 5 bits
            uint16_t g = color[1] >> 2; // 6 bits
            uint16_t b = color[0] >> 3; // 5 bits
            image565.at<uint16_t>(y, x) = (r << 11) | (g << 5) | b;
        }
    }
    return image565;
}

cv::Mat take_picture(rpicamopencv::PiCamStill &cam)
{
    cv::Mat image;
    cam.captureStillImage(image);
    return image;
}

void display_to_fb(cv::Mat& image) {
	static struct FramebufferInfo fbInfo = getFramebufferInfo();
	cv::Mat resizedImage;
	cv::resize(image, resizedImage, cv::Size(fbInfo.width, fbInfo.height));

	cv::Mat colored = convertToRGB565(resizedImage);

	std::ofstream fbOut("/dev/fb0", std::ios::binary);
	if (!fbOut.is_open())
		return;
	fbOut.write(reinterpret_cast<const char *>(colored.data),
			colored.total() * colored.elemSize());
	fbOut.close();
}
