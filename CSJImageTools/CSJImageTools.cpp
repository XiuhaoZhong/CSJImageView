// CSJImageTools.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>

#include "libyuv.h"

#include "common.hpp"
#include "CSJStbiTool.h"

int main() {
	int w = 0, h = 0, comp, req_comp = 0;
	std::string filePath = ResourcePath("resources\\originImages\\slamDumk_images\\slamDumk_yinmu_icon.jpg");

	unsigned char *res = CSJStbiTool::loadJpg(&filePath, w, h, comp, req_comp);

	for (int dx = 0; dx < 10; ++dx) {
		res[comp * w * 10 + dx * comp + 0] = 255;
		res[comp * w * 10 + dx * comp + 1] = 0;
		res[comp * w * 10 + dx * comp + 2] = 0;
	}

	std::string fileName("yinmu_01.jpg");
	int wr = CSJStbiTool::writeImage(&fileName, w, h, comp, res, 0);

	std::cout << "stbi_write result: " << wr << std::endl;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
