// SPDX-License-Identifier: MIT
// Copyright (c) 2020 Manuel Stoiber, German Aerospace Center (DLR)

#include <rbgt/body.h>
#include <rbgt/common.h>
#include <rbgt/KFC_camera.h>
#include <rbgt/normal_image_viewer.h>
#include <rbgt/occlusion_mask_renderer.h>
#include <rbgt/region_modality.h>
#include <rbgt/renderer_geometry.h>
#include <rbgt/tracker.h>

#include <Eigen/Geometry>
#include <memory>
#include <string>

bool ReadPosesRBOTDataset(const std::filesystem::path &path,
                                     std::vector<rbgt::Transform3fA> *poses) {
  std::ifstream ifs;
  ifs.open(path.string(), std::ios::binary);
  if (!ifs.is_open() || ifs.fail()) {
    ifs.close();
    std::cerr << "Could not open file stream " << path.string() << std::endl;
    return false;
  }

  poses->resize(1001);
  std::string parsed;
  std::getline(ifs, parsed);
  for (auto &pose : *poses) {
    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 3; ++j) {
        std::getline(ifs, parsed, '\t');
        pose.matrix()(i, j) = stof(parsed);
      }
    }
    std::getline(ifs, parsed, '\t');
    pose.matrix()(0, 3) = stof(parsed) * 0.001f;
    std::getline(ifs, parsed, '\t');
    pose.matrix()(1, 3) = stof(parsed) * 0.001f;
    std::getline(ifs, parsed);
    pose.matrix()(2, 3) = stof(parsed) * 0.001f;
  }
  return true;
}

int main() {

  std::string dataset_path{"/home/vic/code/3DObjectTracking/data/RBOT_dataset/"};

  std::vector<rbgt::Transform3fA> poses;
  if (!ReadPosesRBOTDataset(dataset_path + "poses_first.txt", &poses))
    return false;

  auto tracker_ptr{std::make_shared<rbgt::Tracker>()};
  auto renderer_geometry_ptr{std::make_shared<rbgt::RendererGeometry>()};

  rbgt::KFCStereo stereo("2", "calib.yaml");
  auto camera_l_ptr{std::make_shared<rbgt::KFCCamera>("camera_left", &stereo, true)};
  auto camera_r_ptr{std::make_shared<rbgt::KFCCamera>("camera_right", &stereo, false)};

  // 设置右相机的 world2camera_pose，将其向右移动 10cm
  rbgt::Transform3fA camera_r_pose = rbgt::Transform3fA::Identity();
  camera_r_pose.translation() = Eigen::Vector3f(-0.1f, 0.0f, 0.0f);  // 向右移动 10cm
  camera_r_ptr->set_world2camera_pose(camera_r_pose);

  auto viewer_l_ptr = std::make_shared<rbgt::NormalImageViewer>();
  viewer_l_ptr->Init("viewer_left", renderer_geometry_ptr, camera_l_ptr);

  auto viewer_r_ptr = std::make_shared<rbgt::NormalImageViewer>();
  viewer_r_ptr->Init("viewer_right", renderer_geometry_ptr, camera_r_ptr);

  tracker_ptr->AddViewer(viewer_l_ptr);
  tracker_ptr->AddViewer(viewer_r_ptr);
  tracker_ptr->set_visualization_time(1);

  auto body_ptr = std::make_shared<rbgt::Body>(
    "body", 
    "/home/vic/code/3DObjectTracking/data/RBOT_dataset/ape/ape.obj", 
    0.001f,
    true, 
    false, 
    0.3f
  );
  renderer_geometry_ptr->ClearBodies();
  renderer_geometry_ptr->AddBody(body_ptr);

  auto model_ptr = std::make_shared<rbgt::Model>("model");
  if (!model_ptr->LoadModel(".", "ape_model")) {
    model_ptr->GenerateModel(
      *body_ptr, 
      0.8f, //sphere_radius_
      4, //n_divides_
      200 //n_points_
    );
    model_ptr->SaveModel(".", "ape_model");
  }

  auto region_modality_l_ptr = std::make_shared<rbgt::RegionModality>();
  region_modality_l_ptr->Init(
    "region_modality_left", 
    body_ptr, 
    model_ptr,
    camera_l_ptr);

  auto region_modality_r_ptr = std::make_shared<rbgt::RegionModality>();
  region_modality_r_ptr->Init(
    "region_modality_right", 
    body_ptr, 
    model_ptr,
    camera_r_ptr);

  tracker_ptr->AddRegionModality(region_modality_l_ptr);
  tracker_ptr->AddRegionModality(region_modality_r_ptr);
  tracker_ptr->SetUpObjects();
  tracker_ptr->UpdateCameras();

  body_ptr->set_body2world_pose(poses[0]);
  region_modality_l_ptr->StartModality();
  region_modality_r_ptr->StartModality();
  region_modality_l_ptr->set_visualize_points_pose_update(true);
  region_modality_r_ptr->set_visualize_points_pose_update(true);

  bool image_available = true;
  while(image_available)
  {
    tracker_ptr->CalculateBeforeCameraUpdate();
    image_available = tracker_ptr->UpdateCameras();

    for (int corr_iteration = 0; corr_iteration < 7; ++corr_iteration) 
    {
      //std::cout<<"corr_iteration: " << corr_iteration << std::endl;
      tracker_ptr->CalculateCorrespondences(corr_iteration);

      int corr_save_idx = corr_iteration;
      tracker_ptr->VisualizeCorrespondences(corr_save_idx);

      for (int update_iteration = 0; update_iteration < 2; ++update_iteration) {
        //std::cout<<"  update_iteration: " << update_iteration << std::endl;
        tracker_ptr->CalculatePoseUpdate();

        int update_save_idx = corr_save_idx * 2 + update_iteration;
        tracker_ptr->VisualizePoseUpdate(update_save_idx);
      }
    }
  }
  return 0;
}
