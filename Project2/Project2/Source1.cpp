#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
using namespace std;
using namespace cv;
/// Global variables
void showPicture(int, void*);
char* trackbar_type = "Type: \n 0: Binary \n 1: Binary Inverted \n 2: Truncate \n 3: To Zero \n 4: To Zero Inverted";
char* window_name = "Threshold Demo";
//int threshold_value = 255;//zero image
int threshold_value = 20;//binary image
int threshold_type = 0;
int canny = 0;//edge algorithm
int picture_num = 0;//the pictures
int const max_value = 255;
int const max_type = 4;
int const max_picture = 4;
int const max_canny = 100;
int const max_BINARY_value = 255;
int MAX_KERNEL_LENGTH = 15;
Mat src, src_gray, dst[4][2];
//{[0]:  median filtering, [1]:threshold algorithm, [2]:edge detection, [3]:ellipse detection}
//{[0]은 단계별 결과 데이터, [1]은 [0]을 다음 단계의 입력값으로 넣게 되어 손상된 데이터}
char* trackbar_value = "Value";
/// Function headers
static void showPicture(int, void*)
{	//cout << picture_num << endl;
	imshow(window_name, dst[picture_num][0]);
}
void ellipseDetect()
{
	// Load image
	Mat img = dst[3][1];
	// Convert to grayscale. Binarize if needed
	Mat1b bin = img;
	//cvtColor(img, bin, COLOR_BGR2GRAY);

	// Find contours
	vector<vector<Point>> contours;
	findContours(bin.clone(), contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
	/*
	if(contours.size() != 0)
		drawContours(img, contours, 0, Scalar(255, 0, 0), 2);
	dst[4][0] = img.clone();
	showPicture(0, 0);
	return;
	*/
	// For each contour
	for (int i = 0; i < contours.size(); ++i)
	{
		// Find ellipse
		RotatedRect ell;
		if (contours[i].size()>4)
			 ell= fitEllipse(contours[i]);
		else
			continue;
		// Draw contour
		Mat1b maskContour(img.rows, img.cols, uchar(0));
		drawContours(maskContour, contours, i, Scalar(255, 0, 0), 2);

		// Draw ellips
		Mat1b maskEllipse(img.rows, img.cols, uchar(0));
		ellipse(maskEllipse, ell, Scalar(255), 2);

		// Intersect
		Mat1b intersection = maskContour & maskEllipse;

		// Count amount of intersection
		float cnz = countNonZero(intersection);
		// Count number of pixels in the drawn contour
		float n = countNonZero(maskContour);
		// Compute your measure
		float measure = cnz / n;
		// Draw, color coded: good -> gree, bad -> red
		ellipse(img, ell, Scalar(0, measure * 255, 255 - measure * 255), 3);
		circle(img, ell.center, 5, Scalar(255, 0, 0));
		cout << ell.center.x << ":" << ell.center.y << endl;
	}
	dst[4][0]= img.clone();
}
void Canny_demo(int, void*) {
	Canny(dst[2][1].clone(), dst[3][0], canny, canny * 3, 3);
}
void processing(Mat parm)
{


	src = parm.clone();
	dst[0][0] = src.clone();
	dst[0][1] = src.clone();
	for (int i = 3; i < MAX_KERNEL_LENGTH; i = i + 2)
	{
		medianBlur(src, dst[1][0], i);
	}/// Convert the image to Gray
	dst[1][1] = dst[1][0].clone();
	threshold(dst[1][1], dst[2][0], threshold_value, max_BINARY_value, threshold_type);
	dst[2][1] = dst[2][0].clone();

	//bitwise_not(dst[2][1], dst[3][0]);
	Canny_demo(0, 0);
	dst[3][1] = dst[3][0].clone();

	ellipseDetect();
	createTrackbar(trackbar_type,
		window_name, &picture_num,
		max_picture, showPicture);
	//showPicture(0, 0);
	/// Wait until user finishes program
}

int main(int argc, char** argv)
{
	/// Load an image
	Mat frame; //= imread("capture.jpg", 1);
	VideoCapture capture("eye1.mp4");
	namedWindow(window_name, CV_WINDOW_AUTOSIZE);
	while (true)
	{
		capture >> frame;
		if (frame.empty())
			break;
		processing(frame);
		showPicture(0, 0);
		waitKey(27); // waits to display frame
	}
	while (true)
	{
		int c;
		c = waitKey(20);
		if ((char)c == 27)
		{
			break;
		}
	}
}

