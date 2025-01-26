#pragma once

#include <string>

// This class encapsulates the stb_image API.
class CSJStbiTool {
public:
	CSJStbiTool();
	~CSJStbiTool();

	/*
	 * load a jpg file from file.
	 *
	 * w    in/out image width
	 * h    in/out image height
	 * comp in/out number of channels
	 *
	 * Comments: The memory returned by this function needs to be release by users.
	 *
	 */
	static unsigned char *loadJpg(std::string *filePath, int &w, int &h, int &channel, int desired_channel);

	/*
	 * write a iamge to the file.
	 *
	 * the format indicated by the suffix of the file name.
	 * 
	 * filePath  in the path where the file save
	 * w         in image width
	 * h         in image height
	 * channel   in image color channels, e.g. the channel is 3 for RGB format
	 * data      in image data
	 * quality   in output image quality
	 *
	 */
	static int writeImage(std::string *filePath, int w, int h, int channel, const void *data, int quality);
};

