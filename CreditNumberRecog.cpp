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

#include "CreditNumberRecog.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "common.h"

namespace ccnr{

CreditNumberRecog::CreditNumberRecog(void)
{
	this->_input_width = 320;
	this->_train_size = cv::Size(16,24);
	this->_FeatureExtractor.init(4, 4, 0.5);
}


CreditNumberRecog::~CreditNumberRecog(void)
{
}

	
void CreditNumberRecog::CreateFeature(const cv::Mat& img, cv::Mat& feature, bool resize_f) const
{
	cv::Mat resize_img;
	if(resize_f){
		cv::resize(img, resize_img, _train_size);
	}
	else{
		resize_img = img;
	}
	_FeatureExtractor(resize_img, feature);
};


void CreditNumberRecog::CreateFeature(const std::string& imgfile, cv::Mat& feature, bool resize_f) const
{
	cv::Mat img = cv::imread(imgfile);
	if(img.empty())
		return;
	CreateFeature(img, feature, resize_f);
}


void CreditNumberRecog::CreateFeatures(const std::vector<std::string>& imglist, cv::Mat& features, bool resize_f) const
{
	std::vector<cv::Mat> feature_vec;
	std::vector<std::string>::const_iterator cit, cit_end = imglist.end();
	for(cit = imglist.begin(); cit != cit_end; cit++){
		cv::Mat img = cv::imread(*cit);
		if(!img.empty()){
			cv::Mat feature;
			CreateFeature(img, feature, resize_f);
			feature_vec.push_back(feature);
		}
	}
	EdgeDirFeatures::ConcatMatFeature2D(feature_vec, features);
}



void CreditNumberRecog::RecognizeCreditCardNumber(const cv::Mat& card_img, std::vector<int>& numbers, std::vector<cv::Rect>& num_pos) const
{
	// グレースケール変換
	cv::Mat img;
	if(card_img.channels() > 1){
		cv::cvtColor(card_img, img, cv::COLOR_RGB2GRAY);
	}
	else{
		img = card_img;
	}

	// 画像サイズ変換
	cv::Mat proc_img;
	cv::resize(img, proc_img, cv::Size(_input_width, round((float)img.rows * _input_width / img.cols)));
	
	// エッジ画像作成
	cv::Mat RowGrad, ColGrad, SumGrad;
	cv::Sobel(proc_img, RowGrad, CV_32F, 1, 0);
	cv::Sobel(proc_img, ColGrad, CV_32F, 0, 1);
	cv::add(cv::abs(RowGrad), cv::abs(ColGrad), SumGrad);

	// 文字領域切り出し
	std::vector<cv::Rect> char_regions;
	NumberDetect::CREDIT_PATTERN pattern;
	_NumberDetector.ExtractNumbers(SumGrad, char_regions, pattern);

	// 検出結果格納
	std::vector<cv::Rect>::iterator rect_it, rect_it_end = char_regions.end();
	float ratio = (float)img.cols / _input_width;
	for(rect_it = char_regions.begin(); rect_it != rect_it_end; rect_it++){
		if(rect_it->width > 0 && rect_it->height > 0){
			cv::Rect rect((int)(ratio * rect_it->x), (int)(ratio * rect_it->y), 
				round(ratio * rect_it->width), round(ratio * rect_it->height));
			num_pos.push_back(TruncateRect(rect, img.size()));
		}
	}

	// 文字認識
	rect_it_end = num_pos.end();
	for(rect_it = num_pos.begin(); rect_it != rect_it_end; rect_it++){
		cv::Mat feature;
		CreateFeature(img(*rect_it).clone(), feature);
		numbers.push_back(_NumberRecognizer.predict(feature));
	}

}


//! 文字（数字）の存在確率にもとづいた各場所のコスト算出
void CreditNumberRecog::CreateCharExistingCost(const cv::Mat& img, int size, std::vector<double>& char_exist_cost, std::vector<double>& char_non_exist_cost) const
{
	// 文字列領域候補の高さを訓練画像のものに合わせる
	cv::Mat resize_img;
	cv::Size detect_size(round((float)img.cols * _train_size.height / img.rows), _train_size.height);
	cv::resize(img, resize_img, detect_size);

	// リサイズした画像から特徴量抽出
	std::vector<cv::Mat> features;
	_FeatureExtractor(resize_img, features);
	
	// 特徴量を元に文字の存在コストと非存在コストを算出
	cv::Mat pos_map, neg_map;
	_NumberRecognizer.CharExistingCostOVR(features, pos_map, neg_map);
	//_NumberRecognizer.CharExistingCost(features, pos_map, neg_map);

	// 特徴量のサイズを画像サイズへ変更
	cv::Mat pos_map2, neg_map2;
	_FeatureExtractor.ConvertFeature2ImageSize(pos_map, pos_map2);
	_FeatureExtractor.ConvertFeature2ImageSize(neg_map, neg_map2);

	// 特徴量マップをdouble型へ変更
	cv::Mat pos_map3, neg_map3;
	pos_map2.convertTo(pos_map3, CV_64FC1);
	neg_map2.convertTo(neg_map3, CV_64FC1);

	// 存在コストを１次元へ変換
	int r = pos_map2.rows / 2;
	cv::Mat exist_cost(1, resize_img.cols, CV_64FC1), non_exist_cost(1, resize_img.cols, CV_64FC1);
	pos_map3(cv::Rect(0,r,pos_map3.cols,1)).copyTo(exist_cost);
	neg_map3(cv::Rect(0,r,neg_map3.cols,1)).copyTo(non_exist_cost);

	// 元のサイズとの不整合部分を補間
	if(pos_map3.cols < detect_size.width){
		double pos_max, pos_min, neg_max, neg_min;
		cv::minMaxLoc(pos_map3, &pos_min, &pos_max);
		cv::minMaxLoc(neg_map3, &neg_min, &neg_max);
		for(int i=pos_map3.cols; i<detect_size.width; i++){
			exist_cost.at<double>(0,i) = pos_max;
			non_exist_cost.at<double>(0,i) = neg_max;
		}
	}

	// コスト分布の長さへリサイズ
	cv::Mat exist_cost2, non_exist_cost2;
	cv::resize(exist_cost, exist_cost2, cv::Size(size,1));
	cv::resize(non_exist_cost, non_exist_cost2, cv::Size(size,1));

	// 分布を格納
	char_exist_cost.clear(), char_non_exist_cost.clear();
	for(int i=0; i<size; i++){
		char_exist_cost.push_back(exist_cost2.at<double>(0,i));
		char_non_exist_cost.push_back(non_exist_cost2.at<double>(0,i));
	}
}


}