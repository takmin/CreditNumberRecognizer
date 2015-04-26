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

#ifndef __NUMBER_DETECT__
#define __NUMBER_DETECT__

#include <opencv2/core/core.hpp>

namespace ccnr{

class NumberDetect
{
public:
	NumberDetect(void);
	~NumberDetect(void);

	typedef struct{
		static const int
			CHAR_BLANK = 0,	// 文字と文字の間
			CHAR_LEFT = 1,	//文字の右端
			CHAR_RIGHT = 2,	//文字の左端
			CHAR_STRING_LEFT = 3,	// 文字列全体の右端（始点）
			CHAR_STRING_RIGHT = 4;	// 文字列全体の左端（終点）
		static const int SIZE = 5;
	}BREAK_TYPE;

	typedef enum{
		TYPE4444,
		TYPE465,
		TYPE464
	}CREDIT_PATTERN;

	float _char_aspect_ratio;	// 文字のアスペクト比
	float _char_width_div;	// 文字の区切り位置ずれに対するペナルティ
	float _min_char_height_ratio;	// 画像の幅に対する最小文字高さの比
	float _max_char_height_ratio;	// 画像の幅に対する最大文字高さの比

	//! クレジットカード番号の位置を取得
	void ExtractNumbers(const cv::Mat& edge_img, std::vector<cv::Rect>& num_pos, CREDIT_PATTERN& pattern) const;

	/////////////////////////////////////
	//! クレジットカード番号列の位置を取得
	static void DetectStringHeight(const cv::Mat& edge_img, std::vector<cv::Rect>& candidates, int min_char_height, int max_char_height);

	/////////////////////////////////////

	//! アピアランスに基づいたコスト関数の生成
	static void CreateAppearanceCosts(const cv::Mat& edge_img, std::vector<std::vector<double>>& app_costs);

	//! 文字間の区切り位置に基づいたコスト関数の生成（正則化項）
	static void CreateRegularizationCosts(std::vector<double>& reg_costs, int window_size, double sigma);


	//! まず文字列の両端を算出したあとで、領域を均等割りしてそれぞれで最適な場所を算出
	/*!
	\param[out] char_breaks 文字の区切り位置
	\param[in] app_costs 見た目ベースのコスト関数
	\param[in] pos_costs 文字の位置ズレのコスト関数
	\param[in] avg_string_len 文字列の長さの平均
	\param[in] sring_len_div 文字列の長さの標準偏差
	\paran[in] char_pattern 区切り文字パターン
	*/
	static double ExtractCharRange(std::vector<int>& char_breaks, const std::vector<std::vector<double>>& app_costs,
		const std::vector<double>& pos_costs, float avg_string_len, float string_len_div,
		const std::vector<int>& char_pattern, double init_cost = 10000);

	//! クレジットカード番号のパターンを取得
	static void CreateCreditBreakPattern(std::vector<int>& pattern, CREDIT_PATTERN type = TYPE4444);
	static void ConvertXtoRects(const std::vector<int>& breaks, std::vector<cv::Rect>& number_rects, 
		const cv::Rect& region, const CREDIT_PATTERN& pattern);

private:
	std::vector<CREDIT_PATTERN> _PATTERN_TYPES;
	std::vector<std::vector<int>> _CHAR_BREAK_PATTERNS;

	//! カード番号のある行から文字間の区切り位置を算出
	/*!
	\param[in] edge_img エッジ画像
	\param[in] number_area 文字列領域
	\param[out] break_pos 文字の区切り位置
	\param[out] pattern クレジットカード番号の並び方（4-4-4-4, 4-6-5, 4-6-4）
	\param[in] min_cost 最小コスト。計算の足切りに使用。
	\return 最小コスト。小さいほど「それっぽい」。
	*/
	double DetectCharacterRange(const cv::Mat& edge_img, const cv::Rect& number_area, std::vector<int>& break_pos, CREDIT_PATTERN& pattern, double min_cost = 10000) const;

	//! カード番号のある行から文字間の区切り位置を算出
	/*!
	\param[in] edge_img エッジ画像
	\param[in] number_area 文字列領域
	\param[out] char_boxes 文字領域
	\param[out] pattern クレジットカード番号の並び方（4-4-4-4, 4-6-5, 4-6-4）
	\return 最小コスト。小さいほど「それっぽい」。
	*/
	double DetectCharacterBoxes(const cv::Mat& edge_img, const std::vector<cv::Rect>& number_area, std::vector<cv::Rect>& char_boxes, CREDIT_PATTERN& pattern) const;

	//! クレジットカード番号のパターンを取得
	//static void CreateCreditBreakPattern(std::vector<int>& pattern, CREDIT_PATTERN type = TYPE4444);

	//! コストマップ生成
	static void CreateCharLeftCost(const cv::Mat& derivmap, std::vector<double>& char_left_cost, int slide = 1);
	static void CreateCharRightCost(const cv::Mat& derivmap, std::vector<double>& char_right_cost, int slide = 1);
	static void CreateCharStringLeftCost(const cv::Mat& block_deriv, const cv::Mat& integ, std::vector<double>& char_string_left_cost, int slide = 1);
	static void CreateCharStringRightCost(const cv::Mat& block_deriv, const cv::Mat& integ, std::vector<double>& char_string_right_cost, int slide = 1);
	//static void CreateCharStringLeftCost(const std::vector<double>& char_left_cost, const cv::Mat& integ, std::vector<double>& char_string_left_cost);
	//static void CreateCharStringRightCost(const std::vector<double>& char_right_cost, const cv::Mat& integ, std::vector<double>& char_string_right_cost);
	static void CreateCharBlankCost(const cv::Mat& derivmap, std::vector<double>& char_blank_cost);

	//! 勾配（微分）
	static void CreateDeriv(const cv::Mat& prj, cv::Mat& div1st, cv::Mat& div2nd);

	//! ブロック毎の勾配
	static void CreateBlockDeriv(const cv::Mat& prj, int block_size, cv::Mat& dst);

	// 結果を格納
	//static void ConvertXtoRects(const std::vector<int>& breaks, std::vector<cv::Rect>& number_rects, 
	//	const cv::Rect& region, const CREDIT_PATTERN& pattern);

	static void InitPositions(const std::vector<double>& app_costs, std::vector<double>& target_costs, std::vector<int>& positions);

	static void MinScorePositions(const std::vector<double>& app_costs, const std::vector<double>& size_costs, 
		int pos, double* min_cost, int* min_position);

	//! クレジットカード番号列の尤度評価
	static void EvaluateNumberStrings(const std::vector<std::pair<int,int>>& line_pos, std::vector<double>& scores, const std::vector<float>& prj);
};

}
#endif
