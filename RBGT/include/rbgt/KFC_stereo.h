#ifndef RBGT_INCLUDE_RBGT_KFC_STEREO_H_
#define RBGT_INCLUDE_RBGT_KFC_STEREO_H_

#include "../../../lib/kcamera.h"
#include <opencv2/opencv.hpp>

using namespace KINGFISHER;

namespace rbgt {

class KFCStereo {
 public:
    KFCStereo(std::string dev, const std::string &calib_file);
    cv::Mat UpdateLeftImages_withCapture();
    cv::Mat UpdateRightImages_withoutCapture();

    KCamera* camera_ = 0;
    int raw_stereo_width_ = 0, raw_width_ = 0, raw_height_ = 0;
    int output_stereo_width_ = 0, output_width_ = 0, output_height_ = 0;

    cv::Mat full_frame_;
    std::vector<unsigned char> jpeg_buffer_;

 private:

    void ReadCalibrationData(const std::string &calib_file);
    bool Capture();
    void Rectify();

};

}


#endif  // RBGT_INCLUDE_RBGT_KFC_STEREO_H_