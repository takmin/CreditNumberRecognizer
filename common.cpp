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

#include "common.h"

namespace ccnr{

template <typename T>
void ProjectionH(const cv::Mat_<T>& src_mat, cv::Mat& dst_hist)
{
	dst_hist.create(src_mat.rows, 1, CV_64FC1);
	for(int r = 0; r<src_mat.rows; r++){
		double val = 0;
		for(int c = 0; c<src_mat.cols; c++){
			val += src_mat.template at<T>(r,c);
		}
		dst_hist.at<double>(r,0) = val / src_mat.cols;
	}
}

template <typename T>
void ProjectionV(const cv::Mat_<T>& src_mat, cv::Mat& dst_hist)
{
	dst_hist.create(1, src_mat.cols, CV_64FC1);
	for(int c = 0; c<src_mat.cols; c++){
		double val = 0;
		for(int r = 0; r<src_mat.rows; r++){
			val += src_mat.template at<T>(r,c);
		}
		dst_hist.at<double>(0,c) = val / src_mat.rows;
	}
}


void Projection(const cv::Mat& src_mat, cv::Mat& dst_hist, bool sum_rows)
{
	int type = src_mat.type();
	if(type == CV_8UC1){
		if(sum_rows)
			ProjectionH<unsigned char>(src_mat, dst_hist);
		else
			ProjectionV<unsigned char>(src_mat, dst_hist);
	}
	else if(type == CV_32SC1){
		if(sum_rows)
			ProjectionH<int>(src_mat, dst_hist);
		else
			ProjectionV<int>(src_mat, dst_hist);
	}
	else if(type == CV_32FC1){
		if(sum_rows)
			ProjectionH<float>(src_mat, dst_hist);
		else
			ProjectionV<float>(src_mat, dst_hist);
	}
	else if(type == CV_64FC1){
		if(sum_rows)
			ProjectionH<double>(src_mat, dst_hist);
		else
			ProjectionV<double>(src_mat, dst_hist);
	}
}

//! はみ出る領域をカット
cv::Rect TruncateRect(const cv::Rect& obj_rect, const cv::Size& img_size)
{
	cv::Rect resize_rect = obj_rect;
	if(obj_rect.x < 0){
		resize_rect.x = 0;
		resize_rect.width += obj_rect.x;
	}
	if(obj_rect.y < 0){
		resize_rect.y = 0;
		resize_rect.height += obj_rect.y;
	}
	if(resize_rect.x + resize_rect.width > img_size.width){
		resize_rect.width = img_size.width - resize_rect.x;
	}
	if(resize_rect.y + resize_rect.height > img_size.height){
		resize_rect.height = img_size.height - resize_rect.y;
	}

	return resize_rect;
}

}
