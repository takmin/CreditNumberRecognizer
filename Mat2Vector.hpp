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

#ifndef __MATVECTOR__
#define __MATVECTOR__

#include <opencv2/core/core.hpp>

namespace ccnr{

template <typename S, typename T>
void Mat2Vector_(const cv::Mat_<S>& src, std::vector<T>& dst)
{
	cv::MatConstIterator_<S> mat_it, mat_end = src.end();
	for(mat_it = src.begin(); mat_it != mat_end; mat_it++){
		dst.push_back((T)(*mat_it));
	}
}


template <typename T>
void Mat2Vector(const cv::Mat& src, std::vector<T>& dst)
{
	if(src.type() == CV_8UC1){
		Mat2Vector_<unsigned char>(src, dst);
	}
	else if(src.type() == CV_32SC1){
		Mat2Vector_<int>(src, dst);
	}
	else if(src.type() == CV_32FC1){
		Mat2Vector_<float>(src, dst);
	}
	else if(src.type() == CV_64FC1){
		Mat2Vector_<double>(src, dst);
	}
}

}

#endif