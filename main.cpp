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
#include <boost/program_options.hpp>
#include <boost/filesystem/operations.hpp>
#include "MainAPI.h"
#include "util.h"

using namespace boost::program_options;
using namespace boost::filesystem;

///////////// Command Line Execution Mode ////////////////////
void print_help(const options_description& opt)
{
	std::cout << "CreditNumberRecognizer input [option]" << std::endl;
	std::cout << opt << std::endl;
}


bool parse_command(int argc, char* argv[], std::string& input,
	std::string& model_file, std::string& output, bool& use_camera)
{
	// Setting of option arguments
	options_description opt("option");
	opt.add_options()
		("input", value<std::string>(), "Input image file or directory path")
		("help,h", "print help")
		("model,m", value<std::string>()->default_value("CreditModel.txt"), "Trained model file path")
		("output,o", value<std::string>()->default_value(std::string()), "Generate output image or directory path")
		("camera,c", "Use web camera input");

	// Arguments
	positional_options_description p;
	p.add("input", 1);

	variables_map argmap;
	try {
		// Obtain command arguments
		// parse command line
		parsed_options parsed = command_line_parser(argc, argv).options(opt).allow_unregistered().positional(p).run();
		store(parsed, argmap);	// store parsed command options into argmap
		notify(argmap);

		// Print Help
		if (argmap.count("help")) {
			print_help(opt);
			return false;
		}

		use_camera = !argmap["camera"].empty();
		input = argmap["input"].as<std::string>();
		output = argmap["output"].as<std::string>();
		model_file = argmap["model"].as<std::string>();

		////// verify command arguments ///////
		if (!use_camera) {
			if(input.empty()) {
				throw std::invalid_argument("no arugument of input");
			}
			else if (hasImageExtention(input)) {
				if (!output.empty() && !hasImageExtention(output)) {
					throw std::invalid_argument("\"output\" must be image file path.");
				}
			}
			else if (is_directory(path(input))) {
				if (!output.empty() && !is_directory(path(output))) {
					throw std::invalid_argument("\"output\" must be directory path.");
				}
			}
			else {
				throw std::invalid_argument("wrong input format");
			}
		}		
	}
	catch (const std::exception& e)
	{
		std::cout << std::endl << e.what() << std::endl;
		print_help(opt);
		return false;
	}

	return true;
}



int CommandLineExe(int argc, char * argv[])
{
	MainAPI CCNR;
	std::string conf_file, input, output, model_file;
	bool use_camera;
	if (!parse_command(argc, argv, input, model_file, output, use_camera))
		return -1;

	try {
		if (!CCNR.LoadClassifier(model_file))
			return -1;

		if (use_camera) {
			CCNR.RecognizeVideoCapture();
		}
		else if (hasImageExtention(input)) {
			CCNR.Recognize(input, output, false);
		}
		else if (is_directory(path(input))) {
			CCNR.RecognizeFolder(input, output);
		}
		return 0;
	}
	catch (std::exception& e) {
		std::cout << std::endl << e.what() << std::endl;
		return -1;
	}
}


///////////////// Interactive Mode //////////////////
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

	if (argc > 1) {
		return CommandLineExe(argc, argv);
	}

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