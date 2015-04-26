CreditRecognizer

2015/04/26 First Version

This program recognize numbers of credit card from an image.
Please take a picture of a credit card to use this program.
An image should be taken with little background and skew.

Windows execution file (CreditRecognizer.exe) was prepared, so you can use this program soon after expand CreditNumberRecognizer.zip.
You need to install Visual Studio 2013 runtime library before you run it.

I developed this program with the environment below:
- Windows8.1
- Visual Stuido2013
- OpenCV 2.4.10
- Boost 1.55

I avoided to use windows-dependent code, thus I expect that this program can be exported to other environments.


How to use:
1. run CreditRecognizer.exe
2. at "command:" prompt, enter "load"
3. at "Classifier File:" prompt, enter file path to "CreditModel.txt"
4. at "command:" prompt, enter "recog"
5. at "Image File Name:" prompt, enter file path to the image of credit card
6. at "Save File Name:" prompt, enter output image file name. Then you can find ther result in that image file.
7. You can finish this program with "exit"

In addition:
- you can process several images at once with "recog_folder" command.
- at "command:" prompt, enter "h" to see all commands.

Notice:
- Error handling was not implemented in this version.

Hope to doÅF
- Detect and correct skew of image
- Imporve recognition of character.

Vision & IT Lab
MINAGAWA Takuya (Freelance Engineer)
