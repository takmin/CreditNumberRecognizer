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

#ifndef __CREDIT_NUNBER_RECOG__
#define __CREDIT_NUNBER_RECOG__

#include "EdgeDirFeatures.h"
#include "NumberDetect.h"
#include "NumberRecog.h"

namespace ccnr{

class CreditNumberRecog
{
public:
	CreditNumberRecog(void);
	~CreditNumberRecog(void);

	void RecognizeCreditCardNumber(const cv::Mat& card_img, std::vector<int>& numbers, std::vector<cv::Rect>& num_pos) const;

	int LoadClassifier(const std::string& train_file){
		return _NumberRecognizer.Load(train_file);
	};

	int LoadDetector(const std::string& detector_file){
		//return _NumberRecognizer.LoadDetector(detector_file, _FeatureExtractor.calcSizeImg2Feature(_train_size));
		return _NumberRecognizer.LoadOVR(detector_file, _FeatureExtractor.calcSizeImg2Feature(_train_size));
	}

	void CreateFeature(const cv::Mat& img, cv::Mat& feature, bool resize = true) const;

	void CreateFeature(const std::string& imgfile, cv::Mat& feature, bool resize = true) const;

	void CreateFeatures(const std::vector<std::string>& imglist, cv::Mat& features, bool resize = true) const;

	int GetProcImageSize() const{
		return _input_width;
	}

	cv::Size GetTrainCharSize() const{
		return _train_size;
	}

	//! 文字（数字）の存在確率にもとづいた各場所のコスト算出
	void CreateCharExistingCost(const cv::Mat& img, int size, std::vector<double>& char_exist_cost, std::vector<double>& char_non_exist_cost) const;

private:
	EdgeDirFeatures _FeatureExtractor;
	NumberDetect _NumberDetector;
	NumberRecog _NumberRecognizer;

	cv::Size _train_size;
	int _input_width;
};

}

#endif