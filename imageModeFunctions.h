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
	std::cout << "[Streaming Service] frame is stored in " << fileName << std::endl;
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
		device_config.camera_fps = K4A_FRAMES_PER_SECOND_15;
		device_config.color_format = K4A_IMAGE_FORMAT_COLOR_MJPG;
		device_config.color_resolution = K4A_COLOR_RESOLUTION_720P;
		device_config.depth_mode = K4A_DEPTH_MODE_WFOV_UNBINNED;
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
			const k4a_image_t color_image = k4a_capture_get_color_image(sensor_capture);
			const k4a_image_t depth_image = k4a_capture_get_depth_image(sensor_capture);
			const k4a_image_t ir_image = k4a_capture_get_ir_image(sensor_capture);

			// Transform depth and ir image
			k4a_image_t transformed_depth_image = NULL;
			k4a_image_t custom_ir_image = NULL;
			k4a_image_t transformed_ir_image = NULL;
			int color_image_width_pixels = k4a_image_get_width_pixels(color_image);
			int color_image_height_pixels = k4a_image_get_height_pixels(color_image);
			int ir_image_width_pixels = k4a_image_get_width_pixels(ir_image);
			int ir_image_height_pixels = k4a_image_get_height_pixels(ir_image);
			if (K4A_RESULT_SUCCEEDED != k4a_image_create_from_buffer(K4A_IMAGE_FORMAT_CUSTOM16, ir_image_width_pixels, ir_image_height_pixels, k4a_image_get_stride_bytes(ir_image),
				k4a_image_get_buffer(ir_image), k4a_image_get_size(ir_image), [](void* _buffer, void* context) {delete[](uint8_t*) _buffer; (void)context; }, NULL, &custom_ir_image))
			{
				errorMessage += "Failed to create custom ir image.\n";
			}
			if (K4A_RESULT_SUCCEEDED != k4a_image_create(K4A_IMAGE_FORMAT_DEPTH16, color_image_width_pixels, color_image_height_pixels, color_image_width_pixels * (int)sizeof(uint16_t), &transformed_depth_image))
			{
				errorMessage += "Failed to create transformed depth image.\n";
			}
			if (K4A_RESULT_SUCCEEDED != k4a_image_create(K4A_IMAGE_FORMAT_CUSTOM16, color_image_width_pixels, color_image_height_pixels, color_image_width_pixels * (int)sizeof(uint16_t), &transformed_ir_image))
			{
				errorMessage += "Failed to create transformed ir image.\n";
			}
			if (K4A_RESULT_SUCCEEDED != k4a_transformation_depth_image_to_color_camera_custom(transformation_handle, depth_image, custom_ir_image, transformed_depth_image, transformed_ir_image, K4A_TRANSFORMATION_INTERPOLATION_TYPE_LINEAR, 0))
			{
				errorMessage += "Failed to compute transformed depth image.\n";
			}

			// Save depth image
			std::ofstream fw("depthImage.txt", std::ofstream::out);
			uint16_t* depth_data = (uint16_t*)(void*)k4a_image_get_buffer(transformed_depth_image);
			uint16_t* ir_data = (uint16_t*)(void*)k4a_image_get_buffer(custom_ir_image);
			for (int i = 0; i < k4a_image_get_width_pixels(transformed_depth_image); i++) {
				for (int j = 0; j < k4a_image_get_height_pixels(transformed_depth_image); j++) {
					fw << depth_data[i* k4a_image_get_height_pixels(transformed_depth_image) + j] << " ";
				}
			}
			fw.close();

			// Save color image
			if (errorMessage == "") {
				std::string colorFileName = "colorImage.jpg";

				writeToFile(colorFileName.c_str(), k4a_image_get_buffer(color_image), k4a_image_get_size(color_image));
			}

			//Release capture and images
			k4a_capture_release(sensor_capture);
			k4a_image_release(color_image);
			k4a_image_release(depth_image);
			k4a_image_release(ir_image);
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