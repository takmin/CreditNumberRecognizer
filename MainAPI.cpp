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

#include "MainAPI.h"
#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <boost/filesystem/path.hpp>
#include "util.h"

MainAPI::MainAPI(void)
{
}


MainAPI::~MainAPI(void)
{
}


void MainAPI::CreateFeature(const std::string& img_file, const std::string& save_file)
{
	cv::Mat img = cv::imread(img_file);
	if(img.empty()){
		std::cerr << "Fail to read " << img_file << std::endl;
		return;
	}
	cv::Mat feature;
	CCNR.CreateFeature(img, feature);

	cv::FileStorage fs(save_file, cv::FileStorage::WRITE);
	if(!fs.isOpened()){
		std::cerr << "Fail to open " << save_file << std::endl;
		return;
	}
	fs << "Features" << feature;
}


void MainAPI::CreateTrainingFeatures(const std::string& directory, const std::string& save_file)
{
	std::vector<std::string> img_list;
	bool ret = ReadImageFilesInDirectory(directory, img_list);
	if(!ret){
		std::cerr << "Fail to load images in " << directory << std::endl;
		return;
	}

	cv::Mat features;
	CCNR.CreateFeatures(img_list, features);

	cv::FileStorage fs(save_file, cv::FileStorage::WRITE);
	if(!fs.isOpened()){
		std::cerr << "Fail to open " << save_file << std::endl;
		return;
	}

	fs << "Features" << features;
}


void MainAPI::CreateTrainingAllFeatures(const std::string& directory, const std::string& save_directory)
{
	using namespace boost::filesystem;

	for(int i=0; i<11; i++){
		path dir, save_file;
		if(i==10){
			dir = path(directory) / path("bg");
			save_file = path(save_directory) / path("bg.txt");
		}
		else{
			dir = path(directory) / path(Int2String(i));
			save_file = path(save_directory) / path(Int2String(i) + ".txt");
		}
		CreateTrainingFeatures(dir.generic_string(), save_file.generic_string());
	}
}


void MainAPI::LoadClassifier(const std::string& filename)
{
	if(CCNR.LoadClassifier(filename) < 0){
		std::cerr << "Fail to load " << filename << std::endl;
	}
}



void MainAPI::Recognize(const std::string& img_file, const std::string& save_name, bool display)
{
	cv::Mat card_img = cv::imread(img_file);
	if(card_img.empty()){
		std::cerr << "Fail to read " << img_file << std::endl;
		return;
	}

	std::vector<int> numbers;
	std::vector<cv::Rect> num_pos;
	CCNR.RecognizeCreditCardNumber(card_img, numbers, num_pos);
	
	if(numbers.empty()){
		std::cerr << "Fail to recognize. Classifier may not be loaded." << std::endl;
		return;
	}

	int num = numbers.size();
	double font_scale = (double)card_img.cols / 300;
	for(int i=0; i<num; i++){
		cv::rectangle(card_img, num_pos[i], cv::Scalar(0,0,255));
		cv::putText(card_img, Int2String(numbers[i]), cv::Point(num_pos[i].x, num_pos[i].y), cv::FONT_HERSHEY_PLAIN, font_scale, cv::Scalar(0,0,255), 2);
		std::cout << numbers[i];
		if(i < num-1)
			std::cout << ",";
	}
	std::cout << std::endl;
	if(display){
		cv::namedWindow("Recognize");
		cv::imshow("Recognize", card_img);
		cv::waitKey();
		cv::destroyWindow("Recognize");
	}

	bool ret = cv::imwrite(save_name, card_img);
	if(!ret){
		std::cerr << "Fail to save " << save_name << std::endl;
	}
	else{
		std::cout << "Save " << save_name << std::endl;
	}
}


void MainAPI::RecognizeFolder(const std::string& directory, const std::string& save_dir)
{
	std::vector<std::string> img_list;
	bool ret = ReadImageFilesInDirectory(directory, img_list);
	if(!ret){
		std::cerr << "Fail to load images in " << directory << std::endl;
		return;
	}
	
	std::vector<std::string>::iterator it, it_end = img_list.end();
	for(it = img_list.begin(); it != it_end; it++){
		boost::filesystem::path save_file_path = boost::filesystem::path(save_dir) / boost::filesystem::path(*it).stem();
		std::string save_file = save_file_path.generic_string() + ".png";
		Recognize(*it, save_file, false);
	}
}


void MainAPI::RecognizeVideoCapture()
{
	cv::VideoCapture cap(0);

	cv::Mat card_img;
	cap >> card_img;
	cv::Rect roi;
	roi.width = card_img.cols * 0.8;
	roi.height = roi.width * 0.6;
	roi.x = (card_img.cols - roi.width) / 2;
	roi.y = (card_img.rows - roi.height) / 2;

	cv::namedWindow("image");
	bool loop = true;
	while (loop) {
		cap >> card_img;
		if (card_img.empty())
			loop = false;
		cv::Mat draw_im = card_img.clone();
		cv::rectangle(draw_im, roi, cv::Scalar(255, 0, 0), 2);
		cv::imshow("image", draw_im);
		int key = cv::waitKey(1);
		if (key > 31 && key < 128)
			loop = false;
	}

	std::vector<int> numbers;
	std::vector<cv::Rect> num_pos;
	CCNR.RecognizeCreditCardNumber(card_img(roi).clone(), numbers, num_pos);

	if (numbers.empty()) {
		std::cerr << "Fail to recognize. Classifier may not be loaded." << std::endl;
		return;
	}

	int num = numbers.size();
	double font_scale = (double)card_img.cols / 300;
	for (int i = 0; i<num; i++) {
		cv::Rect pos = num_pos[i];
		pos.x += roi.x;
		pos.y += roi.y;
		cv::rectangle(card_img, pos, cv::Scalar(0, 0, 255));
		cv::putText(card_img, Int2String(numbers[i]), cv::Point(pos.x, pos.y), cv::FONT_HERSHEY_PLAIN, font_scale, cv::Scalar(0, 0, 255), 2);
		std::cout << numbers[i];
		if (i < num - 1)
			std::cout << ",";
	}
	std::cout << std::endl;
	cv::imshow("image", card_img);
	cv::waitKey();
	cv::destroyWindow("image");
}
