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

#if _TEST_
#pragma comment(lib, "C:\\ProgramFolders\\gtest\\lib\\gtest.lib")
#include <gtest/gtest.h>
#endif

#include <iostream>
#include "MainAPI.h"


std::string AskQuestionGetString(const std::string& question){
	std::cout << question;
	std::string ans;
	std::cin >> ans;
	return ans;
}


int AskQuestionGetInt(const std::string& question){
	std::string ans = AskQuestionGetString(question);
	return atoi(ans.c_str());
}


double AskQuestionGetDouble(const std::string& question){
	std::string ans = AskQuestionGetString(question);
	return atof(ans.c_str());
}


void printMenu()
{
	std::cout << "help" << std::endl;
#if _TEST_
	std::cout << "run_test" << std::endl;
#endif
	std::cout << "img2feature" << std::endl;
	std::cout << "create_train_features" << std::endl;
	std::cout << "create_all_train_features" << std::endl;
	std::cout << "load" << std::endl;
	std::cout << "recog" << std::endl;
	std::cout << "recog_folder" << std::endl;
	std::cout << "recog_capture" << std::endl;
	std::cout << "exit" << std::endl;
}



int main(int argc, char * argv[])
{
#if _TEST_
	::testing::InitGoogleTest(&argc, argv);
#endif
	MainAPI CCNR;
	bool exitflag1 = false;
	std::string opt;

	while(!exitflag1){
		std::string opt = AskQuestionGetString("command: ");
		//std::string img_file = "../Data/CardImg/006.jpg";

		if(opt == "exit"){
			exitflag1 = true;
		}
		else if(opt == "help" || opt == "h"){
			printMenu();
		}
#if _TEST_
		else if(opt=="run_test"){
			RUN_ALL_TESTS();
		}
#endif
		else if(opt == "img2feature"){
			std::string image_file = AskQuestionGetString("Image File Name: ");
			std::string save_file = AskQuestionGetString("Save Feature File Name: ");
			CCNR.CreateFeature(image_file, save_file);
		}
		else if(opt == "create_train_features"){
			std::string load_dir = AskQuestionGetString("Load Directory Name: ");
			std::string save_file = AskQuestionGetString("Save Feature File Name: ");
			CCNR.CreateTrainingFeatures(load_dir, save_file);
		}
		else if(opt == "create_all_train_features"){
			std::string load_dir = AskQuestionGetString("Load Directory Name: ");
			std::string save_dir = AskQuestionGetString("Save Directory Name: ");
			CCNR.CreateTrainingAllFeatures(load_dir, save_dir);
		}
		else if(opt == "load"){
			std::string filename = AskQuestionGetString("Classifier File: ");
			CCNR.LoadClassifier(filename);
		}
		else if(opt == "recog"){
			std::string filename = AskQuestionGetString("Image File Name: ");
			std::string save_name = AskQuestionGetString("Save File Name: ");
			CCNR.Recognize(filename, save_name);
		}
		else if(opt == "recog_folder"){
			std::string dir_name = AskQuestionGetString("Directory Name: ");
			std::string save_dir = AskQuestionGetString("Save Directory: ");
			CCNR.RecognizeFolder(dir_name, save_dir);
		}
		else if (opt == "recog_capture") {
			CCNR.RecognizeVideoCapture();
		}
		else{
			std::cout << "Error: Wrong Command\n" << std::endl;
		}
	}

	return 0;
}