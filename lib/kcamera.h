#pragma once

#ifndef __KINGFISHER_KH__
#define __KINGFISHER_KH__
#include<string>
#include <functional>
#ifdef _WIN32 
#include<Windows.h>
#elif __linux
#include <unistd.h> 
#endif
extern "C" {
	namespace KINGFISHER {

		using namespace std;
		using CameraCallback = std::function<void(const std::string&)>;

#ifdef _WIN32 
#define KINGFISHER_API __declspec(dllexport)

#elif __linux
#define KINGFISHER_API 
#endif

		class KCamera
		{
		public:
			KCamera() = default;
			virtual ~KCamera() = default;
			KCamera(const KCamera&) = delete;
			KCamera& operator=(const KCamera&) = delete;


			virtual int connect(const std::string device_name) = 0;

			virtual int disConnect() = 0;

			virtual int capture(unsigned char* left_image, unsigned char* right_image) = 0;

			virtual int capture_jpeg(unsigned char* combined_image, int& buffer_size) = 0;

			virtual int setCalibData(std::string file_path) = 0;

			virtual int getCalibData(std::string& calib_file) = 0;

			virtual int getMac(std::string& mac) = 0;

			virtual int getResolution(int& width, int& height) = 0;

			virtual int setExposure(int value) = 0;

			virtual int getExposure(int& value) = 0;

			virtual int setAutoExposure(bool enable) = 0;

			virtual int setAutoWhiteBalance(bool enable) = 0;

			virtual int getVersion(std::string& version)=0;

			virtual void setCallback(CameraCallback cb) {}
		};

		KINGFISHER_API void* createKCamera();
		KINGFISHER_API void destroyKCamera(void*);
		KINGFISHER_API int relayPowerOn();
		KINGFISHER_API int relayPowerOff();

	}
}
#endif

