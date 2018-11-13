/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                           License Agreement
//
// Copyright (C) 2015 MINAGAWA Takuya.
// Third party copyrights are property of their respective owners.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is furnished to do
// so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
// PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//M*/

#ifndef __UTIL__
#define __UTIL__

#include <string>
#include <vector>
#include <fstream>
#include <opencv2/core/core.hpp>

std::string Int2String(int x);

inline std::vector<std::string> ImageExtentions() {
	std::string image_exts[] = { ".jpg", ".JPG", ".jpeg", ".JPEG", ".bmp", ".BMP",
		".png", ".PNG", ".dib", ".DIB", ".pbm", ".PBM", ".pgm", ".PGM",
		".ppm", ".PPM", ".sr", ".SR", ".ras", ".RAS" };

	return std::vector<std::string>(image_exts, image_exts + sizeof(image_exts) / sizeof(std::string));
}

bool hasExtention(const std::string& file_name, const std::vector<std::string>& extentions);

inline bool hasImageExtention(const std::string& filename) {
	return hasExtention(filename, ImageExtentions());
}


bool ReadImageFilesInDirectory(const std::string& img_dir, std::vector<std::string>& image_lists);

void DrawHistogram(const cv::Mat& histogram, cv::Mat& draw_img, int width);

cv::Mat Convert8UC3(const cv::Mat& src);

cv::Mat ConcatinateImage(const cv::Mat& img1, const cv::Mat& img2, bool hol);

void SaveMatCSV(const std::string& filename, const cv::Mat& mat);

//! std::vectorをCSVファイルとして保存
template<typename T> void SaveVector(const std::string& filename, const std::vector<T>& src_vector)
{
	std::ofstream csv_ofs(filename);
	int rows = src_vector.size();
	for(int r=0; r<rows; r++){
		csv_ofs << src_vector[r] << std::endl;
	}
}


//! std::vector<std::vector>をCSVファイルとして保存
template<typename T> void SaveVectorCSV(const std::string& filename, const std::vector<std::vector<T>>& src_vector)
{
	std::ofstream csv_ofs(filename);
	int rows = src_vector.size();
	for(int r=0; r<rows; r++){
		int cols = src_vector[r].size();
		if(cols > 0)	
			csv_ofs << src_vector[r][0];
		for(int c=1; c<cols; c++){
			csv_ofs << "," << src_vector[r][c];
		}
		csv_ofs << std::endl;
	}
}


#endif
