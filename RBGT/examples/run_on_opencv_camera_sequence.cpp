// SPDX-License-Identifier: MIT
// Copyright (c) 2020 Manuel Stoiber, German Aerospace Center (DLR)

#include <rbgt/body.h>
#include <rbgt/common.h>
#include <rbgt/opencv_camera.h>
#include <rbgt/dataset_rbot_camera.h>
#include <rbgt/normal_image_viewer.h>
#include <rbgt/occlusion_mask_renderer.h>
#include <rbgt/region_modality.h>
#include <rbgt/renderer_geometry.h>
#include <rbgt/tracker.h>

#include <Eigen/Geometry>
#include <memory>
#include <string>

bool ReadFirstPoseRBOTDataset(const std::filesystem::path &path,
                          rbgt::Transform3fA &pose) 
{
  std::ifstream ifs;
  ifs.open(path.string(), std::ios::binary);
  if (!ifs.is_open() || ifs.fail()) {
    ifs.close();
    std::cerr << "Could not open file stream " << path.string() << std::endl;
    return false;
  }

  std::string parsed;
  std::getline(ifs, parsed);

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

  return true;
}

int main() {

  rbgt::Transform3fA reset_pose;
  if (!ReadFirstPoseRBOTDataset("/home/vic/code/3DObjectTracking/data/RBOT_dataset/poses_first.txt", reset_pose))
    return false;

  auto tracker_ptr{std::make_shared<rbgt::Tracker>()};
  auto renderer_geometry_ptr{std::make_shared<rbgt::RendererGeometry>()};

  auto camera_ptr{std::make_shared<rbgt::OpenCVCamera>("OpenCV Camera", 0)};
  camera_ptr->Init();

  auto viewer_ptr = std::make_shared<rbgt::NormalImageViewer>();
  viewer_ptr->Init("viewer", renderer_geometry_ptr, camera_ptr);

  tracker_ptr->AddViewer(viewer_ptr);
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

  auto region_modality_ptr = std::make_shared<rbgt::RegionModality>();
  region_modality_ptr->Init(
    "region_modality", 
    body_ptr, 
    model_ptr,
    camera_ptr);

  tracker_ptr->AddRegionModality(region_modality_ptr);

  body_ptr->set_body2world_pose(reset_pose);
  region_modality_ptr->StartModality();
  region_modality_ptr->set_visualize_points_pose_update(true);

  tracker_ptr->SetUpObjects();

  while(1)
  {
    tracker_ptr->CalculateBeforeCameraUpdate();
    tracker_ptr->UpdateCameras();

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
