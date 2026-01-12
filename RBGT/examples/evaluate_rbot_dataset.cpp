// SPDX-License-Identifier: MIT
// Copyright (c) 2020 Manuel Stoiber, German Aerospace Center (DLR)

#include "evaluator.h"

int main() {
  // Change accordingly
  std::string dataset_path{"/home/vic/code/3DObjectTracking/data/RBOT_dataset/"};
  std::string result_path{"/home/vic/code/3DObjectTracking/data/RBOT_results/"};

  // std::vector<std::string> body_names{
  //     "ape",  "bakingsoda", "benchviseblue", "broccolisoup", "cam",
  //     "can",  "cat",        "clown",         "cube",         "driller",
  //     "duck", "eggbox",     "glue",          "iron",         "koalacandy",
  //     "lamp", "phone",      "squirrel"};

  std::vector<std::string> body_names{
      "ape"};

  // std::vector<std::string> sequence_names{
  //     "a_regular", "b_dynamiclight", "c_noisy", "d_occlusion", "d_occlusion"};
  std::vector<std::string> sequence_names{
      "a_regular"};

  // std::vector<bool> use_occlusions{false, false, false, false, true};
  std::vector<bool> use_occlusions{false};

  // std::vector<std::string> result_folders{"regular", "dynamic_light", "noisy",
  //                                         "unmodelled_occlusions",
  //                                         "modelled_occlusions"};

  std::vector<std::string> result_folders{"regular"};


  Evaluator evaluator;
  evaluator.set_visualize_all_results(true);
  for (size_t i = 0; i < sequence_names.size(); ++i) {
    evaluator.Init(dataset_path, body_names, sequence_names[i],
                   use_occlusions[i]);
    evaluator.viewer_ptr()->StartSavingImages(result_path + "frames_" + sequence_names[i] + "/");
    evaluator.viewer_ptr()->set_opacity(1.0f);
    evaluator.Evaluate();
    evaluator.SaveResults(result_path + result_folders[i]);
  }
}
