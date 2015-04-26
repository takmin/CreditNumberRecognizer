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

#ifndef __ARGSORT__
#define __ARGSORT__

#include <algorithm>

template <typename T>
struct ARG_SORTER
{
	T	val;
	int idx;

	bool operator<( const struct ARG_SORTER& right ) const{
		return val < right.val;
	}
};

template <typename T>
void argsort_vector(const std::vector<T>& vec, std::vector<int>& idx)
{
	int vec_size = vec.size();
	std::vector<struct ARG_SORTER<T>> sort_pairs;
	for(int i=0; i<vec_size; i++){
		struct ARG_SORTER<T> argsorter;
		argsorter.val = vec[i];
		argsorter.idx = i;
		sort_pairs.push_back(argsorter);
	}

	std::sort(sort_pairs.begin(), sort_pairs.end()); 
	
	idx.clear();
	for(int i=0; i<vec_size; i++){
		idx.push_back(sort_pairs[i].idx);
	}
}

template<typename T> 
int max_arg(const std::vector<T>& vec, T& max_val)
{
	if(vec.empty())
		return -1;
	int num = vec.size();
	int max_i = 0;
	max_val = vec[0];
	for(int i=1; i<num; i++){
		if(max_val < vec[i]){
			max_val = vec[i];
			max_i = i;
		}
	}
	return max_i;
}


template<typename T> 
int min_arg(const std::vector<T>& vec, T& min_val)
{
	if(vec.empty())
		return -1;
	int num = vec.size();
	int min_i = 0;
	min_val = vec[0];
	for(int i=1; i<num; i++){
		if(min_val > vec[i]){
			min_val = vec[i];
			min_i = i;
		}
	}
	return min_i;
}

#endif