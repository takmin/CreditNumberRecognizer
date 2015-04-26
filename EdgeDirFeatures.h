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

#ifndef __EDGE_FEATURES__
#define __EDGE_FEATURES__

#include <opencv2/core/core.hpp>

namespace ccnr{

class EdgeDirFeatures
{
public:
	EdgeDirFeatures(void);
	~EdgeDirFeatures(void);

	// 特徴抽出
	void operator()(const cv::Mat& src_img, std::vector<cv::Mat>& features, int dir_num, int pool_size, float overlap) const;

	void operator()(const cv::Mat& src_img, std::vector<cv::Mat>& features) const{
		operator()(src_img, features, _NumDirections, _MaxPoolSize, _OverlapRatio);
	}

	void operator()(const cv::Mat& src_img, cv::Mat& features, int dir_num, int pool_size, float overlap) const{
		std::vector<cv::Mat> max_pools;
		operator()(src_img, max_pools, dir_num, pool_size, overlap);
		ConcatMatFeature1D(max_pools, features);
	};


	void operator()(const cv::Mat& src_img, cv::Mat& features) const{
		operator()(src_img, features, _NumDirections, _MaxPoolSize, _OverlapRatio);
	}

	void init(int dir_num, int pool_size, float overlap){
		_NumDirections = dir_num;
		_MaxPoolSize = pool_size;
		_OverlapRatio = overlap;
	};


	//! 画像から方向成分を抽出
	static void ExtractEdgeDir(const cv::Mat& src_img, std::vector<cv::Mat>& dir_imgs, int dir_num);

	void ExtractEdgeDir(const cv::Mat& src_img, std::vector<cv::Mat>& dir_imgs) const{
		ExtractEdgeDir(src_img, dir_imgs, _NumDirections);
	};


	//! Max Pooling
	static void MaxPooling(const cv::Mat& img, cv::Mat& output, int pool_size, float overlap);

	void MaxPooling(const cv::Mat& img, cv::Mat& output) const{
		MaxPooling(img, output, _MaxPoolSize, _OverlapRatio);
	};

	static void MaxPooling(const std::vector<cv::Mat>& dir_imgs, std::vector<cv::Mat>& output, int pool_size, float overlap);

	void MaxPooling(const std::vector<cv::Mat>& dir_imgs, std::vector<cv::Mat>& output) const{
		MaxPooling(dir_imgs, output, _MaxPoolSize, _OverlapRatio);
	};

	//! 画像のサイズをMax-pooling後のサイズへ変換
	static int calcSizeEdge2Max(int org_size, int MaxFilterSize, float OverlapRatio)
	{
		int step = MaxFilterSize * (1.0 - OverlapRatio);
		return (org_size - MaxFilterSize) / step + 1;
	}

	int calcSizeEdge2Max(int org_size) const{
		return calcSizeEdge2Max(org_size, _MaxPoolSize, _OverlapRatio);
	}

	int calcSizeImg2Feature(int img_size) const{
		return calcSizeEdge2Max(img_size - 2);
	}

	//! 画像のサイズをMax-poolingのサイズへ変換
	static cv::Size calcSizeEdge2Max(const cv::Size& org_size, int MaxFilterSize, float OverlapRatio)
	{
		int width = calcSizeEdge2Max(org_size.width, MaxFilterSize, OverlapRatio);
		int height = calcSizeEdge2Max(org_size.height, MaxFilterSize, OverlapRatio);
		return cv::Size(width, height);
	}

	cv::Size calcSizeEdge2Max(const cv::Size& org_size) const{
		return calcSizeEdge2Max(org_size, _MaxPoolSize, _OverlapRatio);
	};

	cv::Size calcSizeImg2Feature(const cv::Size& img_size) const{
		return cv::Size(calcSizeImg2Feature(img_size.width), calcSizeImg2Feature(img_size.height));
	}

	//! 特徴サイズから画像サイズを逆算
	static int calcSizeMax2Edge(int feature_size, int MaxFilterSize, float OverlapRatio){
		int step = MaxFilterSize * (1.0 - OverlapRatio);
		return feature_size * step + (MaxFilterSize - step);
	}

	int calcSizeMax2Edge(int feature_size) const{
		return calcSizeMax2Edge(feature_size, _MaxPoolSize, _OverlapRatio);
	}

	int calcSizeFeature2Img(int feature_size) const{
		int size = calcSizeMax2Edge(feature_size);
		return size + 2;
	}

	cv::Size calcSizeFeature2Img(const cv::Size& feature_size) const{
		return cv::Size(calcSizeFeature2Img(feature_size.width), calcSizeFeature2Img(feature_size.height));
	}

	//! エッジ特徴上の座標をMax-pooling後の座標へ変換
	static float calcPosEdge2Max(int x, int MaxFilterSize, float OverlapRatio);

	float calcPosEdge2Max(int x) const{
		return calcPosEdge2Max(x, _MaxPoolSize, _OverlapRatio);
	}

	//! 入力画像上の座標を特徴量上の座標へ変換
	float calcPosImg2Feature(int x) const{
		return calcPosEdge2Max(x-1);
	}

	//! 特徴量を画像サイズへ変換
	void ConvertFeature2ImageSize(const cv::Mat& src, cv::Mat& dst) const;

	//! 方向数を取得
	int GetNumDirections() const{
		return _NumDirections;
	};

	///// 特徴ベクトル成型 /////
	//! Matをつなげて行数が1のベクトルへ変換する
	static void ConcatMatFeature1D(const std::vector<cv::Mat>& mat_vec, cv::Mat& concat_feature);

	//! Matをつなげて、行数がMatの数、列数がMatの要素数となる１つのMatを生成
	static void ConcatMatFeature2D(const std::vector<cv::Mat>& train_features, cv::Mat& concat_feature);


private:
	int _NumDirections;
	int _MaxPoolSize;
	float _OverlapRatio;

	//! 特徴量を画像サイズへ変換
	template <typename T>
	void ConvertFeature2ImageSize(const cv::Mat_<T>& src, cv::Mat_<T>& dst) const;
};

}
#endif