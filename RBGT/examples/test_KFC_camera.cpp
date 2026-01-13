
#include <rbgt/KFC_camera.h>
#include <rbgt/KFC_stereo.h>
#include <opencv2/highgui.hpp>
#include <iostream>

int main() {
    // 1. 实例化，0 通常是笔记本内置摄像头
    rbgt::KFCStereo stereo("2", "calib.yaml");
    rbgt::KFCCamera camera_left("left_camera", &stereo, true);
    rbgt::KFCCamera camera_right("right_camera", &stereo, false);


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