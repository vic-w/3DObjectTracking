#include "rbgt/KFC_camera.h"
#include <iostream>

namespace rbgt {

// 静态成员初始化
std::shared_ptr<KFCCamera::SharedHardware> KFCCamera::hardware_ptr_ = nullptr;
int KFCCamera::active_instances_ = 0;

KFCCamera::KFCCamera(const std::string &name, std::string dev_path, bool is_left)
{
    name_ = name;
    dev_path_ = dev_path;
    is_left_ = is_left;
    active_instances_++;
}

bool KFCCamera::Init() 
{    
    intrinsics_.fu = 580;
    intrinsics_.fv = 580;
    intrinsics_.ppu = 320;
    intrinsics_.ppv = 240;
    intrinsics_.width = 853;
    intrinsics_.height = 480;

    std::cout << "Initializing KFCCamera..." << std::endl;
    if (hardware_ptr_ == nullptr) {
        hardware_ptr_ = std::make_shared<SharedHardware>();
        
        // 创建 KCamera 实例 (假设 createKCamera 是全局函数)
        hardware_ptr_->camera = (KCamera*)createKCamera();
        if (!hardware_ptr_->camera) return false;

        // 连接相机，这里假设 dev_path_ 对应设备路径，如 "/dev/video0"
        if (hardware_ptr_->camera->connect(dev_path_.c_str()) < 0) {
            return false;
        }
    }
    initialized_ = true;
    std::cout << "KFCCamera initialized successfully." << std::endl;
    int width = 0, height = 0;
    hardware_ptr_->camera->getResolution(width, height);
    std::vector<unsigned char> jpeg_buffer(width * height * 3);
    int jpeg_size = 0;

    // 1. 从硬件获取 JPEG 数据
    if (hardware_ptr_->camera->capture_jpeg(jpeg_buffer.data(), jpeg_size) < 0) {
        return false;
    }
    //std::cout << "抓取到 JPEG 数据，大小：" << jpeg_size << " 字节" << std::endl;
    return true;
}


bool KFCCamera::UpdateImage() {
    //std::cout << "Updating image for " << (is_left_ ? "left" : "right") << " camera." << std::endl;
    if (!hardware_ptr_ || !hardware_ptr_->camera) return false;

    if (is_left_) 
    {
        int width = 0, height = 0;
        hardware_ptr_->camera->getResolution(width, height);
        int target_height = 480;
        int target_width = width * target_height / height * 2;

        // 临时 buffer 用于存放抓取的 JPEG 数据
        // 建议在 SharedHardware 中复用此 buffer 以提高性能
        std::vector<unsigned char> jpeg_buffer(width * height * 3);
        int jpeg_size = 0;

        // 1. 从硬件获取 JPEG 数据
        if (hardware_ptr_->camera->capture_jpeg(jpeg_buffer.data(), jpeg_size) < 0) {
            return false;
        }
        //std::cout << "抓取到 JPEG 数据，大小：" << jpeg_size << " 字节" << std::endl;
        // 2. 解码到共享的 full_frame
        if (jpeg_size > 0) {
            cv::Mat raw_data(1, jpeg_size, CV_8UC1, jpeg_buffer.data());
            hardware_ptr_->full_frame = cv::imdecode(raw_data, cv::IMREAD_COLOR);
            cv::resize(hardware_ptr_->full_frame, hardware_ptr_->full_frame, cv::Size(target_width, target_height));
        }
        if (hardware_ptr_->full_frame.empty()) return false;
    }

    // --- 策略：所有实例（包括第一个）都从共享的大图中裁剪自己的部分 ---
    int total_width = hardware_ptr_->full_frame.cols;
    int total_height = hardware_ptr_->full_frame.rows;
    int half_width = total_width / 2;

    if (is_left_) {
        // 这里的 image_ 是基类 Camera 的成员变量
        this->image_ = hardware_ptr_->full_frame(cv::Rect(0, 0, half_width, total_height)).clone();
    } else {
        this->image_ = hardware_ptr_->full_frame(cv::Rect(half_width, 0, half_width, total_height)).clone();
    }

    return true;
}

} // namespace rbgt