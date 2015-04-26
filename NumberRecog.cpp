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

#include "NumberRecog.h"
#include "common.h"
#include <opencv2/imgproc/imgproc.hpp>

namespace ccnr{

NumberRecog::NumberRecog(void)
{
}


NumberRecog::~NumberRecog(void)
{
}


cv::Mat NumberRecog::HomogeneousVector(const cv::Mat& feature)
{
	cv::Mat conv_mat;
	if(feature.type() == CV_32FC1){
		conv_mat = feature;
	}
	else{
		feature.convertTo(conv_mat, CV_32FC1);
	}

	cv::Mat dest_mat(feature.rows, feature.cols+1, CV_32FC1);
	conv_mat.copyTo(dest_mat(cv::Rect(0,0,feature.cols,feature.rows)));

	int c = feature.cols;
	for(int r=0; r<feature.rows; r++){
		dest_mat.at<float>(r,c) = 1.0;
	}
	return dest_mat;
}


int NumberRecog::SvmCoeff2Filters(const cv::Mat& svm_param, const cv::Size& filter_size, std::vector<cv::Mat>& filters, double& bias, int type)
{
	assert(svm_param.rows == 1);
	assert(svm_param.cols % (filter_size.width * filter_size.height) == 1);

	cv::Mat svm_coeff;
	if(type < 0 || type == svm_param.type()){
		svm_coeff = svm_param;
	}
	else if(type == CV_64FC1 || type == CV_32FC1){
		svm_param.convertTo(svm_coeff, type);
	}
	else{
		return -1;
	}

	int dir_num = svm_param.cols / (filter_size.width * filter_size.height);

	unsigned char* ptr = svm_coeff.data;
	int data_size = filter_size.width * filter_size.height * svm_coeff.elemSize();
	for(int i=0; i<dir_num; i++){
		cv::Mat filter(filter_size, svm_coeff.type());
		memcpy(filter.data, ptr, data_size);
		ptr += data_size;

		filters.push_back(filter);
	}
	if(svm_coeff.type() == CV_32FC1){
		bias = svm_coeff.at<float>(svm_coeff.rows-1, svm_coeff.cols-1);
	}
	else if(svm_coeff.type() == CV_64FC1){
		bias = svm_coeff.at<double>(svm_coeff.rows-1, svm_coeff.cols-1);
	}

	return 0;
}



int NumberRecog::Load(const std::string& svm_file)
{
	cv::FileStorage fs(svm_file, cv::FileStorage::READ);
	if(!fs.isOpened())
		return -1;

	cv::Mat svm_coeffs;
	fs["svm_coeff"] >> svm_coeffs;
	if(svm_coeffs.empty())
		return -1;

	if(svm_coeffs.type() == CV_32FC1)
		_SvmCoeffs = svm_coeffs;
	else
		svm_coeffs.convertTo(_SvmCoeffs, CV_32FC1);

	_NumClass = round(std::sqrt(2 * _SvmCoeffs.rows - 0.25) + 0.5);

	return _NumClass;
}


cv::Mat NumberRecog::score(const cv::Mat& feature) const
{
	if(feature.cols+1 != _SvmCoeffs.cols)
		return cv::Mat();

	cv::Mat hom_feat = HomogeneousVector(feature);
	return _SvmCoeffs * hom_feat.t();
}


int NumberRecog::predict(const cv::Mat& feature) const
{
	cv::Mat scores = score(feature);
	if(scores.empty())
		return -1;
	std::vector<int> count(_NumClass, 0);
	int a=0, b=1;
	for(int r=0; r<scores.rows; r++){
		if(scores.at<float>(r,0) > 0){
			count[a]++;
		}
		else{
			count[b]++;
		}
		b++;
		if(b >= _NumClass){
			a++;
			b = a+1;
		}
	}
	int max_val;
	return max_arg(count, max_val);
}


int NumberRecog::LoadDetector(const std::string& train_file, const cv::Size& filter_size)
{
	cv::FileStorage fs(train_file, cv::FileStorage::READ);
	if(!fs.isOpened())
		return -1;

	cv::Mat svm_coeffs;
	fs["svm_coeff"] >> svm_coeffs;
	if(svm_coeffs.empty())
		return -1;

	int ret = SvmCoeff2Filters(svm_coeffs, filter_size, _Filters, _Bias, CV_32FC1);
	if(ret < 0)
		return ret;

	return 0;
}


void NumberRecog::ScoreMap(const std::vector<cv::Mat>& feature_map, cv::Mat& response_map, const std::vector<cv::Mat>& filter, double bias)
{
	assert(feature_map.size() == filter.size());
	
	int num_filter = filter.size();
	for(int i=0; i<num_filter; i++){
		cv::Mat dst;
		cv::filter2D(feature_map[i], dst, feature_map[i].type(), filter[i]);
		if(i==0)
			response_map = dst;
		else
			response_map += dst;
	}
	response_map += bias;
}



void NumberRecog::Score2Cost(const cv::Mat& response_map, cv::Mat& pos_cost_map, cv::Mat& neg_cost_map)
{
	assert(response_map.type() == CV_32FC1 || response_map.type() == CV_64FC1);

	cv::Mat pos_exp_map, neg_exp_map;
	cv::exp(-response_map, pos_exp_map);
	pos_exp_map += 1.0;
	cv::log(pos_exp_map, pos_cost_map);

	cv::exp(response_map, neg_exp_map);
	neg_exp_map += 1.0;
	cv::log(neg_exp_map, neg_cost_map);
}

////////// One-vs-Rest Filter /////////
int NumberRecog::LoadOVR(const std::string& train_file, const cv::Size& filter_size)
{
	cv::FileStorage fs(train_file, cv::FileStorage::READ);
	if(!fs.isOpened())
		return -1;

	cv::Mat svm_coeffs;
	fs["svm_coeff"] >> svm_coeffs;
	if(svm_coeffs.empty())
		return -1;

	int ret = SvmCoeff2Filters(svm_coeffs, filter_size, _FilterOVR, _BiasOVR, CV_32FC1);
	if(ret < 0)
		return ret;

	return 0;
}


int NumberRecog::SvmCoeff2Filters(const cv::Mat& svm_param, const cv::Size& filter_size, 
	std::vector<std::vector<cv::Mat>>& filterOVR, std::vector<double>& biasOVR, int type)
{
	filterOVR.clear();
	biasOVR.clear();
	for(int i=0; i<svm_param.rows; i++){
		std::vector<cv::Mat> filter;
		double bias;
		int ret = SvmCoeff2Filters(svm_param(cv::Rect(0,i,svm_param.cols,1)), filter_size, filter, bias, type);
		if(ret < 0)
			return ret;
		filterOVR.push_back(filter);
		biasOVR.push_back(bias);
	}
	return 0;
}


void NumberRecog::ScoreMapOVR(const std::vector<cv::Mat>& feature_map, std::vector<cv::Mat>& response_map) const
{
	int size = _FilterOVR.size();
	response_map.clear();
	for(int i=0; i<size; i++){
		cv::Mat response;
		ScoreMap(feature_map, response, _FilterOVR[i], _BiasOVR[i]);
		response_map.push_back(response);
	}
}


void NumberRecog::Score2CostOVR(const std::vector<cv::Mat>& response_map, cv::Mat& pos_cost_map, cv::Mat& neg_cost_map)
{
	// ç≈å„ÇÃâûìöÇ™îwåi
	int class_num = response_map.size();
	std::vector<cv::Mat> exp_mats;
	for(int i=0; i<class_num; i++){
		cv::Mat expMat;
		cv::exp(response_map[i], expMat);
		exp_mats.push_back(expMat);
	}

	cv::Mat sum_mat = exp_mats[0].clone();
	for(int i=1; i<class_num; i++){
		sum_mat += exp_mats[i];
	}
	
	cv::Mat neg_prob = exp_mats[class_num-1] / sum_mat;
	cv::Mat pos_prob = -neg_prob + 1.0;

	cv::Mat neglog, poslog;
	cv::log(neg_prob,neglog);
	cv::log(pos_prob,poslog);
	pos_cost_map = poslog * -1.0;
	neg_cost_map = neglog * -1.0;
}


}