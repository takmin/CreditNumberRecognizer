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

#ifndef __MSER1D__
#define __MSER1D__

#include <vector>
#include <algorithm>

namespace ccnr{

//! 各しきい値における領域の構造体
typedef struct{
	int pos;	// 開始位置
	int len;	// 長さ
	double threshold;	// しきい値
	std::vector<int> child_idx;	// 子ノード（しきい値をより大きくした場合）
}REGION_1D;


//! 1次元マスクをかける
template<typename _T>
void Mask1D(const std::vector<_T>& histogram, std::vector<_T>& masked_histogram, const std::vector<unsigned char>& mask)
{
	if(mask.empty()){
		masked_histogram = histogram;
		return;
	}
	
	assert(mask.size() == histogram.size());

	std::vector<_T>::const_iterator min_it = std::min_element(histogram.begin(), histogram.end());

	masked_histogram.resize(histogram.size(), *min_it);
	int hist_len = histogram.size();
	for(int i = 0; i < hist_len; i++){
		if(mask[i] > 0){
			masked_histogram[i] = histogram[i];
		}
	}
}



//! １次元配列からしきい値を変えながら再帰的に連続領域の入れ子構造を抽出
/*!
\param[in] histogram 1次元配列
\param[out] regions 木構造を持った領域
\param[in] rgn_id 現在参照している領域のID（regionsのインデックス）
\param[in] step しきい値を上げるステップ幅
*/
template <typename _T>
void CreateMserRegionTree(const std::vector<_T>& histogram, std::vector<REGION_1D>& regions, 
	int rgn_id, double step)
{
	int start, end;
	double th;
	// 初期状態の場合
	if(regions.empty()){
		start = 0;
		end = histogram.size();
		std::vector<_T>::const_iterator min_it = std::min_element(histogram.begin(), histogram.end());
		th = (double)(*min_it) - 0.000001;
	}
	else{
		start = regions[rgn_id].pos;
		end = start + regions[rgn_id].len;
		th = regions[rgn_id].threshold + step;
	}
	for(int i=start; i<end; i++){
		if(histogram[i] >= th){
			REGION_1D rgn;
			rgn.pos = i;
			int count = 0;
			while(i<end && histogram[i] >= th){
				count ++;
				i++;
			}
			rgn.len = count;
			rgn.threshold = th;
			int new_rgn_id = regions.size();
			if(new_rgn_id > 0){
				regions[rgn_id].child_idx.push_back(new_rgn_id);
			}
			regions.push_back(rgn);

			CreateMserRegionTree(histogram, regions, new_rgn_id, step);
		}
	}
}



//! 木を分解してクラスタリング(小ノード数が１である連続領域)
/*!
\param[in] tree_regions 全しきい値での全領域とその木構造
\param[out] clustered_idx 木構造を子ノードが１となる領域でクラスタリングしたID（しきい値小から大の順に）
*/
void ClusterRegionsFromTree(const std::vector<REGION_1D>& tree_regions, std::vector<std::vector<int> >& clustered_idx)
{
	std::vector<bool> check(tree_regions.size(), false);
	for(int i=0; i<tree_regions.size(); i++){
		if(!check[i]){
			std::vector<int> idx_chain;
			idx_chain.push_back(i);
			check[i] = true;
			int j = i;
			while(tree_regions[j].child_idx.size() == 1){
				j = tree_regions[j].child_idx[0];
				idx_chain.push_back(j);
				check[j] = true;
			}
			clustered_idx.push_back(idx_chain);
		}
	}
}


//! 領域の変化量を算出
/*!
\param[in] tree_regions 全しきい値での全領域とその木構造
\param[out] area_variation clustered_idxに対応する面積の変化量
\param[in] clustered_idx 木構造を子ノードが１となる領域でクラスタリングしたID（しきい値小から大の順に）
*/
void AreaVariation(const std::vector<REGION_1D>& tree_regions, 
	std::vector<std::vector<double> >& area_variation, 
	const std::vector<std::vector<int> >& clustered_idx,
	int delta, int min_area, int max_area)
{
	std::vector<std::vector<int> >::const_iterator it, it_end = clustered_idx.end();
	for(it = clustered_idx.begin(); it != it_end; it++){
		std::vector<double> variation;
		if(it->size() < 2*delta + 1 || 
			tree_regions[it->front()].len < min_area || tree_regions[it->back()].len > max_area){
			area_variation.push_back(variation);
			continue;
		}
		
		for(int i=0; i<delta; i++){
			variation.push_back(-1);
		}
		int end = it->size() - delta;
		for(int i=delta; i<end; i++){
			int cand_idx = (*it)[i];
			double val = (double)(tree_regions[cand_idx-delta].len - tree_regions[cand_idx+delta].len) / tree_regions[cand_idx].len;
			variation.push_back(val);
		}
		for(int i=0; i<delta; i++){
			variation.push_back(-1);
		}
		area_variation.push_back(variation);
	}
}



//! 領域の変化量の局所的に最小なインデックスを算出
/*!
\param[in] clustered_idx 木構造を子ノードが１となる領域でクラスタリングしたID（しきい値小から大の順に）
\param[in] area_variation clustered_idxに対応する面積の変化量
\param[in] mser_idx area_variationで極小値をとるインデックス
*/
void GetLocalVariationMaxima(const std::vector<std::vector<int> >& clustered_idx,
	const std::vector<std::vector<double> >& area_variation, 
	std::vector<int>& mser_idx)
{
	int num_vec = area_variation.size();

	if(num_vec != clustered_idx.size()){
		throw std::exception("num_vec != clustered_idx.size()");
	}

	for(int i=0; i<num_vec; i++){
		double min = 10000;
		int min_id = -1;
		int num_rgn = area_variation[i].size();
		for(int j=0; j<num_rgn; j++){
			if(area_variation[i][j] >= 0){
				if(min > area_variation[i][j]){
					min = area_variation[i][j];
					min_id = j;
				}
			}
		}
		if(min_id >= 0)
			mser_idx.push_back(clustered_idx[i][min_id]);
	}
/*	for(int i=0; i<num_vec; i++){
		double prev = -1;
		int num_rgn = area_variation[i].size();
		for(int j=0; j<num_rgn; j++){
			if(area_variation[i][j] >= 0){
				if(prev > 0 && prev < area_variation[i][j]){
					mser_idx.push_back(clustered_idx[i][j-1]);
				}
				prev = area_variation[i][j];
			}
		}
	}*/
}


//! １次元Maximally Stable Extreme Regions (MSER)の抽出
/*!
\param[in] histogram １次元信号
\param[out] msers 抽出された領域。first: 開始位置、second：長さ
\param[in] step しきい値変化のステップ
\param[in] delta 局所的な極小値を求めるための幅
\param[in] min_area 検出最小サイズ
\param[in] max_area 検出最大サイズ
\param[in] mask 処理マスク
*/
template<typename _T> 
void Mser1D(const std::vector<_T>& histogram, std::vector<std::pair<int, int> >& msers, 
	double step = 1.0, double delta = 1.0, int min_area = 1, int max_area = -1, 
	const std::vector<unsigned char>& mask = std::vector<unsigned char>())
{
	// マスクをかける
	std::vector<_T> masked_histogram;
	Mask1D(histogram, masked_histogram, mask);
	
	// 小から大へしきい値を変化させながら、しきい値以上の連続領域を木構造として取得
	std::vector<REGION_1D> regions;
	CreateMserRegionTree(masked_histogram, regions, 0, step);

	// 木構造を分解してクラスタリング(小ノード数が１である連続領域)
	std::vector<std::vector<int> > clustered_region_indices;
	ClusterRegionsFromTree(regions, clustered_region_indices);

	// 領域の変化量を算出
	int i_delta = (int)(delta / step + 0.5);
	if(max_area < 1)
		max_area = histogram.size();
	std::vector<std::vector<double> > area_variation;
	AreaVariation(regions, area_variation, clustered_region_indices, i_delta, min_area, max_area);

	// 領域の変化量の局所的に最小なインデックスを算出
	std::vector<int> mser_idx;
	GetLocalVariationMaxima(clustered_region_indices, area_variation, mser_idx);

	// Maximally stable extremal regionsを取得
	std::vector<int>::iterator it, it_end = mser_idx.end();
	for(it = mser_idx.begin(); it != it_end; it++){
		std::pair<int,int> rgn;
		rgn.first = regions[*it].pos;
		rgn.second = regions[*it].len;
		msers.push_back(rgn);
	}
}

}
#endif