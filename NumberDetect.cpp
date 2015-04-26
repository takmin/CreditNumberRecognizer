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

#include <opencv2/imgproc/imgproc.hpp>
#include "common.h"
#include "NumberDetect.h"
#include "Mser1D.hpp"


namespace ccnr{

NumberDetect::NumberDetect(void)
{
	_char_aspect_ratio = 1.5;
	_char_width_div = 0.2;
	_min_char_height_ratio = 0.05;
	_max_char_height_ratio = 0.1;
	_PATTERN_TYPES.push_back(CREDIT_PATTERN::TYPE4444);
	_PATTERN_TYPES.push_back(CREDIT_PATTERN::TYPE465);
	_PATTERN_TYPES.push_back(CREDIT_PATTERN::TYPE464);
	std::vector<CREDIT_PATTERN>::iterator it, it_end = _PATTERN_TYPES.end();
	for(it=_PATTERN_TYPES.begin(); it != it_end; it++){
		std::vector<int> break_pattern;
		CreateCreditBreakPattern(break_pattern, *it);
		_CHAR_BREAK_PATTERNS.push_back(break_pattern);
	}
}


NumberDetect::~NumberDetect(void)
{
}

//! クレジットカード番号の位置を取得
void NumberDetect::ExtractNumbers(const cv::Mat& edge_img, std::vector<cv::Rect>& num_pos, CREDIT_PATTERN& pattern) const
{
	// クレジットカード番号列の位置を取得
	std::vector<cv::Rect> candidates;
	int min_char_height = round(_min_char_height_ratio * edge_img.cols);
	int max_char_height = round(_max_char_height_ratio * edge_img.cols);
	DetectStringHeight(edge_img, candidates, min_char_height, max_char_height);

	double cost = DetectCharacterBoxes(edge_img, candidates, num_pos, pattern);
}


void NumberDetect::ConvertXtoRects(const std::vector<int>& breaks, std::vector<cv::Rect>& number_rects, 
	const cv::Rect& region, const CREDIT_PATTERN& pattern)
{
	if(breaks.empty())
		return;

	// クレジットカード番号領域格納
	std::vector<int> break_b, break_e;
	if(pattern == CREDIT_PATTERN::TYPE4444){
		int b[] = {0,5,10,15};
		int e[] = {4,9,14,19};
		break_b.insert(break_b.end(),b,b+4);
		break_e.insert(break_e.end(),e,e+4);
	}
	else if(pattern == CREDIT_PATTERN::TYPE465){
		int b[] = {0,5,12};
		int e[] = {4,11,17};
		break_b.insert(break_b.end(),b,b+3);
		break_e.insert(break_e.end(),e,e+3);
	}
	else if(pattern == CREDIT_PATTERN::TYPE464){
		int b[] = {0,5,12};
		int e[] = {4,11,16};
		break_b.insert(break_b.end(),b,b+3);
		break_e.insert(break_e.end(),e,e+3);
	}
	for(int j=0; j<break_b.size(); j++){
		for(int i=break_b[j]; i<break_e[j]; i++){
			cv::Rect rect(breaks[i], region.y, breaks[i+1] - breaks[i], region.height);
			number_rects.push_back(rect);
		}
	}
}


void NumberDetect::EvaluateNumberStrings(const std::vector<std::pair<int,int>>& line_pos, std::vector<double>& scores, const std::vector<float>& prj)
{
	// カード番号の位置は中心よりやや下あたり
	float myu = 0.6 * prj.size();	// 平均
	float delta = 0.5 * prj.size();	//標準偏差

	std::vector<std::pair<int,int>>::const_iterator cit, cit_end = line_pos.end();
	for(cit = line_pos.begin(); cit != cit_end; cit++){
		float center = (float)cit->second / 2 + cit->first;
		float tmp = (center - myu) / delta;
		float score = std::exp(- tmp*tmp/2) / (std::sqrt(2.0*CV_PI) * delta);
		scores.push_back(score);
	}
}


void NumberDetect::DetectStringHeight(const cv::Mat& edge_img, std::vector<cv::Rect>& candidates, int min_char_height, int max_char_height)
{
	cv::Mat prj;
	Projection(edge_img, prj, true);
	
	int filter_width = edge_img.cols / 80;
	filter_width = (filter_width < 3) ? 3 : filter_width + (1 - filter_width % 2);

	cv::Mat gprj;
	cv::GaussianBlur(prj, gprj, cv::Size(1,filter_width), 0.0, 1.0);

	std::vector<float> gprj_vec;
	Mat2Vector(gprj, gprj_vec);

	std::vector<std::pair<int,int>> msers;
	Mser1D(gprj_vec, msers, 1.0, 2.0, min_char_height, max_char_height);
//	Mser1D(gprj_vec, msers, 1.0, 2.0, 20, 32);

	if(msers.empty())
		return;

	std::vector<double> scores;
	EvaluateNumberStrings(msers, scores, gprj_vec);

	std::vector<int> idx;
	argsort_vector(scores, idx);

	double max_score = scores[idx[idx.size()-1]];

	for(int i=idx.size() -1; i>=0; i--){
		if(scores[idx[i]] / max_score < 0.90)
			break;
		cv::Rect rect(0, msers[idx[i]].first, edge_img.cols, msers[idx[i]].second);
		candidates.push_back(rect);
	}
}



void NumberDetect::CreateCreditBreakPattern(std::vector<int>& pattern, CREDIT_PATTERN type)
{
	pattern.push_back(BREAK_TYPE::CHAR_STRING_LEFT);

	if(type == CREDIT_PATTERN::TYPE4444){
		for(int j=0; j<4; j++){
			for(int i=0; i<3; i++){
				pattern.push_back(BREAK_TYPE::CHAR_BLANK);
			}
			if(j<3){
				pattern.push_back(BREAK_TYPE::CHAR_RIGHT);
				pattern.push_back(BREAK_TYPE::CHAR_LEFT);
			}
		}
	}
	else if(type == CREDIT_PATTERN::TYPE465){
		for(int i=0; i<3; i++){
			pattern.push_back(BREAK_TYPE::CHAR_BLANK);
		}
		pattern.push_back(BREAK_TYPE::CHAR_RIGHT);
		pattern.push_back(BREAK_TYPE::CHAR_LEFT);
		for(int i=0; i<5; i++){
			pattern.push_back(BREAK_TYPE::CHAR_BLANK);
		}
		pattern.push_back(BREAK_TYPE::CHAR_RIGHT);
		pattern.push_back(BREAK_TYPE::CHAR_LEFT);
		for(int i=0; i<4; i++){
			pattern.push_back(BREAK_TYPE::CHAR_BLANK);
		}
	}
	else if(type == CREDIT_PATTERN::TYPE464){
		for(int i=0; i<3; i++){
			pattern.push_back(BREAK_TYPE::CHAR_BLANK);
		}
		pattern.push_back(BREAK_TYPE::CHAR_RIGHT);
		pattern.push_back(BREAK_TYPE::CHAR_LEFT);
		for(int i=0; i<5; i++){
			pattern.push_back(BREAK_TYPE::CHAR_BLANK);
		}
		pattern.push_back(BREAK_TYPE::CHAR_RIGHT);
		pattern.push_back(BREAK_TYPE::CHAR_LEFT);
		for(int i=0; i<3; i++){
			pattern.push_back(BREAK_TYPE::CHAR_BLANK);
		}
	}
	pattern.push_back(BREAK_TYPE::CHAR_STRING_RIGHT);
}


void NumberDetect::CreateCharLeftCost(const cv::Mat& derivmap, std::vector<double>& char_left_cost, int slide)
{
	assert(derivmap.type() == CV_64FC1);
	cv::Mat tmp_map, edgecost;
	cv::exp(-derivmap, tmp_map);
	cv::log(tmp_map + 1, edgecost);

	int num = edgecost.total();
	double* ptr = (double*)edgecost.data;
	for(int i=slide; i<num; i++){
		char_left_cost.push_back(ptr[i]);
	}
	for(int i=0; i<slide; i++){
		char_left_cost.push_back(1.0);
	}
}


void NumberDetect::CreateCharRightCost(const cv::Mat& derivmap, std::vector<double>& char_right_cost, int slide)
{
	assert(derivmap.type() == CV_64FC1);

	cv::Mat tmp_map, edgecost;
	cv::exp(derivmap, tmp_map);
	cv::log(tmp_map + 1, edgecost);

	int num = edgecost.total();
	double* ptr = (double*)edgecost.data;
	for(int i=0; i<slide; i++){
		char_right_cost.push_back(1.0);
	}
	for(int i=0; i<num-slide; i++){
		char_right_cost.push_back(ptr[i]);
	}
}


void NumberDetect::CreateCharStringLeftCost(const cv::Mat& block_deriv, const cv::Mat& integ, std::vector<double>& char_string_left_cost, int slide)
{
	assert(integ.type() == CV_64FC1);
	double _epsilon = 0.00001;
	double total = integ.at<double>(integ.rows-1, integ.cols-1) + _epsilon;
	cv::Mat integ2 = (-integ(cv::Rect(1,1,integ.cols-1,integ.rows-1)) + total) / total;

	std::vector<double> char_left_cost;
	CreateCharLeftCost(block_deriv, char_left_cost, slide);
	int full_size = char_left_cost.size();

	for(int i=0; i<full_size; i++){
		//char_string_left_cost.push_back(- std::log(integ2.at<double>(0,i)));
		//char_string_left_cost.push_back(char_left_cost[i]);
		char_string_left_cost.push_back(char_left_cost[i] - std::log(integ2.at<double>(0,i)));
	}
}


void NumberDetect::CreateCharStringRightCost(const cv::Mat& block_deriv, const cv::Mat& integ, std::vector<double>& char_string_right_cost, int slide)
{
	assert(integ.type() == CV_64FC1);
	double _epsilon = 0.00001;
	double total = integ.at<double>(integ.rows-1, integ.cols-1) + _epsilon;
	cv::Mat integ2 = (integ(cv::Rect(1,1,integ.cols-1,integ.rows-1)) + _epsilon)/ total;

	std::vector<double> char_right_cost;
	CreateCharRightCost(block_deriv, char_right_cost, slide);
	int full_size = char_right_cost.size();

	for(int i=0; i<full_size; i++){
		//char_string_right_cost.push_back(- std::log(integ2.at<double>(0,i)));
		//char_string_right_cost.push_back(char_right_cost[i]);
		char_string_right_cost.push_back(char_right_cost[i] - std::log(integ2.at<double>(0,i)));
	}
}


void NumberDetect::CreateCharBlankCost(const cv::Mat& derivmap, std::vector<double>& char_blank_cost)
{
	assert(derivmap.type() == CV_64FC1);
	cv::Mat tmp_map, edgecost;
	cv::exp(-derivmap, tmp_map);
	cv::log(tmp_map + 1, edgecost);

	int num = edgecost.total();
	double* ptr = (double*)edgecost.data;
	for(int i=0; i<num; i++){
		char_blank_cost.push_back(ptr[i]);
	}
}


//! 勾配（微分）	
void NumberDetect::CreateDeriv(const cv::Mat& prj, cv::Mat& deriv1st, cv::Mat& deriv2nd)
{
	cv::Mat derivfilter(1,3,CV_64FC1);
	derivfilter.at<double>(0,0) = -0.5;
	derivfilter.at<double>(0,1) = 0;
	derivfilter.at<double>(0,2) = 0.5;
	cv::filter2D(prj, deriv1st, prj.depth(), derivfilter);	//１次微分
	cv::filter2D(deriv1st, deriv2nd, prj.depth(), derivfilter);		//２次微分
}


//! 数字列の端点のコスト算出のための勾配算出
void NumberDetect::CreateBlockDeriv(const cv::Mat& prj, int block_size, cv::Mat& dst)
{
	block_size += (1-block_size%2);

	cv::Mat tprj;
	cv::boxFilter(prj, tprj, CV_64FC1, cv::Size(block_size, 1), cv::Point(-1,-1), true);
	cv::Mat filter = cv::Mat::zeros(1, block_size+2, CV_64FC1); 
	filter.at<double>(0,0) = -0.1;
	filter.at<double>(0,block_size+1) = 0.1;
	cv::filter2D(tprj, dst, CV_64FC1, filter);
}


//! アピアランスに基づいたコスト関数の生成
void NumberDetect::CreateAppearanceCosts(const cv::Mat& edge_img, std::vector<std::vector<double>>& app_costs)
{
	cv::Mat prj, nprj, gprj;
	Projection(edge_img, prj);

	int filter_width = edge_img.cols / 80;
	filter_width = (filter_width < 3) ? 3 : filter_width + (1 - filter_width % 2);
	//cv::GaussianBlur(prj, gprj, cv::Size(filter_width,1), 1.0, 1.0);
	//cv::normalize(gprj, nprj, 100.0, 0.0, cv::NORM_MINMAX, CV_64FC1);
	cv::normalize(prj, gprj, 100.0, 0.0, cv::NORM_MINMAX, CV_64FC1);
	cv::GaussianBlur(gprj, nprj, cv::Size(filter_width,1), 1.0, 1.0);

	// 積分分布
	cv::Mat integ;
	cv::integral(gprj, integ);

	// プロジェクションの微分
	cv::Mat derivmap, derivmap2nd;
	CreateDeriv(nprj, derivmap, derivmap2nd);

	// ブロック単位の微分
	cv::Mat blockderiv;
	int block_size = edge_img.rows;
	CreateBlockDeriv(gprj, block_size, blockderiv);

	app_costs.resize(BREAK_TYPE::SIZE);

	//BREAK_TYPE::CHAR_LEFT：上り勾配の大きさを少し左にずらす
	std::vector<double> char_left_costs;
	CreateCharLeftCost(derivmap, char_left_costs, 1);
	app_costs[BREAK_TYPE::CHAR_LEFT] = char_left_costs;

	//BREAK_TYPE::CHAR_RIGHT：下り勾配の大きさを少し右にずらす
	std::vector<double> char_right_costs;
	CreateCharRightCost(derivmap, char_right_costs, 1);
	app_costs[BREAK_TYPE::CHAR_RIGHT] = char_right_costs;

	//BREAK_TYPE::CHAR_STRING_LEFT：CHAR_LEFTに場所による重み付け
	std::vector<double> char_string_left_costs;
	//CreateCharStringLeftCost(char_left_costs, integ, char_string_left_costs);
	CreateCharStringLeftCost(blockderiv, integ, char_string_left_costs, 1);
	app_costs[BREAK_TYPE::CHAR_STRING_LEFT] = char_string_left_costs;

	//BREAK_TYPE::CHAR_STRING_RIGHT：CHAR_RIGHTに場所による重み付け
	std::vector<double> char_string_right_costs;
	//CreateCharStringRightCost(char_right_costs, integ, char_string_right_costs);
	CreateCharStringRightCost(blockderiv, integ, char_string_right_costs, 1);
	app_costs[BREAK_TYPE::CHAR_STRING_RIGHT] = char_string_right_costs;

	//BREAK_TYPE::CHAR_BLANK：二次微分の大きさ
	std::vector<double> char_blank_costs;
	CreateCharBlankCost(derivmap2nd, char_blank_costs);
	app_costs[BREAK_TYPE::CHAR_BLANK] = char_blank_costs;
}


//! 文字間の長さに基づいたコスト関数の生成（正則化項）
void NumberDetect::CreateRegularizationCosts(std::vector<double>& reg_costs, int window_size, double sigma)
{
	int half_size = window_size / 2 + window_size % 2;
	for(int i=0; i<half_size; i++){
		double diff = (double)i/sigma;
		reg_costs.push_back((diff * diff) / 2.0);
	}
}


void NumberDetect::InitPositions(const std::vector<double>& app_costs, std::vector<double>& target_costs, std::vector<int>& positions)
{
	argsort_vector(app_costs, positions);
	target_costs.clear();
	std::vector<int>::iterator it, it_end = positions.end();
	for(it = positions.begin(); it != it_end; it++){
		target_costs.push_back(app_costs[*it]);
	}
}


void NumberDetect::MinScorePositions(const std::vector<double>& app_costs, const std::vector<double>& size_costs, 
	int pos, double* min_cost, int* min_position)
{
	int bi = - size_costs.size() + 1;
	int bp = pos + bi;
	int ep = pos + size_costs.size();
	if(bp < 0){
		bi -= bp;
		bp = 0;
	}
	if(ep >= app_costs.size())
		ep = app_costs.size() - 1;

	std::vector<double> costs;
	for(int p = bp, i=bi; p<ep; p++, i++){
		costs.push_back(app_costs[p] + size_costs[std::abs(i)]);
	}

	int idx = min_arg(costs, *min_cost);
	*min_position = bp + idx;
}



//! まず文字列の両端を算出したあとで、領域を均等割りしてそれぞれで最適な場所を算出
double NumberDetect::ExtractCharRange(std::vector<int>& char_breaks, const std::vector<std::vector<double>>& app_costs,
	const std::vector<double>& pos_costs, float avg_string_len, float string_len_div, const std::vector<int>& char_pattern, double init_cost)
{
	std::vector<double> start_costs, end_costs;
	std::vector<int> start_pos, end_pos;

	int ptn_size = char_pattern.size();

	// コスト関数に基づいて、数字列の左端と右端をソート
	InitPositions(app_costs[char_pattern[0]], start_costs, start_pos);
	InitPositions(app_costs[char_pattern[ptn_size-1]], end_costs, end_pos);

	std::vector<int> cur_char_breaks(ptn_size);

	int max_string_width = app_costs[0].size();
	int min_string_width = max_string_width / 3;
	double min_cost = init_cost;
	for(int s=0; s<start_costs.size(); s++){
		double cur_cost = start_costs[s];
		cur_char_breaks[0] = start_pos[s];
		if(cur_cost >= min_cost)
			break;

		for(int e=0; e<end_costs.size(); e++){
			double cur_cost2 = cur_cost + end_costs[e];
			cur_char_breaks[ptn_size-1] = end_pos[e];

			int char_str_range = end_pos[e] - start_pos[s];
			if(char_str_range < min_string_width)
				continue;
			float diff = ((avg_string_len - char_str_range) / string_len_div);
			cur_cost2 += (diff * diff / 2.0);
			if(cur_cost2 >= min_cost)
				break;
			//if(char_str_range <= 0)
			//	continue;

			float char_size = (float)char_str_range / (ptn_size - 1);
			for(int p=1; p<ptn_size-1; p++){
				int app_idx = char_pattern[p];
				double target_cost;
				int position;
				MinScorePositions(app_costs[app_idx], pos_costs, start_pos[s] + round(char_size * p), &target_cost, &position);
				cur_cost2 += target_cost;
				cur_char_breaks[p] = position;
				if(cur_cost2 >= min_cost)
					break;
				if(p == ptn_size - 2){
					min_cost = cur_cost2;
					char_breaks = cur_char_breaks;
				}
			}
		}
	}
	return min_cost;
}



double NumberDetect::DetectCharacterRange(const cv::Mat& edge_img, const cv::Rect& area, std::vector<int>& break_pos, CREDIT_PATTERN& pattern, double min_cost) const
{
	std::vector<std::vector<double>> app_costs;
	CreateAppearanceCosts(edge_img(area).clone(), app_costs);

	std::vector<double> reg_costs, length_costs;
	float char_size = (float)area.height/_char_aspect_ratio;
	int win_size = char_size / 2;
	win_size += (win_size + 1) % 2;	// 奇数に
	CreateRegularizationCosts(reg_costs, win_size, _char_width_div * char_size);

	std::vector<double> min_costs;
	std::vector<std::vector<int>> char_break_positions;
	int min_idx = 0;
	for(int i=0; i<_PATTERN_TYPES.size(); i++){
		float avg_length = char_size * (_CHAR_BREAK_PATTERNS[i].size() - 1);
		std::vector<int> char_break_pos;
		double cost = 
			ExtractCharRange(char_break_pos, app_costs, reg_costs, avg_length, _char_width_div * avg_length, _CHAR_BREAK_PATTERNS[i], min_cost);
		if(cost < min_cost && !char_break_pos.empty()){
			min_cost = cost;
			min_idx = i;
			break_pos = char_break_pos;
		}
	}
	pattern = _PATTERN_TYPES[min_idx];

	return min_cost;
}


double NumberDetect::DetectCharacterBoxes(const cv::Mat& edge_img, const std::vector<cv::Rect>& number_area, std::vector<cv::Rect>& char_boxes, CREDIT_PATTERN& pattern) const
{
	std::vector<int> min_break_pos;
	double min_cost = 10000;
	int cand_size = number_area.size();
	int min_i = -1;
	for(int i = 0;i<cand_size;i++){
		CREDIT_PATTERN cur_pattern;
		std::vector<int> break_pos;
		double cost = DetectCharacterRange(edge_img, number_area[i], break_pos, cur_pattern, min_cost);
		if(cost < min_cost){
			min_cost = cost;
			min_break_pos = break_pos;
			pattern = cur_pattern;
			min_i = i;
		}
	}

	// クレジットカード番号領域格納
	if(min_i >= 0){
		ConvertXtoRects(min_break_pos, char_boxes, number_area[min_i], pattern);
	}
	return min_cost;
}

}