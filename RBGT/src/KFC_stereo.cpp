#include "rbgt/KFC_stereo.h"

namespace rbgt {

KFCStereo::KFCStereo(std::string dev, const std::string &calib_file)
{
    std::cout << "KFCStereo" << std::endl;
    ReadCalibrationData(calib_file, stereo_params_);
    initRectifyMaps(stereo_params_, rectify_maps_);

    camera_ = (KCamera*)createKCamera();

    if (camera_->connect(dev.c_str()) < 0) {
        std::cerr << "Error: Unable to connect to KFC camera at " << dev << std::endl;
    }

    camera_->getResolution(raw_width_, raw_height_);
    raw_stereo_width_ = raw_width_ * 2;

    output_height_ = 540;
    output_stereo_width_ = raw_stereo_width_ * output_height_ / raw_height_;
    output_width_ = output_stereo_width_ / 2;

    jpeg_buffer_.resize(raw_width_ * raw_height_ * 3);
    full_frame_.create(output_height_, output_stereo_width_, CV_8UC3);
    rectified_left_.create(output_height_, output_width_, CV_8UC3);
    rectified_right_.create(output_height_, output_width_, CV_8UC3);

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
        applyRectify(
            full_frame_(cv::Rect(0, 0, raw_width_, raw_height_)),
            full_frame_(cv::Rect(raw_width_, 0, raw_width_, raw_height_)),
            rectify_maps_,
            rectified_left_,
            rectified_right_
        );
        cv::resize(rectified_left_, rectified_left_, cv::Size(output_width_, output_height_));
        cv::resize(rectified_right_, rectified_right_, cv::Size(output_width_, output_height_));
    }
    return true;
}

cv::Mat KFCStereo::UpdateLeftImages_withCapture()
{
    Capture();
    cv::Mat left_image = rectified_left_.clone();
    return left_image;
}

cv::Mat KFCStereo::UpdateRightImages_withoutCapture()
{
    cv::Mat right_image = rectified_right_.clone();
    return right_image;
}

bool KFCStereo::ReadCalibrationData(const std::string &calib_file, StereoParams& p)
{
    std::cout << "Using hardcoded calibration data. File path is ignored: " << calib_file << std::endl;

    // 图像尺寸
    p.image_size = cv::Size(1920, 1080);

    // 左相机内参矩阵 (cam1_k)
    p.cam_k1 = (cv::Mat_<double>(3, 3) <<
        971.919094032262, 0.0, 955.668802292051,
        0.0, 971.950001716095, 532.827188380593,
        0.0, 0.0, 1.0);

    // 右相机内参矩阵 (cam2_k)
    p.cam_k2 = (cv::Mat_<double>(3, 3) <<
        972.838320289128, 0.0, 946.706055796104,
        0.0, 973.033812251345, 525.997757496531,
        0.0, 0.0, 1.0);

    // 左相机畸变系数 (dist_1)
    p.dist1 = (cv::Mat_<double>(1, 5) <<
        0.013028066547, -0.054557745379, 0.000281372503, -0.000470137150, 0.035016324176);

    // 右相机畸变系数 (dist_2)
    p.dist2 = (cv::Mat_<double>(1, 5) <<
        0.007651457554, -0.049792990926, -0.000922724396, 0.000177871464, 0.034022057153);

    // 右相机相对于左相机的旋转矩阵 (R_l_r)
    p.R = (cv::Mat_<double>(3, 3) <<
        0.999945571018, -0.010394026349, 0.000905106300,
        0.010395558833, 0.999944510721, -0.001705237538,
        -0.000887331792, 0.001714553810, 0.999998136472);

    // 右相机相对于左相机的平移向量 (t_l_r) - 注意单位是米！
    // 你的文件里是 -100.06，这看起来是毫米，所以这里除以 1000
    p.t = (cv::Mat_<double>(3, 1) <<
        -100.060854765175 / 1000.0,
        -0.540883826948 / 1000.0,
        0.038299342702 / 1000.0);

    return true;
}

bool KFCStereo::initRectifyMaps(const StereoParams& p, StereoRectifyMaps& maps) {
    std::cout << "Initializing rectify maps..." << std::endl;
    std::cout << "Image size: " << p.image_size << std::endl;
    // 检查输入参数是否合法
    if (p.cam_k1.empty() || p.R.empty() || p.t.empty()) return false;

    // 1. 中间变量：用于存储 stereoRectify 计算出的变换矩阵
    cv::Mat R1, R2, P1, P2, Q;
    cv::Rect validPixROI1, validPixROI2;

    // 2. 执行双目校正计算
    // flags=cv::CALIB_ZERO_DISPARITY (1024) 强制使主点高度一致 (cy1 = cy2)
    // alpha=0 意味着缩放图像以去除所有黑边
    cv::stereoRectify(p.cam_k1, p.dist1, p.cam_k2, p.dist2, 
                      p.image_size, p.R, p.t, 
                      R1, R2, P1, P2, Q, 
                      cv::CALIB_ZERO_DISPARITY, 0, p.image_size, 
                      &validPixROI1, &validPixROI2);

    // 3. 提取并输出 RectifiedCameraInfo (核心 5 参数)
    // 从 P2 (右投影矩阵) 中提取，因为 P1 和 P2 的 fx, fy, cy 是一样的
    maps.info.fx = P2.at<double>(0, 0);
    maps.info.fy = P2.at<double>(1, 1);
    maps.info.cx = P2.at<double>(0, 2);
    maps.info.cy = P2.at<double>(1, 2);
    
    // 基线计算：Baseline = |Tx| = |P2[0,3] / fx|
    // 注意：P2.at<double>(0, 3) = -fx * baseline
    maps.info.baseline = std::abs(P2.at<double>(0, 3) / P2.at<double>(0, 0));

    // 4. 生成 Remap 映射表 (用于 applyRectify 函数)
    // 使用 CV_32FC1 可以获得较好的性能与精度平衡
    cv::initUndistortRectifyMap(p.cam_k1, p.dist1, R1, P1, p.image_size, CV_32FC1, maps.map_l_x, maps.map_l_y);
    cv::initUndistortRectifyMap(p.cam_k2, p.dist2, R2, P2, p.image_size, CV_32FC1, maps.map_r_x, maps.map_r_y);

    std::cout << "Rectify maps initialized successfully." << std::endl;
    std::cout << "fx: " << maps.info.fx << std::endl;
    std::cout << "fy: " << maps.info.fy << std::endl;
    std::cout << "cx: " << maps.info.cx << std::endl;
    std::cout << "cy: " << maps.info.cy << std::endl;
    std::cout << "baseline: " << maps.info.baseline << std::endl;
    return !maps.map_l_x.empty();
}

void KFCStereo::applyRectify(const cv::Mat& src_l, const cv::Mat& src_r, 
                  const StereoRectifyMaps& maps,
                  cv::Mat& out_l, cv::Mat& out_r) 
{
    // 使用双线性插值 (INTER_LINEAR) 进行重投影
    // 这是性能与画质最均衡的选择
    cv::remap(src_l, out_l, maps.map_l_x, maps.map_l_y, cv::INTER_LINEAR);
    cv::remap(src_r, out_r, maps.map_r_x, maps.map_r_y, cv::INTER_LINEAR);
}

}
