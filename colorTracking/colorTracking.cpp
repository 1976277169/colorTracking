// colorTracking.cpp : Defines the entry point for the console application.
//

//#include "stdio.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

using namespace cv;
using namespace std;

void getColors(Mat& image, Mat& r, Mat& g)
{
	return;
}

int main(int argc, char** argv)
{
	//Train
	Mat image;



	namedWindow("Display window", WINDOW_AUTOSIZE); // Create a window for display.
	imshow("Display window", image); // Show our image inside it.

	waitKey(0); // Wait for a keystroke in the window
	return 0;

}

