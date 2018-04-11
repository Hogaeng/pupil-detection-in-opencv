#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <cmath>
#include <thread>
using namespace std;
using namespace cv;
/// Global variables
int threshold_value = 20;//binary image
int canny = 0;//edge algorithm
int picture_num = 0;//the pictures
int picture_num2 = 0;//the pictures

int const max_value = 255;
int const max_type = 4;
int const max_picture = 4;
int const max_canny = 100;
int const max_binary_value = 255;
int MAX_KERNEL_LENGTH = 15;

//{[0]:  median filtering, [1]:threshold algorithm, [2]:edge detection, [3]:ellipse detection}
//{[0]은 단계별 결과 데이터, [1]은 [0]을 다음 단계의 입력값으로 넣게 되어 손상된 데이터}
char* trackbar_value = "Value";
/// Function headers
Point P1(0, 0);
Point P2(0, 0);
Point P3(0, 0);
Point P4(0, 0);

bool clicked = false;
bool clicked2 = false;

char* winName = "eye0_";
char* winName2 = "eye1_";

float perPixel = 7.2;
float NOWIPD = 65;
class ImageOperate
{
private:
	Mat src, src_gray, dst[5][2];
	vector<Point> points, realpoints;
	vector<RotatedRect> rectangles, realrectangles;
	Point realpoint;
	RotatedRect realrect;
	char * winName;
	Rect cropRect;
	void orgnize_point_center()//Deprecated.무조건 중앙의 점을 눈동자의 점이라고 인식하는 방법
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
		
		if (reali != -1) {
			/*cout << "d_"<<winName;
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
		cropRect = Rect(0, 0, 0, 0);
	}
	void ellipseDetect()//다각형을 찾고 그 다각형에 맞는 타원을 찾아내는 함수
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

	bool mainprocessing(Mat parm)//메디안 필터->흑백 변환 필터 -> canny의 엣지알고리즘->타원(동공)찾기->동공 그리기
	{
		src = parm.clone();
		dst[0][0] = src.clone();
		dst[0][1] = src.clone();
		for (int i = 3; i < MAX_KERNEL_LENGTH; i = i + 2)
		{
			medianBlur(src, dst[1][0], i);
		}/// Convert the image to Gray
		dst[1][1] = dst[1][0].clone();
		threshold(dst[1][1], dst[2][0], 20, max_binary_value, 0);
		dst[2][1] = dst[2][0].clone();

		Canny_demo(0, 0);
		dst[3][1] = dst[3][0].clone();

		ellipseDetect();
		if (realpoint.x != -1 && realpoint.y != -1)
		{
			cv::ellipse(dst[0][0], realrect, Scalar(0, 0, 255));
			cv::circle(dst[0][0], realpoint, realrect.boundingRect2f().width > realrect.boundingRect2f().height ? realrect.boundingRect2f().height / 2 : realrect.boundingRect2f().width / 2, Scalar(0, 255, 0));
			/*cout << winName;
			cout << ";";
			cout << realpoint.x << ":" << realpoint.y;
			cout << ";";
			cout << cropRect.x;
			cout << ":";
			cout << cropRect.y;
			cout << ":";
			cout << cropRect.width;
			cout << ":";
			cout << cropRect.height;
			cout << endl;*/
			return true;
		}
		else
			return false;
	}
	Mat getDst(int i, int j)
	{
		return dst[i][j];
	}
	Rect getcropRect()
	{
		return cropRect;
	}
	void setcropRect(int x,int y,int w, int h)
	{
		cropRect.x = x;
		cropRect.y = y;
		cropRect.width = w;
		cropRect.height = h;
	}
	Point getRealpoint()
	{
		return realpoint;
	}
};
ImageOperate im1(winName);
ImageOperate im2(winName2);

void checkBoundary(Mat img,ImageOperate im) {//check croping rectangle exceed image boundary
	int x = im.getcropRect().x;
	int y = im.getcropRect().y;
	int height = im.getcropRect().height;
	int width = im.getcropRect().width;
	if (im.getcropRect().width>img.cols - im.getcropRect().x)
		width = img.cols - im.getcropRect().x;

	if (im.getcropRect().height>img.rows - im.getcropRect().y)
		height = img.rows - im.getcropRect().y;

	if (im.getcropRect().x<0)
		x = 0;

	if (im.getcropRect().y<0)
		height = 0;
	im.setcropRect(x, y, width, height);
};
void showImage(Mat frame, char* winName,ImageOperate im) {
	Mat img = frame.clone();
	rectangle(img, im.getcropRect(), Scalar(0, 255, 0), 2, 8, 0);
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
	int cx = im1.getcropRect().x;
	int cy = im1.getcropRect().y;
	int cwidth = im1.getcropRect().width;
	int cheight = im1.getcropRect().height;
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
			cx = P2.x;
			cwidth = P1.x - P2.x;
		}
		else {
			cx = P1.x;
			cwidth = P2.x - P1.x;
		}

		if (P1.y>P2.y) {
			cy = P2.y;
			cheight = P1.y - P2.y;
		}
		else {
			cy = P1.y;
			cheight = P2.y - P1.y;
		}
	}
	im1.setcropRect(cx, cy, cwidth, cheight);
}

void onMouse2(int event, int x, int y, int f, void*) {
	int cx = im2.getcropRect().x;
	int cy = im2.getcropRect().y;
	int cwidth = im2.getcropRect().width;
	int cheight = im2.getcropRect().height;
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
			cx = P4.x;
			cwidth = P3.x - P4.x;
		}
		else {
			cx = P3.x;
			cwidth = P4.x - P3.x;
		}

		if (P3.y>P4.y) {
			cy = P4.y;
			cheight = P3.y - P4.y;
		}
		else {
			cy = P3.y;
			cheight = P4.y - P3.y;
		}

	}
	im2.setcropRect(cx, cy, cwidth, cheight);

}

void showPicture(int, void*)
{
	// cout << picture_num << endl;
	rectangle(im1.getDst(picture_num,0), im1.getcropRect(), Scalar(0, 255, 0), 2, 8, 0);
	circle(im1.getDst(picture_num, 0), Point(im1.getcropRect().x + im1.getcropRect().width / 2, im1.getcropRect().y + im1.getcropRect().height / 2), 3, Scalar(255, 0, 0), 2, 8);
	imshow(winName, im1.getDst(picture_num,0));
	waitKey(27);
}
void showPicture2(int, void*)
{
	// cout << picture_num << endl;
	rectangle(im2.getDst(picture_num,0), im2.getcropRect(), Scalar(0, 255, 0), 2, 8, 0);
	circle(im2.getDst(picture_num, 0), Point(im2.getcropRect().x + im2.getcropRect().width / 2, im2.getcropRect().y + im2.getcropRect().height / 2), 3, Scalar(255, 0, 0), 2, 8);
	imshow(winName2, im2.getDst(picture_num,0));
	waitKey(27);
}
bool pause = false;

void calculateIPD(Point rpivot, Point lpivot)
{
	Point null(-1, -1);
	Point right = null;
	Point left = null;
	right = im1.getRealpoint();
	left = im2.getRealpoint();
	if (right == null || left==null)
	{
		return;
	}
	int d_lx = left.x -lpivot.x ;
	int d_rx = right.x - rpivot.x;
	int d_ly = lpivot.y - left.y;
	int d_ry = rpivot.y - right.y;
	/*if (!pause)
		cout << "lx:" << d_lx << ":" << d_ly << ":";*/
	double drpix = sqrt(pow(d_rx, 2) + pow(d_ry, 2));
	double dlpix = sqrt(pow(d_lx, 2) + pow(d_ly, 2));
	double mmr = drpix / perPixel;
	double mml = dlpix / perPixel;
	/*if (!pause){
		cout << "pix:" << drpix << ":" << dlpix << ":";
		cout << "mmm:" << mmr << ":" << mml << ":" <<endl;
	}*/
	bool nw_right, nw_left; /// true는 넓어지는 방향, false는 좁아지는 방향

	/*if (d_rx == 0) /// y축만 +차이가 있는 경우, 좁아진다고 하자.
		if (d_ry > 0)
			pm_right = false;
		else
			pm_right = true;
	
	else if ((float)d_ry/d_rx > 0)/// 오른쪽 위로 뻗으면 좁아지니 false, 왼쪽 위로 뻗으면 좁아지니 true;
			pm_right = false;
	else
		pm_right = true;

	if (d_lx == 0) /// y축만 +차이가 있는 경우, 좁아진다고 하자.
		if (d_ly > 0)
			pm_left = false;
		else
			pm_left = true;
	else if ((float)d_ly / d_lx > 0) /// 오른쪽 위로 뻗으면 넓어지니 true, 왼쪽 위로 뻗으면 좁아지니 false
		pm_left = true;
	else
		pm_left = false;*/
	if (d_rx > 0)
		nw_right = false;
	else
		nw_right = true;
	if (d_lx > 0)
		nw_left = true;
	else
		nw_left=false;
	/*if (!pause)
		cout << nw_right << endl;*/
	/*if (!pause)
		cout << nw_left << endl;*/
	if (!nw_right)
		mmr = -mmr;
	if (!nw_left)
		mml = -mml;

	if (!pause)
		cout << "IPD:" << NOWIPD + mml + mmr << endl;

}
void func()
{
	if (cv::waitKey(1) == 'p')
			pause = !pause;
}
int main()
{
	while(1){
		cin >> NOWIPD;
		break;
	}
	cout << "NOWIPD:" << NOWIPD << endl;
	//IPD 적는 단계
	namedWindow(winName, CV_WINDOW_AUTOSIZE);
	namedWindow(winName2, CV_WINDOW_AUTOSIZE);
	VideoCapture capture("eye0.mp4");//right eye
	VideoCapture capture2("eye1.mp4");//left eye
	Mat frame;
	Mat frame2;
	Point rpivot;
	Point lpivot;
	//두 눈 중앙값 지정단계
	capture >> frame;
	flip(frame, frame, -1);
	capture2 >> frame2;
	setMouseCallback(winName, onMouse, NULL);
	setMouseCallback(winName2, onMouse2, NULL);
	checkBoundary(frame,im1);
	checkBoundary(frame2,im2);
	while(1){
		showImage(frame, winName,im1);
		showImage(frame2, winName2,im2);
		if ((!clicked && (im1.getcropRect().width > 0 && im1.getcropRect().height > 0)) && (!clicked2 && (im2.getcropRect().width > 0 && im2.getcropRect().height > 0)))
			break;
	}
	rpivot = Point(im1.getcropRect().x + im1.getcropRect().width / 2, im1.getcropRect().y + im1.getcropRect().height/2);
	lpivot = Point(im2.getcropRect().x + im2.getcropRect().width / 2, im2.getcropRect().y + im2.getcropRect().height/2);
	//초록 사각형 지정하는 단계
	cvDestroyWindow(winName);
	cvDestroyWindow(winName2);
	createTrackbar(winName,
		winName, &picture_num,
		max_picture, showPicture);
	createTrackbar(winName2,
		winName2, &picture_num2,
		max_picture, showPicture2);
	int i = 0;
	int j = 0;
	while (true)
	{
		thread t1;
		thread t2;
		if (cv::waitKey(1) == 'p')
			pause = !pause;
		if (!pause)
		{
			capture >> frame;
			flip(frame, frame, -1);
			capture2 >> frame2;	
			//im1.mainprocessing(frame);
			//im2.mainprocessing(frame2);
			
		}
		if (frame.empty() && frame2.empty())
			break;
		else if (!frame.empty() && !frame2.empty()) {	
			/*t1 = thread(&ImageOperate::mainprocessing, &im1, frame);
			t2 = thread(&ImageOperate::mainprocessing, &im2, frame2);
			t1.join();
			t2.join();
			showPicture(0, 0);
			showPicture2(0, 0);
			calculateIPD(rpivot, lpivot);*/
		}
		else if (frame.empty()){
			cout << "end_0_" << i << endl;
			i--;
		}
		else if (frame2.empty()){
			cout << "end_1_" << j << endl;
			j--;
		}
		i++;
		j++;
		
	}
	///동공 추적 후 IPD 계산단계
	cout << "frame: " << i << endl;
	cout << "frame2: " << j << endl;
	return 0;
}
