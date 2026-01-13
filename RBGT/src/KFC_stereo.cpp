#include "rbgt/KFC_stereo.h"

namespace rbgt {

KFCStereo::KFCStereo(std::string dev, const std::string &calib_file)
{
    ReadCalibrationData(calib_file);

    camera_ = (KCamera*)createKCamera();

    if (camera_->connect(dev.c_str()) < 0) {
        std::cerr << "Error: Unable to connect to KFC camera at " << dev << std::endl;
    }

    camera_->getResolution(raw_width_, raw_height_);
    raw_stereo_width_ = raw_width_ * 2;

    output_height_ = 480;
    output_stereo_width_ = raw_stereo_width_ * output_height_ / raw_height_;
    output_width_ = output_stereo_width_ / 2;

    jpeg_buffer_.resize(raw_width_ * raw_height_ * 3);

    Capture();
}

bool KFCStereo::Capture()
{
    int jpeg_size = 0;
    if (camera_->capture_jpeg(jpeg_buffer_.data(), jpeg_size) < 0) {
        return false;
    }
    //std::cout<<"jpeg size: "<<jpeg_size<<std::endl;

    if (jpeg_size > 0) {
        cv::Mat raw_data(1, jpeg_size, CV_8UC1, jpeg_buffer_.data());
        full_frame_ = cv::imdecode(raw_data, cv::IMREAD_COLOR);
        cv::resize(full_frame_, full_frame_, cv::Size(output_stereo_width_, output_height_));
    }
    return true;
}

cv::Mat KFCStereo::UpdateLeftImages_withCapture()
{
    Capture();
    cv::Mat left_image = full_frame_(cv::Rect(0, 0, output_width_, output_height_)).clone();
    return left_image;
}

cv::Mat KFCStereo::UpdateRightImages_withoutCapture()
{
    cv::Mat right_image = full_frame_(cv::Rect(output_width_, 0, output_width_, output_height_)).clone();
    return right_image;
}

void KFCStereo::ReadCalibrationData(const std::string &calib_file)
{

}

void KFCStereo::Rectify()
{

}

}
