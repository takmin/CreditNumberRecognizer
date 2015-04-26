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

#include "util.h"
#include <sstream>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <opencv2/imgproc/imgproc.hpp>

// intをStringへ変換
std::string Int2String(int x)
{
	std::stringstream strstr;
	strstr << x;
	return strstr.str();
}

// ディレクトリから画像ファイル名一覧を取得
bool ReadImageFilesInDirectory(const std::string& img_dir, std::vector<std::string>& image_lists)
{
	using namespace boost::filesystem;

	path img_dir_path(img_dir);
	if(!is_directory(img_dir_path)){
		return false;
	}

	directory_iterator end;
	for(directory_iterator p(img_dir_path); p != end; ++p){
		path file_p = p->path();
		std::string ext = file_p.extension().string();
		if(ext == ".jpg" || ext == ".JPG" || ext == ".bmp" || ext == ".BMP" || ext == ".png" || ext == ".PNG"){
			image_lists.push_back(file_p.string());
		}
	}
	return true;
}


void DrawHistogram(const cv::Mat& histogram, cv::Mat& draw_img, int width)
{
	double minval, maxval;
	cv::minMaxLoc(histogram, &minval, &maxval);

	cv::Mat norm_hist;
	histogram.convertTo(norm_hist, CV_32FC1, (double)width / maxval);

	if(histogram.cols == 1){
		draw_img = cv::Mat::zeros(histogram.rows, width, CV_8UC1);
		for(int r=0; r<histogram.rows; r++){
			for(int c=0; c<norm_hist.at<float>(r,0); c++){
				draw_img.at<unsigned char>(r, c) = 255;
			}
		}
	}
	else if(histogram.rows == 1){
		draw_img = cv::Mat::zeros(width, histogram.cols, CV_8UC1);
		for(int c=0; c<histogram.cols; c++){
			for(int r=0; r<norm_hist.at<float>(0,c); r++){
				draw_img.at<unsigned char>(width - r -1, c) = 255;
			}
		}
	}
}


cv::Mat Convert8UC3(const cv::Mat& src)
{
	cv::Mat tmp, dst;
	if(src.depth() != CV_8U){
		cv::normalize(src, tmp, 255, 0, cv::NORM_MINMAX, CV_8U);
	}
	else{
		tmp = src;
	}
	if(tmp.channels() == 1){
		cv::cvtColor(tmp, dst, cv::COLOR_GRAY2RGB);
	}
	else{
		dst = tmp;
	}
	return dst;
}


cv::Mat ConcatinateImage(const cv::Mat& src1, const cv::Mat& src2, bool hol)
{
	cv::Mat img1 = Convert8UC3(src1);
	cv::Mat img2 = Convert8UC3(src2);

	cv::Mat concatImg;
	cv::Size img_size;
	cv::Point pt;
	if(hol){
		img_size.width = img1.cols + img2.cols;
		img_size.height = std::max(img1.rows, img2.rows);
		pt.x = img1.cols;
		pt.y = 0;
	}
	else{
		img_size.width = std::max(img1.cols, img2.cols);
		img_size.height = img1.rows + img2.rows;
		pt.x = 0;
		pt.y = img1.rows;
	}

	concatImg = cv::Mat::zeros(img_size, img1.type());
	img1.copyTo(concatImg(cv::Rect(0,0,img1.cols,img1.rows)));
	img2.copyTo(concatImg(cv::Rect(pt.x,pt.y,img2.cols,img2.rows)));
	return concatImg;
}


template<typename T> void SaveMatCSV_T(const std::string& filename, const cv::Mat& mat)
{
	T* data_ptr = (T*)mat.data;
	std::ofstream ofs(filename);
	for(int r=0; r<mat.rows; r++){
		for(int c=0; c<mat.cols-1; c++){
			ofs << *data_ptr << ",";
			data_ptr++;
		}
		ofs << *data_ptr << std::endl;
		data_ptr++;
	}
}


void SaveMatCSV(const std::string& filename, const cv::Mat& mat)
{
	if(mat.type() == CV_8U){
		SaveMatCSV_T<unsigned char>(filename,mat);
	}
	else if(mat.type() == CV_32S){
		SaveMatCSV_T<int>(filename,mat);
	}
	else if(mat.type() == CV_32F){
		SaveMatCSV_T<float>(filename,mat);
	}
	else if(mat.type() == CV_64F){
		SaveMatCSV_T<double>(filename,mat);
	}
}

