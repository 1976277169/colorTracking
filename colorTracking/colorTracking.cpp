// colorTracking.cpp : Defines the entry point for the console application.
//

//#include "stdio.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "utilities/image_utils.h"
#include <iostream>

using namespace cv;
using namespace std;

/*
* Function: colorSpaceMapping
* Usage:  colorSpaceMapping(iamge, r, g);
* ----------------------------------------
* Converts an image to a smaller colorspace and 
* returns the color space data
*/
void colorSpaceMapping(Mat& image, Mat& r, Mat& g)
{
	vector<Mat> rgb(3);
	split(image, rgb);

	Mat I = rgb[0] + rgb[1] + rgb[2];
	I.convertTo(I, CV_32F);
	//cout << I.depth() << endl;

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
/*
* Function: getRanges
* Usage:  getRanges(iamge, r_range, g_range);
* ----------------------------------------
* Converts an image to a smaller colorspace and returns the 
* range the image is in.
*/
void getRanges(Mat& image, double r_range[2], double g_range[2])
{
	Mat rRed, gRed;
	colorSpaceMapping(image, rRed, gRed);
	double gmax, gmin;
	double rmax, rmin;
	minMaxLoc(rRed, &rmin, &rmax);
	minMaxLoc(gRed, &gmin, &gmax);

	double adjustment1 = (rmax - rmin)*.4;
	double adjustment2 = (gmax - gmin)*.4;
	r_range[0] = rmin + adjustment1;
	r_range[1] = rmax - adjustment1;
	g_range[0] = gmin + adjustment2;
	g_range[1] = gmax - adjustment2;
}
/*
* Function: detectColor
* Usage:  detectColor(iamge, mask, ref);
* ----------------------------------------
* Converts the image into a smaller colorspace and creates
* a mask using <code>ref</code>
*/
void detectColor(Mat& image, Mat& mask, Mat& ref)
{
	//Mapping to new color space for ranges to detect on
	double r_range[2] = { 0, 0 };
	double g_range[2] = { 0, 0 };
	getRanges(ref, r_range, g_range);
	//cout << r_range[0] << r_range[1] << endl;

	Mat r, g;
	colorSpaceMapping(image, r, g);
	mask = ((g>g_range[0]) & (g<g_range[1]) &(r>r_range[0]) & (r<r_range[1]));
	return;
}
/*
* Function: mergeMask
* Usage:  mergeMask(mask);
* ----------------------------------------
* Uses mathematical morphology to merge detections
*/
void mergeMask(Mat& mask)
{
	int type = MORPH_ELLIPSE;
	int size = 2;
	Mat element = getStructuringElement(type,
		Size(2 * size + 1, 2 * size + 1),
		Point(size, size));
	int size2 = 4;
	Mat element2 = getStructuringElement(type,
		Size(2 * size2 + 1, 2 * size2 + 1),
		Point(size2, size2));
	morphologyEx(mask, mask, MORPH_DILATE, element2, Point(-1, -1), 1);
	//morphologyEx(mask, mask, MORPH_CLOSE, element, Point(-1, -1), 1);
	//morphologyEx(mask, mask, MORPH_OPEN, element, Point(-1, -1), 1);


	//morphologyEx(mask, mask, MORPH_DILATE, element2, Point(-1, -1), 1);
}
/*
* Function: getLargestBox
* Usage:  biggestBox = getLargestBox(boxes);
* ----------------------------------------
* Returns the biggest box in an array of Rects
*/
Rect getLargestBox(vector<Rect> boxes)
{
	RNG rng(12345);
	Rect biggestBox(0, 0, 0, 0);
	for (int i = 0; i < boxes.size(); i++)
	{
		int area = boxes[i].area();
		//cout << "area " <<area << endl;
		if (area > biggestBox.area() && area > 300)
			biggestBox = boxes[i];
	}
	//cout << "area " << biggestBox.area() << endl;
	return biggestBox;
}
/*
* Function: processVideo
* Usage:  processVideo(videoFilename);
* ----------------------------------------
* Processes the video <code>videoFilename</code>
*/
void processVideo(char* videoFilename)
{
	//Read and display
	Mat red = imread("training/red.png", CV_LOAD_IMAGE_COLOR);
	Mat green = imread("training/green.png", CV_LOAD_IMAGE_COLOR);
	Mat blue = imread("training/blue.png", CV_LOAD_IMAGE_COLOR);
	Mat orange = imread("training/orange.png", CV_LOAD_IMAGE_COLOR);
	GaussianBlur(red, red, Size(3, 3), 1, 1);
	GaussianBlur(green, green, Size(3, 3), 1, 1);
	GaussianBlur(orange, orange, Size(3, 3), 1, 1);
	GaussianBlur(blue, blue, Size(3, 3), 1, 1);

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
		GaussianBlur(orig,orig,Size(5,5),2,2);

		//Detect Red cup
		Mat redMask;
		detectColor(orig, redMask, red);

		Mat orangeMask;
		detectColor(orig, orangeMask, orange);

		Mat greenMask;
		detectColor(orig, greenMask, green);

		Mat blueMask;
		detectColor(orig, blueMask, blue);

		//Clean up the mask for cups
		mergeMask(greenMask);
		mergeMask(redMask);
		mergeMask(blueMask);
		
		//Clean up the mask for the ball
		int size2 = 2;
		Mat element2 = getStructuringElement(MORPH_ELLIPSE,
			Size(2 , 2 ));
		Mat element3 = getStructuringElement(MORPH_ELLIPSE,
			Size(7, 7));
		morphologyEx(orangeMask, orangeMask, MORPH_OPEN, element2, Point(-1, -1), 1);
		morphologyEx(orangeMask, orangeMask, MORPH_DILATE, element3, Point(-1, -1), 3);
	

		//Add bounding boxes
		vector<Rect> boxesO = addBoundingBox(orig, orangeMask, false);
		vector<Rect> boxesR = addBoundingBox(orig, redMask, false);
		vector<Rect> boxesG = addBoundingBox(orig, greenMask, false);
		vector<Rect> boxesB = addBoundingBox(orig, blueMask, false);

		//Keep large box only and draw
		Rect redCup = getLargestBox(boxesR);
		Rect ball = getLargestBox(boxesO);
		Rect greenCup = getLargestBox(boxesG);
		Rect blueCup = getLargestBox(boxesB);

		if (ball != Rect(0, 0, 0, 0))
		{
			lastBallLoc = ball;
		}
		//cout << lastBallLoc << endl;

		//NOTE: Jumps from Red to Green for some reason: because of a false detection on the ball!
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
			//cout << "The red cup" << endl;
		}
		else if (lastBallLoc.x < (greenCup.x + greenCup.width) && (lastBallLoc.x + lastBallLoc.width) > greenCup.x &&
			lastBallLoc.y < (greenCup.y + greenCup.height) && (lastBallLoc.y + lastBallLoc.height) > greenCup.y &&
			ballInCup == 0)
		{
			rectangle(orig, greenCup.tl(), greenCup.br(), color, 2, 8, 0);
			lastBallLoc = greenCup;
			ballInCup = 2;
			//cout << "The green cup" << endl;
		}
		else if (lastBallLoc.x < (blueCup.x + blueCup.width) && (lastBallLoc.x + lastBallLoc.width) > blueCup.x &&
			lastBallLoc.y < (blueCup.y + blueCup.height) && (lastBallLoc.y + lastBallLoc.height) > blueCup.y &&
			ballInCup == 0)
		{
			rectangle(orig, blueCup.tl(), blueCup.br(), color, 2, 8, 0);
			lastBallLoc = blueCup;
			ballInCup = 3;
			//cout << "The green cup" << endl;
		}
		//Check for ball outside
		else if (lastBallLoc.x < (ball.x + ball.width) && (lastBallLoc.x + lastBallLoc.width) > ball.x &&
			lastBallLoc.y < (ball.y + ball.height) && (lastBallLoc.y + lastBallLoc.height) > ball.y)
		{
			ballInCup = 0;
			rectangle(orig, ball.tl(), ball.br(), color, 2, 8, 0);
			lastBallLoc = ball;
			//cout << "The ball" << endl;
		}

		if (ballInCup == 1)
			rectangle(orig, redCup.tl(), redCup.br(), color, 2, 8, 0);
		else if (ballInCup == 2)
			rectangle(orig, greenCup.tl(), greenCup.br(), color, 2, 8, 0);
		else if (ballInCup == 3)
			rectangle(orig, blueCup.tl(), blueCup.br(), color, 2, 8, 0);


		//Display
		imshow("Frame", orig);
		imshow("RED", redMask);
		imshow("Orange", orangeMask);
		imshow("green", greenMask);
		imshow("blue", blueMask);
	keyboard = waitKey(30);
	}

	//delete capture object
	capture.release();
}

int main(int argc, char** argv)
{

	//Detect red cup
	processVideo("3-cups/3-3.avi");


	waitKey(0); // Wait for a keystroke in the window
	return 0;

}

