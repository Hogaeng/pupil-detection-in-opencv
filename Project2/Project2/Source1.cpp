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
//int threshold_value = 255;//zero image
int threshold_value = 20;//binary image
int canny = 0;//edge algorithm
int picture_num = 0;//the pictures
int picture_num2 = 0;//the pictures

int const max_value = 255;
int const max_type = 4;
int const max_picture = 4;
int const max_canny = 100;
int const max_BINARY_value = 255;
int MAX_KERNEL_LENGTH = 15;

//{[0]:  median filtering, [1]:threshold algorithm, [2]:edge detection, [3]:ellipse detection}
//{[0]은 단계별 결과 데이터, [1]은 [0]을 다음 단계의 입력값으로 넣게 되어 손상된 데이터}
char* trackbar_value = "Value";
/// Function headers
Rect cropRect(0, 0, 0, 0);
Point P1(0, 0);
Point P2(0, 0);
Rect cropRect2(0, 0, 0, 0);
Point P3(0, 0);
Point P4(0, 0);

bool clicked = false;
bool clicked2 = false;

char* winName = "eye0";
char* winName2 = "eye1";
class ImageOperate
{
private:
	Mat src, src_gray, dst[5][2];
	vector<Point> points, realpoints;
	vector<RotatedRect> rectangles, realrectangles;
	Point realpoint;
	RotatedRect realrect;
	char * winName;
	void orgnize_point_center()//무조건 중앙의 점을 눈동자의 점이라고 인식하는 방법
	{
		int width = 320 / 2;
		int height = 240 / 2;
		int nexx, prex = width;
		int nexy, prey = height;
		int reali = -1;
		for (int i = 0; i < points.size(); i++)
		{
			nexx = abs(width - points[i].x);
			nexy = abs(width - points[i].y);
			if (nexx < prex && nexy < prey) {
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

			realpoints[realpoints.size() - 1];
			realrect = realrectangles[realrectangles.size() - 1];
		}
		else {
			realpoint = points[reali];
			realrect = rectangles[reali];
			realpoints.push_back(realpoint);
			realrectangles.push_back(realrect);
		}
	}
	void orgnize_point_rectangle()//내부 정사각형 안에서 눈동자 점을 찾는 방법
	{
		int reali = -1;
		int prex = cropRect.x;
		int nexx = prex + cropRect.width;
		int prey = cropRect.y;
		int nexy = prey + cropRect.height;
		int diffpx, diffpy, diffnx, diffny;
		for (int i = 0; i < points.size(); i++)
		{
			diffpx = points.at(i).x - prex;
			diffpy = points.at(i).y - prey;
			diffnx = nexx - points.at(i).x;
			diffny = nexy - points.at(i).y;
			if (diffpx > 0 && diffpy > 0 && diffnx > 0 && diffny > 0) {
				reali = i;
				break;
			}
		}
		/*cout << "diff";
		cout << prex;
		cout << ":";
		cout << prey;
		cout << ":";
		cout << nexx;
		cout << ":";
		cout << nexy;
		cout << ":";
		cout << reali;
		cout << endl;*/
		if (reali != -1) {
			realpoint = points[reali];
			realrect = rectangles[reali];
		}
		else {
			realpoint = Point(-1, -1);
		}
	}
public:
	ImageOperate(char* _winName)
	{
		winName = _winName;
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
		for (int i = 0; i < contours.size(); ++i)
		{
			// Find ellipse
			RotatedRect ell;
			if (contours[i].size()>4)
				ell = fitEllipse(contours[i]);
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
			if (measure >0.5)
			{
				//cv::ellipse(img, ell, Scalar(0, measure * 255, 255 - measure * 255), 3);
				rectangles.push_back(ell);
				points.push_back(ell.center);
			}
		}
		orgnize_point_rectangle();

		//cv::circle(mat, realpoint, 5, Scalar(255, 0, 0));
		//std::cout << points.size() << ":" << rectangles.size() << endl;

		points.clear();
		rectangles.clear();
		dst[4][0] = mat.clone();
	}
	void Canny_demo(int, void*) {
		Canny(dst[2][1].clone(), dst[3][0], canny, canny * 3, 3);
	}

	void mainprocessing(Mat parm)
	{
		src = parm.clone();
		dst[0][0] = src.clone();
		dst[0][1] = src.clone();
		for (int i = 3; i < MAX_KERNEL_LENGTH; i = i + 2)
		{
			medianBlur(src, dst[1][0], i);
		}/// Convert the image to Gray
		dst[1][1] = dst[1][0].clone();
		threshold(dst[1][1], dst[2][0], 20, max_BINARY_value, 0);
		dst[2][1] = dst[2][0].clone();

		Canny_demo(0, 0);
		dst[3][1] = dst[3][0].clone();

		ellipseDetect();
		if (realpoint.x != -1 && realpoint.y != -1)
		{
			cv::ellipse(dst[0][0], realrect, Scalar(0, 0, 255));
			cv::circle(dst[0][0], realpoint, realrect.boundingRect2f().width>realrect.boundingRect2f().height ? realrect.boundingRect2f().height / 2 : realrect.boundingRect2f().width / 2, Scalar(0, 255, 0));
			/*cout << realpoint.x << ":" << realpoint.y << endl;
			cout << cropRect.x;
			cout << ":";
			cout << cropRect.y;
			cout << ":";
			cout << cropRect.width;
			cout << ":";
			cout << cropRect.height;
			cout << endl;*/
		}
	}
	Mat getDst(int i, int j)
	{
		return dst[i][j];
	}
};
ImageOperate im1(winName);
ImageOperate im2(winName2);

void checkBoundary(Mat img) {
	//check croping rectangle exceed image boundary
	if (cropRect.width>img.cols - cropRect.x)
		cropRect.width = img.cols - cropRect.x;

	if (cropRect.height>img.rows - cropRect.y)
		cropRect.height = img.rows - cropRect.y;

	if (cropRect.x<0)
		cropRect.x = 0;

	if (cropRect.y<0)
		cropRect.height = 0;
};
void checkBoundary2(Mat img) {
	//check croping rectangle exceed image boundary
	if (cropRect2.width>img.cols - cropRect2.x)
		cropRect2.width = img.cols - cropRect2.x;

	if (cropRect2.height>img.rows - cropRect2.y)
		cropRect2.height = img.rows - cropRect2.y;

	if (cropRect2.x<0)
		cropRect2.x = 0;
	
	if (cropRect2.y<0)
		cropRect2.height = 0;
};
void showImage(Mat frame, char* winName) {
	Mat img = frame.clone();
	rectangle(img, cropRect, Scalar(0, 255, 0), 2, 8, 0);
	imshow(winName, img);
	waitKey(27);
	//잘린 사진이 나오는 곳.
}
void showImage2(Mat frame, char* winName) {
	Mat img = frame.clone();
	rectangle(img, cropRect2, Scalar(0, 255, 0), 2, 8, 0);
	imshow(winName, img);
	waitKey(27);
	//잘린 사진이 나오는 곳.

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
	default:
		break;
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
}

void onMouse2(int event, int x, int y, int f, void*) {
	switch (event) {
	case  CV_EVENT_LBUTTONDOWN:
		clicked2 = true;
		P3.x = x;
		P3.y = y;
		P4.x = x;
		P4.y = y;
		//destroyWindow("cropped");
		break;
	case  CV_EVENT_LBUTTONUP:
		P4.x = x;
		P4.y = y;
		clicked2 = false;
		break;
	case  CV_EVENT_MOUSEMOVE:
		if (clicked2) {
			P4.x = x;
			P4.y = y;
		}
		break;
	default:
		break;
	}
	if (clicked2) {
		if (P3.x>P4.x) {
			cropRect2.x = P4.x;
			cropRect2.width = P3.x - P4.x;
		}
		else {
			cropRect2.x = P3.x;
			cropRect2.width = P4.x - P3.x;
		}

		if (P3.y>P4.y) {
			cropRect2.y = P4.y;
			cropRect2.height = P3.y - P4.y;
		}
		else {
			cropRect2.y = P3.y;
			cropRect2.height = P4.y - P3.y;
		}

	}
}

void showPicture(int, void*)
{
	// cout << picture_num << endl;
	rectangle(im1.getDst(picture_num,0), cropRect, Scalar(0, 255, 0), 2, 8, 0);
	imshow(winName, im1.getDst(picture_num,0));
	waitKey(27);

}
void showPicture2(int, void*)
{
	// cout << picture_num << endl;
	rectangle(im2.getDst(picture_num,0), cropRect2, Scalar(0, 255, 0), 2, 8, 0);
	imshow(winName2, im2.getDst(picture_num,0));
	waitKey(27);

}

int main()
{
	
	namedWindow(winName, CV_WINDOW_AUTOSIZE);
	namedWindow(winName2, CV_WINDOW_AUTOSIZE);
	VideoCapture capture("eye0.mp4");
	VideoCapture capture2("eye1.mp4");
	Mat frame;
	Mat frame2;
	capture >> frame;
	capture2 >> frame2;
	setMouseCallback(winName, onMouse, NULL);
	setMouseCallback(winName2, onMouse2, NULL);
	checkBoundary(frame);
	checkBoundary2(frame2);
	while(1){
		showImage(frame, winName);
		showImage2(frame2, winName2);
		if ((!clicked && (cropRect.width > 0 && cropRect.height > 0)) && (!clicked2 && (cropRect2.width > 0 && cropRect2.height > 0)))
			break;
	}
	//imshow(winName, frame);
	cvDestroyWindow(winName);
	cvDestroyWindow(winName2);

	createTrackbar(trackbar_type,
		winName, &picture_num,
		max_picture, showPicture);
	createTrackbar(trackbar_type,
		winName2, &picture_num2,
		max_picture, showPicture2);
	int i = 0;
	while (true)
	{
		capture >> frame;
		capture2 >> frame2;
		if(!frame.empty() && !frame2.empty()){
			im1.mainprocessing(frame);
			im2.mainprocessing(frame2);
			showPicture(0, 0);
			showPicture2(0, 0);
		}
		else
		{
			cout << i << endl;
			i++;
			continue;
		}
	}
	return 0;
}
