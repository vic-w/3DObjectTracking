#ifndef RBGT_INCLUDE_RBGT_KFC_STEREO_H_
#define RBGT_INCLUDE_RBGT_KFC_STEREO_H_

#include "../../../lib/kcamera.h"
#include <opencv2/opencv.hpp>

using namespace KINGFISHER;

namespace rbgt {

struct StereoParams {
    cv::Mat cam_k1, cam_k2, dist1, dist2, R, t;
    cv::Size image_size;
};

struct RectifiedCameraInfo {
    double fx;
    double fy;
    double cx;
    double cy;
    double baseline; // 单位与标定文件中的 t_l_r 一致
};

struct StereoRectifyMaps {
    cv::Mat map_l_x, map_l_y;
    cv::Mat map_r_x, map_r_y;
    RectifiedCameraInfo info;
};


class KFCStereo {
 public:
    KFCStereo(std::string dev, const std::string &calib_file);
    cv::Mat UpdateLeftImages_withCapture();
    cv::Mat UpdateRightImages_withoutCapture();

    KCamera* camera_ = 0;
    int raw_stereo_width_ = 0, raw_width_ = 0, raw_height_ = 0;
    int output_stereo_width_ = 0, output_width_ = 0, output_height_ = 0;

    cv::Mat full_frame_;
    cv::Mat rectified_left_;
    cv::Mat rectified_right_;
    std::vector<unsigned char> jpeg_buffer_;

    // rectify之前的参数
    StereoParams stereo_params_;
    StereoRectifyMaps rectify_maps_;

 private:

    bool ReadCalibrationData(const std::string &calib_file, StereoParams& p);
    bool Capture();
    bool initRectifyMaps(const StereoParams& p, StereoRectifyMaps& maps);
    void applyRectify(const cv::Mat& src_l, const cv::Mat& src_r, 
                  const StereoRectifyMaps& maps,
                  cv::Mat& out_l, cv::Mat& out_r);

};

}


#endif  // RBGT_INCLUDE_RBGT_KFC_STEREO_H_