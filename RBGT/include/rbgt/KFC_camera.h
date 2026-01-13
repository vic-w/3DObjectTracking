#ifndef RBGT_INCLUDE_RBGT_KFC_CAMERA_H_
#define RBGT_INCLUDE_RBGT_KFC_CAMERA_H_

#include <rbgt/camera.h>
#include <opencv2/opencv.hpp>
#include "rbgt/KFC_stereo.h"
#include <memory>

namespace rbgt {

class KFCCamera : public Camera {
 public:
    // 构造函数：需要知道这是左目还是右目
    KFCCamera(const std::string &name, KFCStereo* stereo, bool is_left);
    bool UpdateImage() override;

 private:

    KFCStereo* stereo_;
    bool is_left_;
};

}  // namespace rbgt
#endif