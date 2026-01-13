#ifndef RBGT_INCLUDE_RBGT_KFC_CAMERA_H_
#define RBGT_INCLUDE_RBGT_KFC_CAMERA_H_

#include <rbgt/camera.h>
#include <opencv2/opencv.hpp>
#include "../../../lib/kcamera.h"
#include <memory>
using namespace KINGFISHER;

namespace rbgt {

class KFCCamera : public Camera {
 public:
    // 构造函数：需要知道这是左目还是右目
    KFCCamera(const std::string &name, std::string dev_path, bool is_left);

    bool Init();
    bool UpdateImage() override;

 private:
    // --- 共享资源 (所有实例共用) ---
    struct SharedHardware {
        KCamera* camera = 0;
        cv::Mat full_frame;       // 包含左右目的大图
        //std::mutex mtx;           // 暂不考虑线程安全
    };

    static std::shared_ptr<SharedHardware> hardware_ptr_;
    static int active_instances_; // 记录有多少个实例在使用硬件

    // --- 实例私有成员 ---
    std::string dev_path_;
    bool is_left_;
};

}  // namespace rbgt
#endif