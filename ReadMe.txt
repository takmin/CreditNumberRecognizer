CreditNumberRecognizer

2018/11/14 Version 1.2
- Able to execute directly from command line
- Recognize from camera capture
- Add CMakeFile.txt
- Use OpenCV version 3.x

2015/04/26 First Version

-------------------------------
This program recognize numbers of credit card from an image.
Please take a picture of a credit card to use this program.
An image should be taken with little background and skew.

Windows execution file (CreditRecognizer.exe) was prepared, so you can use this program soon after expand CreditNumberRecognizer.zip.
You need to install Visual Studio 2013 runtime library before you run it.

I developed this program with the environment below:
- Windows10 or CentOS7.4
- Visual Stuido2017
- OpenCV 3.4.3
- Boost 1.68

I avoided to use windows-dependent code, thus I expect that this program can be exported to other environments.

How to build (Linux)ÅF
Before install this program, you MUST install OpenCV(https://opencv.org/) and Boost(https://www.boost.org/).

$ git clone https://github.com/takmin/CreditNumberRecognizer.git
$ cd CreditNumberRecognizer
$ mkdir build
$ cd build
$ cmake ..
$ make
Then you can find an executable file "CreditNumberRecognizer".

How to use (Interaction Mode):
1. run CreditNumberRecognizer
2. at "command:" prompt, enter "load"
3. at "Classifier File:" prompt, enter file path to "CreditModel.txt"
4. at "command:" prompt, enter "recog"
5. at "Image File Name:" prompt, enter file path to the image of credit card
6. at "Save File Name:" prompt, enter output image file name. Then you can find ther result in that image file.
7. You can finish this program with "exit"


In addition:
- you can process several images at once with "recog_folder" command.
- you can use camera capture as input image with "recog_capture" command.
- at "command:" prompt, enter "h" to see all commands.


Excecute directly from command line:
You can avoid interaction and run this function directly with some arguments.
----
CreditNumberRecognizer input [option]
option:
  -i [--input ] arg                     Input image file or directory path
  -h [ --help ]                         print help
  -m [ --model ] arg (=CreditModel.txt) Trained model file path
  -o [ --output ] arg                   Generate output image or directory path
  -c [ --camera ]                       Use web camera input
----


Notice:
- Error handling was not implemented in this version.

Hope to doÅF
- Detect and correct skew of image
- Imporve recognition of character.

Vision & IT Lab
MINAGAWA Takuya (Freelance Engineer)
