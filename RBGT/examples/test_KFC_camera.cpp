#include <rbgt/KFC_camera.h>
#include <opencv2/highgui.hpp>
#include <iostream>

int main() {
    // 1. 实例化，0 通常是笔记本内置摄像头
    rbgt::KFCCamera camera_left("camera_left", "2", true);
    rbgt::KFCCamera camera_right("camera_right", "2", false);

    // 2. 初始化相机
    if (!camera_left.Init()) {
        std::cerr << "错误：无法初始化left摄像头。请检查权限或设备索引。" << std::endl;
        return -1;
    }
    if (!camera_right.Init()) {
        std::cerr << "错误：无法初始化right摄像头。请检查权限或设备索引。" << std::endl;
        return -1;
    }

    std::cout << "摄像头已启动。按任意键退出预览窗口。" << std::endl;

    // 3. 实时预览循环
    while (true) {
        // 更新画面
        if (!camera_left.UpdateImage()) {
            std::cerr << "获取left图像失败" << std::endl;
            break;
        }
        if (!camera_right.UpdateImage()) {
            std::cerr << "获取right图像失败" << std::endl;
            break;
        }

        // 获取当前帧并显示
        // 虽然 image() 是基类方法，但它会返回派生类 UpdateImage() 更新后的 image_ 成员
        cv::imshow("Left Camera Test", camera_left.image());
        cv::imshow("Right Camera Test", camera_right.image());

        // 等待 1ms，如果有键盘输入则退出
        if (cv::waitKey(1) >= 0) {
            break;
        }
    }

    return 0;
}