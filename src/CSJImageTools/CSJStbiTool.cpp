
#include "CSJStbiTool.h"

#include <string>

#include "common.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

CSJStbiTool::CSJStbiTool() {

}

CSJStbiTool::~CSJStbiTool() {

}

unsigned char* CSJStbiTool::loadJpg(std::string *filePath, int &w, int &h, int &channel, int desireChannel) {

	unsigned char *res = stbi_load(filePath->c_str(), &w, &h, &channel, desireChannel);
	return res;
}

int CSJStbiTool::writeImage(std::string *filePath, int w, int h, int channel, const void *data, int quality) {
	
	return stbi_write_jpg(filePath->c_str(), w, h, channel, data, quality);
}