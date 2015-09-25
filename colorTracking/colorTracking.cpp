// colorTracking.cpp : Defines the entry point for the console application.
//

//#include "stdio.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "utilities/image_utils.h"
#include <iostream>

using namespace cv;
using namespace std;

void colorSpaceMapping(Mat& image, Mat& r, Mat& g)
{
	vector<Mat> rgb(3);
	split(image, rgb);

	Mat I = rgb[0] + rgb[1] + rgb[2];
	I.convertTo(I, CV_32F);
	cout << I.depth() << endl;

	for (int i = 0; i < I.rows; i++)
	{
		float* Mi = I.ptr<float>(i);
		for (int j = 0; j < I.cols; j++)
		{
			if (Mi[j] == 0.0)
				Mi[j] = 100000.0;
		}
	}
	rgb[0].convertTo(rgb[0], CV_32F);
	rgb[1].convertTo(rgb[1], CV_32F);
	r = rgb[0] / I;
	g = rgb[1] / I;

	return;
}

void getRanges(Mat& image, double r_range[2], double g_range[2])
{
	Mat rRed, gRed;
	colorSpaceMapping(image, rRed, gRed);
	double gmax, gmin;
	double rmax, rmin;
	minMaxLoc(rRed, &rmin, &rmax);
	minMaxLoc(gRed, &gmin, &gmax);

	double adjustment1 = (rmax - rmin)*.25;
	double adjustment2 = (gmax - gmin)*.25;
	r_range[0] = rmin + adjustment1;
	r_range[1] = rmax - adjustment1;
	g_range[0] = gmin + adjustment2;
	g_range[1] = gmax - adjustment2;
}

void detectColor(Mat& image, Mat& mask, Mat& ref)
{
	//Mapping to new color space for ranges to detect on
	double r_range[2] = { 0, 0 };
	double g_range[2] = { 0, 0 };
	getRanges(ref, r_range, g_range);
	cout << r_range[0] << r_range[1] << endl;

	Mat r, g;
	colorSpaceMapping(image, r, g);
	mask = ((g>g_range[0]) & (g<g_range[1]) &(r>r_range[0]) & (r<r_range[1]));
	return;
}

void processVideo(char* videoFilename)
{
	//Read and display
	Mat red = imread("training/red.png", CV_LOAD_IMAGE_COLOR);
	cout << red.channels() << endl;
	displayImg(red, "RED");
	Mat green = imread("training/green.png", CV_LOAD_IMAGE_COLOR);
	displayImg(green, "GREEN");
	Mat blue = imread("training/blue.png", CV_LOAD_IMAGE_COLOR);
	displayImg(blue, "BLUE");
	Mat orange = imread("training/orange.png", CV_LOAD_IMAGE_COLOR);
	displayImg(orange, "ORANGE");

	//Getting the VideoCapture object
	VideoCapture capture(videoFilename);
	if (!capture.isOpened())
	{
		cerr << "Unable to open video file: " << videoFilename << endl;
		exit(EXIT_FAILURE);
	}

	//read input data and process
	int keyboard = 0;
	Mat orig;
	
	while ((char)keyboard != 'q' && (char)keyboard != 27)
	{
		if (keyboard == 'p')
		{
			waitKey(0);
		}
		//Reading the frame
		if (!capture.read(orig))
		{
			cerr << "Unable to read next frame." << endl;
			cerr << "Exiting..." << endl;
			exit(EXIT_FAILURE);
		}


		//Detect Red cup
		Mat redMask;
		detectColor(orig, redMask, red);

		Mat orangeMask;
		detectColor(orig, orangeMask, orange);

		Mat greenMask;
		detectColor(orig, greenMask, green);

		//Display
		imshow("Frame", orig);
		imshow("RED", redMask);
		imshow("Orange", orangeMask);
		imshow("green", greenMask);
	keyboard = waitKey(30);
	}

	//delete capture object
	capture.release();
}

int main(int argc, char** argv)
{

	//Detect red cup
	processVideo("2-cups/2-1.avi");


	waitKey(0); // Wait for a keystroke in the window
	return 0;

}

