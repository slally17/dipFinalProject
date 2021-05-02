#pragma once

#include <k4a/k4a.h>
#include <k4arecord/record.h>
#include <k4arecord/playback.h>
#include <k4abt.h>

#include <iostream>
#include <string>

void writeToFile(const char* fileName, void* buffer, size_t bufferSize) {
	assert(buffer != NULL);

	std::ofstream hFile;
	hFile.open(fileName, std::ios::out | std::ios::trunc | std::ios::binary);
	if (hFile.is_open())
	{
		hFile.write((char*)buffer, static_cast<std::streamsize>(bufferSize));
		hFile.close();
	}
	std::cout << "[Streaming Service] Color frame is stored in " << fileName << std::endl;
}

std::string imageModeFunction() {
	std::string errorMessage = "";
	uint32_t kinectCount = k4a_device_get_installed_count();

	if (kinectCount == 1 && errorMessage == "") { //Run program if Kinect is found
		//Connect to the Kinect
		k4a_device_t device = NULL;
		if (K4A_FAILED(k4a_device_open(K4A_DEVICE_DEFAULT, &device))) {
			errorMessage += "Kinect was found by program, but can't connect. Please try reconnecting.\n";
		}

		//Initialize the Kinect
		k4a_device_configuration_t device_config = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
		device_config.camera_fps = K4A_FRAMES_PER_SECOND_30;
		device_config.color_format = K4A_IMAGE_FORMAT_COLOR_MJPG;
		device_config.color_resolution = K4A_COLOR_RESOLUTION_720P;
		device_config.depth_mode = K4A_DEPTH_MODE_NFOV_UNBINNED;
		device_config.synchronized_images_only = true;
		k4a_calibration_t sensor_calibration;
		if (errorMessage == "" && K4A_FAILED(k4a_device_get_calibration(device, device_config.depth_mode, device_config.color_resolution, &sensor_calibration))) {
			errorMessage += "Get depth camera calibration failed. \n";
		}
		k4a_transformation_t transformation_handle = NULL;
		transformation_handle = k4a_transformation_create(&sensor_calibration);

		//Start recording
		if (errorMessage == "" && K4A_FAILED(k4a_device_start_cameras(device, &device_config))) {
			errorMessage += "Kinect camera failed to start, please try reconnecting.\n";
			k4a_device_close(device);
		}

		//Get current frame
		k4a_capture_t sensor_capture;
		k4a_wait_result_t get_capture_result = k4a_device_get_capture(device, &sensor_capture, K4A_WAIT_INFINITE);


		//Process current frame
		if (get_capture_result == K4A_WAIT_RESULT_SUCCEEDED) {
			// Get color and depth images
			k4a_image_t color_image = k4a_capture_get_color_image(sensor_capture);
			k4a_image_t depth_image = k4a_capture_get_depth_image(sensor_capture);
			
			// Transform depth image
			k4a_image_t transformed_depth_image = NULL;
			int color_image_width_pixels = k4a_image_get_width_pixels(color_image);
			int color_image_height_pixels = k4a_image_get_height_pixels(color_image);			
			if (K4A_RESULT_SUCCEEDED != k4a_image_create(K4A_IMAGE_FORMAT_DEPTH16, color_image_width_pixels, color_image_height_pixels,	color_image_width_pixels * (int)sizeof(uint16_t), &transformed_depth_image))
			{
				errorMessage += "Failed to create transformed depth image.\n";
			}
			if (K4A_RESULT_SUCCEEDED !=	k4a_transformation_depth_image_to_color_camera(transformation_handle, depth_image, transformed_depth_image))
			{
				errorMessage += "Failed to compute transformed depth image.\n";
			}

			// Save color and transformed depth image
			if (errorMessage == "") {
				std::string depthFileName = "depthfile.bin";
				std::string colorFileName = "colorfile.jpg";

				writeToFile(depthFileName.c_str(), k4a_image_get_buffer(depth_image), k4a_image_get_size(depth_image));
				writeToFile(colorFileName.c_str(), k4a_image_get_buffer(color_image), k4a_image_get_size(color_image));
			}

			//Release capture and images
			k4a_capture_release(sensor_capture);
			k4a_image_release(color_image);
			k4a_image_release(depth_image);
			k4a_image_release(transformed_depth_image);
		}


		//Stop Kinect
		k4a_transformation_destroy(transformation_handle);
		k4a_device_stop_cameras(device);
		k4a_device_close(device);
	}
	else if (kinectCount == 0) { //End program if Kinect isn't found
		errorMessage += "Kinect can't be found by program, please try reconnecting.\n";
	}
	else if (errorMessage == "") { //End program if multiple Kinects are found
		errorMessage += "Multiple Kinects, detected. Please unplug additional ones.\n";
	}

	return errorMessage;
}