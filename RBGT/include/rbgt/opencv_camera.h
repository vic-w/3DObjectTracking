#ifndef RBGT_INCLUDE_RBGT_OPENCV_CAMERA_H_
#define RBGT_INCLUDE_RBGT_OPENCV_CAMERA_H_

#include <rbgt/camera.h>
#include <opencv2/opencv.hpp>

namespace rbgt {

class OpenCVCamera : public Camera {
 public:
  // camera_index 通常 0 代表内置摄像头
  OpenCVCamera(const std::string &name, int camera_index = 0);
  
  // 覆盖基类 Init（无参）
  bool Init();

  // 每一帧抓取图像的核心函数
  bool UpdateImage() override;

 private:
  cv::VideoCapture capture_;
  int camera_index_;
};

}  // namespace rbgt

#endif  // RBGT_INCLUDE_RBGT_OPENCV_CAMERA_H_