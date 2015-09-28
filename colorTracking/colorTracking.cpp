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

	double adjustment1 = (rmax - rmin)*.30;
	double adjustment2 = (gmax - gmin)*.30;
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

void mergeMask(Mat& mask)
{
	int type = MORPH_RECT;
	int size = 2;
	Mat element = getStructuringElement(type,
		Size(2 * size + 1, 2 * size + 1),
		Point(size, size));
	morphologyEx(mask, mask, MORPH_OPEN, element, Point(-1, -1), 1);
	morphologyEx(mask, mask, MORPH_CLOSE, element, Point(-1, -1), 3);

	morphologyEx(mask, mask, MORPH_DILATE, element, Point(-1, -1), 3);
}

Rect getLargestBox(vector<Rect> boxes)
{
	RNG rng(12345);
	Rect biggestBox(0, 0, 0, 0);
	for (int i = 0; i < boxes.size(); i++)
	{
		if (boxes[i].area() > biggestBox.area())
			biggestBox = boxes[i];
	}

	return biggestBox;
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
	Rect lastBallLoc(0, 0, 0, 0);
	int ballInCup = 0;
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

		//Enhance the frame
		GaussianBlur(orig,orig,Size(3,3),1,1);

		//Detect Red cup
		Mat redMask;
		detectColor(orig, redMask, red);

		Mat orangeMask;
		detectColor(orig, orangeMask, orange);

		Mat greenMask;
		detectColor(orig, greenMask, green);

		//Clean up the mask
		mergeMask(orangeMask);
		mergeMask(greenMask);
		mergeMask(redMask);

		//Add bounding boxes
		vector<Rect> boxesO = addBoundingBox(orig, orangeMask, false);
		vector<Rect> boxesR = addBoundingBox(orig, redMask, false);
		vector<Rect> boxesG = addBoundingBox(orig, greenMask, false);

		//Keep large box only and draw
		Rect redCup = getLargestBox(boxesR);
		Rect ball = getLargestBox(boxesO);
		Rect greenCup = getLargestBox(boxesG);

		if (ball != Rect(0, 0, 0, 0))
		{
			lastBallLoc = ball;
		}
		cout << lastBallLoc << endl;

		//NOTE: Jumps from Red to Green for some reason
		//Check if ball is in a cup
		RNG rng(12345);
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		if (lastBallLoc.x < (redCup.x + redCup.width) && (lastBallLoc.x + lastBallLoc.width) > redCup.x &&
			lastBallLoc.y < (redCup.y + redCup.height) && (lastBallLoc.y + lastBallLoc.height) > redCup.y &&
			ballInCup == 0)
		{
			rectangle(orig, redCup.tl(), redCup.br(), color, 2, 8, 0);
			lastBallLoc = redCup;
			ballInCup = 1;
			cout << "The red cup" << endl;
		}
		else if (lastBallLoc.x < (greenCup.x + greenCup.width) && (lastBallLoc.x + lastBallLoc.width) > greenCup.x &&
			lastBallLoc.y < (greenCup.y + greenCup.height) && (lastBallLoc.y + lastBallLoc.height) > greenCup.y &&
			ballInCup == 0)
		{
			rectangle(orig, greenCup.tl(), greenCup.br(), color, 2, 8, 0);
			lastBallLoc = greenCup;
			ballInCup = 2;
			cout << "The green cup" << endl;
		}
		//Check for ball outside
		else if (lastBallLoc.x < (ball.x + ball.width) && (lastBallLoc.x + lastBallLoc.width) > ball.x &&
			lastBallLoc.y < (ball.y + ball.height) && (lastBallLoc.y + lastBallLoc.height) > ball.y)
		{
			ballInCup = 0;
			rectangle(orig, ball.tl(), ball.br(), color, 2, 8, 0);
			lastBallLoc = ball;
			cout << "The ball" << endl;
		}

		if (ballInCup == 1)
			rectangle(orig, redCup.tl(), redCup.br(), color, 2, 8, 0);
		else if (ballInCup == 2)
			rectangle(orig, greenCup.tl(), greenCup.br(), color, 2, 8, 0);



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

