#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <cmath>
using namespace std;
using namespace cv;
/// Global variables

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


Mat src, src_gray, dst[5][2];
vector<Point> points,realpoints;
vector<RotatedRect> rectangles,realrectangles;
Point realpoint;
RotatedRect realrect;
//{[0]:  median filtering, [1]:threshold algorithm, [2]:edge detection, [3]:ellipse detection}
//{[0]은 단계별 결과 데이터, [1]은 [0]을 다음 단계의 입력값으로 넣게 되어 손상된 데이터}
char* trackbar_value = "Value";
/// Function headers


#include <iostream>
#include "opencv2/opencv.hpp"
#include <stdio.h>

using namespace std;
using namespace cv;

Mat frame, img, ROI;
Rect cropRect(0, 0, 0, 0);
Point P1(0, 0);
Point P2(0, 0);

const char* winName = "Crop Image";
bool clicked = false;
int i = 0;
char imgName[15];

void checkBoundary() {
	//check croping rectangle exceed image boundary
	if (cropRect.width>img.cols - cropRect.x)
		cropRect.width = img.cols - cropRect.x;

	if (cropRect.height>img.rows - cropRect.y)
		cropRect.height = img.rows - cropRect.y;

	if (cropRect.x<0)
		cropRect.x = 0;

	if (cropRect.y<0)
		cropRect.height = 0;
}
void showImage() {
	img = frame.clone();
	//checkBoundary();
	//if (cropRect.width>0 && cropRect.height>0) {
	//	//ROI = src(cropRect);
	//	//imshow("cropped", ROI);
	//}
	rectangle(img, cropRect, Scalar(0, 255, 0), 3, 8, 0);
	imshow(winName, img);
	waitKey(27);
	//잘린 사진이 나오는 곳.
}
void onMouse(int event, int x, int y, int f, void*) {
	switch (event) {
	case  CV_EVENT_LBUTTONDOWN:
		clicked = true;
		P1.x = x;
		P1.y = y;
		P2.x = x;
		P2.y = y;
		//destroyWindow("cropped");
		break;
	case  CV_EVENT_LBUTTONUP:
		P2.x = x;
		P2.y = y;
		clicked = false;
		break;
	case  CV_EVENT_MOUSEMOVE:
		if (clicked) {
			P2.x = x;
			P2.y = y;
		}
		break;
	default:   break;
	}
	if (clicked) {
		if (P1.x>P2.x) {
			cropRect.x = P2.x;
			cropRect.width = P1.x - P2.x;
		}
		else {
			cropRect.x = P1.x;
			cropRect.width = P2.x - P1.x;
		}

		if (P1.y>P2.y) {
			cropRect.y = P2.y;
			cropRect.height = P1.y - P2.y;
		}
		else {
			cropRect.y = P1.y;
			cropRect.height = P2.y - P1.y;
		}

	}
	//showImage();
}
void showPicture(int, void*)
{	
	// cout << picture_num << endl;
	rectangle(dst[picture_num][0], cropRect, Scalar(0, 255, 0), 2, 8, 0);
	imshow(window_name, dst[picture_num][0]);
	waitKey(27);

}
void orgnize_point_center()//무조건 중앙의 점을 눈동자의 점이라고 인식하는 방법
{
	int width = 320/2;
	int height = 240/2;
	int nexx, prex = width;
	int nexy, prey = height;
	int reali=-1;
	for (int i = 0; i < points.size(); i++)
	{
		nexx = abs(width - points[i].x);
		nexy = abs(width - points[i].y);
		if (nexx < prex && nexy<prey) {
			prex = nexx;
			prey = nexy;
			reali = i;
		}
		/*
		if (nexy < prey) {
			prey = nexy;
			reali = i;
		}*/
	}
	if (reali == -1)
	{
		realpoint = realpoints[realpoints.size() - 1];
		realrect = realrectangles[realrectangles.size() - 1];
	}
	else{
		realpoint = points[reali];
		realrect = rectangles[reali];
		realpoints.push_back(realpoint);
		realrectangles.push_back(realrect);
	}
}
//void orgnize_point_cali()//기준이 되는 동영상의 데이터를 가지고 눈동자 점을 찾는 방법
//{
//	int nexx, prex;
//	int nexy, prey;
//	int reali, realj= 0;
//	for (int i = 0;i < realpoints.size();i++)
//	{
//		for (int j = 0;j < points.size();i++)
//		{
//			prex = realpoints[i].x - points[j].x;
//			prey = realpoints[i].y - points[j].y;
//			if (prex < nexx)
//			{
//
//			}
//			if (prey < nexy)
//			{
//
//			}
//		}
//	}
//}
void orgnize_point_rect()//내부 정사각형 안에서 눈동자 점을 찾는 방법
{
	int reali;
	int prex = cropRect.x;
	int nexx = prex+cropRect.width;
	int prey = cropRect.y;
	int nexy = prey+cropRect.height;
	int diffpx,diffpy,diffnx, diffny;
	for (int i = 0; i < points.size(); i++)
	{
		diffpx = points[i].x - prex;
		diffpy = points[i].y - prey;
		diffnx = nexx - points[i].x;
		diffny = nexy - points[i].y;
		if (diffpx > 0 && diffpy > 0 &&diffnx > 0 &&diffny > 0){
			reali = i;
			break;
		}
	}
	realpoint = points[reali];
}
void ellipseDetect()
{
	// Load image
	Mat mat = dst[3][1];
	// Convert to grayscale. Binarize if needed
	Mat1b bin = mat;
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
		Mat1b maskContour(mat.rows, mat.cols, uchar(0));
		cv::drawContours(maskContour, contours, i, Scalar(255, 0, 0), 2);

		// Draw ellipse
		Mat1b maskEllipse(mat.rows, mat.cols, uchar(0));
		cv::ellipse(maskEllipse, ell, Scalar(255), 2);

		// Intersect
		Mat1b intersection = maskContour & maskEllipse;

		// Count amount of intersection
		float cnz = countNonZero(intersection);
		// Count number of pixels in the drawn contour
		float n = countNonZero(maskContour);
		// Compute your measure
		float measure = cnz / n;
		// Draw, color coded: good -> green, bad -> red
		if(measure >0.5)
		{
			//cv::ellipse(img, ell, Scalar(0, measure * 255, 255 - measure * 255), 3);
			rectangles.push_back(ell);
			points.push_back(ell.center);
		}
	}
	orgnize_point_rect();
	cv::circle(mat, realpoint, 5, Scalar(255, 0, 0));
	//std::cout <<points.size()<<":"<< realpoint.x << ":" << realpoint.y << endl;
	//std::cout << points.size() << ":" << rectangles.size() << endl;
	cout << cropRect.x;
	cout << ":";
	cout << cropRect.y;
	cout << ":";
	cout << cropRect.width;
	cout << ":";
	cout << cropRect.height;
	cout << "\n" << endl;
	points.clear();
	rectangles.clear();
	dst[4][0] = mat.clone();
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
	cv::ellipse(dst[0][0], realrect, Scalar(0, 0, 255));
	cv::circle(dst[0][0], realpoint, realrect.boundingRect2f().width>realrect.boundingRect2f().height? realrect.boundingRect2f().height /2: realrect.boundingRect2f().width/2, Scalar(0, 255, 0));

	createTrackbar(trackbar_type,
		window_name, &picture_num,
		max_picture, showPicture);
	//showPicture(0, 0);
	/// Wait until user finishes program
}
//int main(int argc, char** argv)
//{
//	/// Load an image
//	Mat frame = imread("capture.jpg", 1);
//	//VideoCapture capture("eye1.mp4");
//	namedWindow(window_name, CV_WINDOW_AUTOSIZE);
//	//while (true)
//	//{
//		//capture >> frame;
//		//if (frame.empty())
//			//break;
//		processing(frame);
//		showPicture(0, 0);
//		//waitKey(27); // waits to display frame
//	//}
//	while (true)
//	{
//		int c;
//		c = waitKey(20);
//		if ((char)c == 27)
//		{
//			break;
//		}
//	}
//}
int pureMain(int argc, char** argv)
{
	/// Load an image
	 //= imread("capture.jpg", 1);
	VideoCapture capture("eye0.mp4");
	namedWindow(window_name, CV_WINDOW_AUTOSIZE);
	while (true)
	{
		capture >> frame;
		if (frame.empty())
			break;
		processing(frame);
		showPicture(0, 0);
		waitKey(27); // waits e display frame
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

int coarseMergingMain()
{
	namedWindow(winName, CV_WINDOW_AUTOSIZE);
	frame = imread("capture.JPG", 1);
	//VideoCapture capture("eye0.mp4");
	//namedWindow(window_name, CV_WINDOW_AUTOSIZE);
	//while (true)
	//{
	//	capture >> frame;
	//	if (frame.empty())
	//		break;
	//	processing(frame);
	//	showPicture(0, 0);
	//}
	setMouseCallback(winName, onMouse, NULL);
	imshow(winName, frame);
	while (1) {
		VideoCapture capture("eye0.mp4");
		//namedWindow(window_name, CV_WINDOW_AUTOSIZE);
		while (true)
		{
			capture >> frame;
			if (frame.empty())
				break;
			processing(frame);
			showPicture(0, 0);
		}
		char c = waitKey();
		if (c == 's'&&ROI.data) {
			printf(imgName, "%d.jpg", i++);
			imwrite(imgName, ROI);
			cout << "  Saved " << imgName << endl;
		}
		if (c == '6') cropRect.x++;
		if (c == '4') cropRect.x--;
		if (c == '8') cropRect.y--;
		if (c == '2') cropRect.y++;

		if (c == 'w') { cropRect.y--; cropRect.height++; }
		if (c == 'd') cropRect.width++;
		if (c == 'x') cropRect.height++;
		if (c == 'a') { cropRect.x--; cropRect.width++; }

		if (c == 't') { cropRect.y++; cropRect.height--; }
		if (c == 'h') cropRect.width--;
		if (c == 'b') cropRect.height--;
		if (c == 'f') { cropRect.x++; cropRect.width--; }

		if (c == 27) break;
		if (c == 'r') { cropRect.x = 0; cropRect.y = 0; cropRect.width = 0; cropRect.height = 0; }
		showImage();
	}
	return 0;
}
int main()
{
	namedWindow(winName, CV_WINDOW_AUTOSIZE);
	VideoCapture capture("eye0.mp4");
	capture >> frame;
	//frame = imread("capture.JPG", 1);
	//VideoCapture capture("eye0.mp4");
	//namedWindow(window_name, CV_WINDOW_AUTOSIZE);
	//while (true)
	//{
	//	capture >> frame;
	//	if (frame.empty())
	//		break;
	//	processing(frame);
	//	showPicture(0, 0);
	//}
	setMouseCallback(winName, onMouse, NULL);
	checkBoundary();
	while(1){
		showImage();
		if (!clicked && (cropRect.width > 0 && cropRect.height > 0))
			break;
	}
	//imshow(winName, frame);
	cvDestroyWindow(winName);
	while (1) {
		//namedWindow(window_name, CV_WINDOW_AUTOSIZE);
		while (true)
		{
			capture >> frame;
			if (frame.empty())
				break;
			processing(frame);
			showPicture(0, 0);
		}
		char c = waitKey();
		if (c == 's'&&ROI.data) {
			printf(imgName, "%d.jpg", i++);
			imwrite(imgName, ROI);
			cout << "  Saved " << imgName << endl;
		}
		if (c == '6') cropRect.x++;
		if (c == '4') cropRect.x--;
		if (c == '8') cropRect.y--;
		if (c == '2') cropRect.y++;

		if (c == 'w') { cropRect.y--; cropRect.height++; }
		if (c == 'd') cropRect.width++;
		if (c == 'x') cropRect.height++;
		if (c == 'a') { cropRect.x--; cropRect.width++; }

		if (c == 't') { cropRect.y++; cropRect.height--; }
		if (c == 'h') cropRect.width--;
		if (c == 'b') cropRect.height--;
		if (c == 'f') { cropRect.x++; cropRect.width--; }

		if (c == 27) break;
		if (c == 'r') { cropRect.x = 0; cropRect.y = 0; cropRect.width = 0; cropRect.height = 0; }
		//showImage();
	}
	return 0;
}
