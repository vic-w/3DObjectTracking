#include "rbgt/KFC_camera.h"
#include <iostream>

namespace rbgt {


KFCCamera::KFCCamera(const std::string &name, KFCStereo* stereo, bool is_left)
{
    stereo_ = stereo;
    name_ = name;
    is_left_ = is_left;

    intrinsics_.fu = 496.0395;
    intrinsics_.fv = 496.0395;
    intrinsics_.ppu = 475.653;
    intrinsics_.ppv = 264.565;
    intrinsics_.width = 960;
    intrinsics_.height = 540;

    initialized_ = true;
}

bool KFCCamera::UpdateImage() 
{
    if (is_left_) 
    {
        this->image_ = stereo_->UpdateLeftImages_withCapture();
    } else {
        this->image_ = stereo_->UpdateRightImages_withoutCapture();
    }

    return true;
}

} // namespace rbgt