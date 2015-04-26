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

#include "EdgeDirFeatures.h"
#include <opencv2/imgproc/imgproc.hpp>

namespace ccnr{

//! 小数点以下四捨五入
inline int round(double a){return (int)(a+0.5);};


EdgeDirFeatures::EdgeDirFeatures(void)
{
	_NumDirections = 4;
	_MaxPoolSize = 4;
	_OverlapRatio = 0.5;
}


EdgeDirFeatures::~EdgeDirFeatures(void)
{
}


// 特徴抽出
void EdgeDirFeatures::operator()(const cv::Mat& src_img, std::vector<cv::Mat>& features, int dir_num, int pool_size, float overlap) const
{
	// エッジを方向成分に分解
	std::vector<cv::Mat> edge_dir;
	ExtractEdgeDir(src_img, edge_dir, dir_num);
	
	// フィルターの端を切り取り
	std::vector<cv::Mat> edge_dir2;
	cv::Rect trunc_rect(1,1, src_img.cols-2, src_img.rows-2);
	std::vector<cv::Mat>::iterator it, it_end = edge_dir.end();
	for(it = edge_dir.begin(); it != it_end; it++){
		cv::Mat feature = (*it)(trunc_rect).clone();
		edge_dir2.push_back(feature);
	}

	MaxPooling(edge_dir2, features, pool_size, overlap);
};


//! 画像から方向成分を抽出
void EdgeDirFeatures::ExtractEdgeDir(const cv::Mat& src_img, std::vector<cv::Mat>& dir_imgs, int dir_num)
{
	dir_imgs.resize(dir_num);
	for(int i=0; i<dir_num; i++){
		dir_imgs[i] = cv::Mat::zeros(src_img.size(), CV_64FC1);
	}

	cv::Mat gray;
	if(src_img.channels() > 1)
		cv::cvtColor(src_img, gray, cv::COLOR_RGB2GRAY);
	else
		gray = src_img;

	cv::Mat X_sobelMat, Y_sobelMat;

	cv::Sobel(gray, X_sobelMat, CV_64F, 1, 0);
	cv::Sobel(gray, Y_sobelMat, CV_64F, 0, 1);

	double* x_sobel_data = (double*)X_sobelMat.data;
	double* y_sobel_data = (double*)Y_sobelMat.data;

	double angle = CV_PI /(double)dir_num;	//量子化する時の角度
	double gradient;

	double magnitude;
	double xDelta, yDelta;
//	int step_size = img.step / sizeof(float);
	int step_size = src_img.cols;
	for(int y=0; y<src_img.rows; y++){
		for(int x=0; x<src_img.cols; x++){
			xDelta = *(x_sobel_data + y*step_size + x);
			yDelta = *(y_sobel_data + y*step_size + x);
			magnitude = sqrt(xDelta * xDelta + yDelta * yDelta);
		
			//勾配方向の算出
			gradient = atan2((double)yDelta, (double)xDelta);
			//ラジアンから角度へ変換
//			gradient = (gradient*180.0)/CV_PI;
			//符号が負である場合は反転
			if(gradient < 0.0){
				gradient += 2.0 * CV_PI;
			}
			//0～360度から0～180度に変換
			if(gradient >= CV_PI){
				gradient -= CV_PI;
			}
			//角度分割
			int dir = round(gradient/angle) % dir_num;

			dir_imgs[dir].at<double>(y,x) = magnitude;
		}
	}
}


void EdgeDirFeatures::MaxPooling(const cv::Mat& img, cv::Mat& output, int pool_size, float overlap)
{
	cv::Size out_size = calcSizeEdge2Max(img.size(), pool_size, overlap);
	int step = pool_size * (1.0 - overlap);
		
	output = cv::Mat::zeros(out_size.height, out_size.width, CV_32FC1);
	cv::Rect rect(0,0,pool_size,pool_size);
	int x, y;
	int max_rect_x = img.cols - step;
	int max_rect_y = img.rows - step;
	for(y=0, rect.y=0;y<output.rows && rect.y <= max_rect_y;y++, rect.y += step){
		for(x=0, rect.x=0;x<output.cols && rect.x <= max_rect_x;x++, rect.x += step){
			double min_val, max_val;
			cv::minMaxLoc(img(rect), &min_val, &max_val);
			output.at<float>(y,x) = (float)max_val;
		}
	}
}



void EdgeDirFeatures::MaxPooling(const std::vector<cv::Mat>& dir_imgs, std::vector<cv::Mat>& output, int pool_size, float overlap)
{
	// Set size of Layer
	output.clear();

	std::vector<cv::Mat>::const_iterator cit, cit_end = dir_imgs.end();
	for(cit = dir_imgs.begin(); cit != cit_end; cit++){
		cv::Mat out;
		MaxPooling(*cit, out, pool_size, overlap);
		output.push_back(out);
	}
}


//! Matをつなげて、行数がMatの数、列数がMatの要素数となる１つのMatを生成
void EdgeDirFeatures::ConcatMatFeature2D(const std::vector<cv::Mat>& train_features, cv::Mat& concat_feature)
{
	if(train_features.empty() || train_features[0].empty())
		return;

	int rows = train_features.size();
	int cols = train_features[0].total();
	int type = train_features[0].type();
	concat_feature.create(rows, cols, type);
	unsigned char* data_ptr = concat_feature.data;
	int data_size = concat_feature.elemSize() * cols;
	for(int i=0; i<rows; i++){
		assert(train_features[i].total() == cols);
		assert(train_features[i].type() == type);
		memcpy(data_ptr, train_features[i].data, data_size);
		data_ptr += data_size;
	}
}


//! Matをつなげて行数が1のベクトルへ変換する
void EdgeDirFeatures::ConcatMatFeature1D(const std::vector<cv::Mat>& mat_vec, cv::Mat& concat_feature)
{
	if(mat_vec.empty())
		return;

	int sum_size = 0;
	std::vector<cv::Mat>::const_iterator it, it_end = mat_vec.end();
	for(it = mat_vec.begin(); it != it_end; it++){
		sum_size += it->total();
	}

	concat_feature.create(1,sum_size,mat_vec[0].type());
	int elem_size = mat_vec[0].elemSize();
	unsigned char* ptr = concat_feature.data;
	for(it = mat_vec.begin(); it != it_end; it++){
		int size = it->total() * elem_size;
		memcpy(ptr, it->data, size);
		ptr += size;
	}
}


//! エッジ特徴上の座標をMax-pooling後の座標へ変換
float EdgeDirFeatures::calcPosEdge2Max(int x, int MaxFilterSize, float OverlapRatio){
	int step = MaxFilterSize * (1.0 - OverlapRatio);
	int res_step = MaxFilterSize - step;
	float pos = x / step;
	int res = x % step;
	if(res_step > res){
		pos -= (1.0 - ((float)res + 0.5) / res_step);
	}
	return pos;
}


//! 特徴量を画像サイズへ変換
void EdgeDirFeatures::ConvertFeature2ImageSize(const cv::Mat& src, cv::Mat& dst) const
{
	cv::Size dst_size = calcSizeFeature2Img(src.size());
	dst = cv::Mat::zeros(dst_size, src.type());
	if(src.type() == CV_32FC1){
		ConvertFeature2ImageSize((cv::Mat_<float>&)src, (cv::Mat_<float>&)dst);
	}
	else if(src.type() == CV_64FC1){
		ConvertFeature2ImageSize((cv::Mat_<double>&)src, (cv::Mat_<double>&)dst);
	}
	else if(src.type() == CV_8UC1){
		ConvertFeature2ImageSize((cv::Mat_<unsigned char>&)src, (cv::Mat_<unsigned char>&)dst);
	}
	else if(src.type() == CV_32SC1){
		ConvertFeature2ImageSize((cv::Mat_<int>&)src, (cv::Mat_<int>&)dst);
	}
};


//! 特徴量を画像サイズへ変換
template <typename T>
void EdgeDirFeatures::ConvertFeature2ImageSize(const cv::Mat_<T>& src, cv::Mat_<T>& dst) const
{
	for(int y=0; y<dst.rows; y++){
		float src_y = calcPosImg2Feature(y);
		if(src_y < 0 || src_y > src.rows - 1)
			continue;

		int src_iy = (int)src_y;
		float y_res = src_y - src_iy;
		float y_res_inv = 1.0 - y_res;
		for(int x=0; x<dst.cols; x++){
			float src_x = calcPosImg2Feature(x);
			if(src_x < 0 || src_x > src.cols - 1)
				continue;

			int src_ix = (int)src_x;
			float x_res = src_x - src_ix;
			float x_res_inv = 1.0 - x_res;
			T val = src(src_iy, src_ix) * x_res_inv * y_res_inv;
			if(x_res > 0)
				val += src(src_iy, src_ix + 1) * x_res * y_res_inv;
			if(y_res > 0)
				val += src(src_iy + 1, src_ix) * x_res_inv * y_res;
			if(x_res > 0 && y_res > 0)
				val += src(src_iy + 1, src_ix + 1) * x_res * y_res;
			dst(y,x) = val;
		}
	}
}


}
