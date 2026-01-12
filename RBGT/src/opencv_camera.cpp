#include <rbgt/opencv_camera.h>
#include <iostream>

namespace rbgt {

OpenCVCamera::OpenCVCamera(const std::string &name, int camera_index)
    : camera_index_{camera_index} {
  // Camera 没有带参构造，这里直接设置名称
  name_ = name;
}

bool OpenCVCamera::Init() {
  // 1) 打开摄像头
  capture_.open(camera_index_, cv::CAP_V4L2);
  if (!capture_.isOpened()) {
    std::cerr << "Cannot open camera: " << camera_index_ << std::endl;
    return false;
  }

  // 2) 设置分辨率（与需要的渲染分辨率一致）
  capture_.set(cv::CAP_PROP_FRAME_WIDTH, 640);
  capture_.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

  // 3) 设置相机内参（示例值，建议用标定结果替换）
  intrinsics_.fu = 580.0f;
  intrinsics_.fv = 580.0f;
  intrinsics_.ppu = 320.0f;
  intrinsics_.ppv = 240.0f;

  // 5) 标记已初始化并抓取第一帧
  initialized_ = true;
  return UpdateImage();
}

bool OpenCVCamera::UpdateImage() {
  capture_ >> image_;
  if (image_.empty()) return false;
  return true;
}

}  // namespace rbgt