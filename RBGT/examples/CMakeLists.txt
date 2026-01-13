cmake_minimum_required(VERSION 3.10)

add_executable(evaluate_rbot_dataset evaluate_rbot_dataset.cpp evaluator.cpp)
target_link_libraries(evaluate_rbot_dataset PUBLIC rbgt)

add_executable(evaluate_ablation_study evaluate_ablation_study.cpp evaluator.cpp)
target_link_libraries(evaluate_ablation_study PUBLIC rbgt)

add_executable(run_on_recorded_sequence run_on_recorded_sequence.cpp)
    target_link_libraries(run_on_recorded_sequence PUBLIC rbgt)

if(USE_AZURE_KINECT)
    add_executable(record_camera_sequence record_camera_sequence.cpp)
    target_link_libraries(record_camera_sequence PUBLIC rbgt)

    add_executable(run_on_camera_sequence run_on_camera_sequence.cpp)
    target_link_libraries(run_on_camera_sequence PUBLIC rbgt)
endif()

add_executable(run_on_opencv_camera_sequence run_on_opencv_camera_sequence.cpp)
target_link_libraries(run_on_opencv_camera_sequence PUBLIC rbgt)

add_executable(run_on_stereo_sequence run_on_stereo_sequence.cpp)
target_link_libraries(run_on_stereo_sequence PUBLIC rbgt)

add_executable(test_opencv_camera test_opencv_camera.cpp)
target_link_libraries(test_opencv_camera PUBLIC rbgt ${OpenCV_LIBS})

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../lib/)
add_executable(test_KFC_camera test_KFC_camera.cpp)
target_link_libraries(test_KFC_camera PUBLIC rbgt ${OpenCV_LIBS} kfc_cpp)