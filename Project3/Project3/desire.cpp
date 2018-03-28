#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <queue>
#include <stdio.h>
#include <math.h>
/** Constants **/


/** Function Headers */
void detectAndDisplay(cv::Mat frame);

/** Global variables */
//-- Note, either copy these two files from opencv/data/haarscascades to your current folder, or change these locations
cv::String face_cascade_name = "res/haarcascade_frontalface_alt.xml";
cv::CascadeClassifier face_cascade;
std::string main_window_name = "Capture - Face detection";
std::string face_window_name = "Capture - Face";
cv::RNG rng(12345);
cv::Mat debugImage;
cv::Mat skinCrCbHist = cv::Mat::zeros(cv::Size(256,256), CV_8UC1);

/**
* @function main
*/
int main(int argc, const char** argv) {
	cv::Mat frame;

	// Load the cascades
	if (!face_cascade.load(face_cascade_name)) { printf("--(!)Error loading face cascade, please change face_cascade_name in source code.\n"); return -1; };
	cv::namedWindow(main_window_name, CV_WINDOW_NORMAL);
	cv::moveWindow(main_window_name, 400, 100);
	cv::namedWindow(face_window_name, CV_WINDOW_NORMAL);
	cv::moveWindow(face_window_name, 10, 100);
	cv::namedWindow("Right Eye", CV_WINDOW_NORMAL);
	cv::moveWindow("Right Eye", 10, 600);
	cv::namedWindow("Left Eye", CV_WINDOW_NORMAL);
	cv::moveWindow("Left Eye", 10, 800);
	cv::namedWindow("aa", CV_WINDOW_NORMAL);
	cv::moveWindow("aa", 10, 800);
	cv::namedWindow("aaa", CV_WINDOW_NORMAL);
	cv::moveWindow("aaa", 10, 800);

	//ellipse(skinCrCbHist, cv::Point(113, 155.6), cv::Size(23.4, 15.2),43.0, 0.0, 360.0, cv::Scalar(255, 255, 255), -1);

	// I make an attempt at supporting both 2.x and 3.x OpenCV
#if CV_MAJOR_VERSION < 3
	CvCapture* capture = cvCaptureFromCAM(0);
	if (capture) {
		while (true) {
			frame = cvQueryFrame(capture);
#else
	cv::VideoCapture capture("video.mp4");
	if (capture.isOpened()) {
		std::cout << "dkdkd" << std::endl;
		while (true) {
			capture.read(frame);
#endif
			// mirror it
			cv::flip(frame, frame, 1);
			frame.copyTo(debugImage);

			// Apply the classifier to the frame
			if (!frame.empty()) {
				detectAndDisplay(frame);

			}
			else {
				printf(" --(!) No captured frame -- Break!");
				break;
			}

			imshow(main_window_name, debugImage);

			int c = cv::waitKey(10);
			if ((char)c == 'c') { break; }
			if ((char)c == 'f') {
				imwrite("frame.png", frame);
			}
		}
	}
	return 0;
}
void detectAndDisplay(cv::Mat frame) {
	std::vector<cv::Rect> faces;
	//cv::Mat frame_gray;

	std::vector<cv::Mat> rgbChannels(3);
	cv::split(frame, rgbChannels);
	cv::Mat frame_gray = rgbChannels[2];

	//cvtColor( frame, frame_gray, CV_BGR2GRAY );
	//equalizeHist( frame_gray, frame_gray );
	//cv::pow(frame_gray, CV_64F, frame_gray);
	//-- Detect faces
	//face_cascade.detectMultiScale(frame_gray, faces, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE | CV_HAAR_FIND_BIGGEST_OBJECT, cv::Size(150, 150));
	//  findSkin(debugImage);

	for (int i = 0; i < faces.size(); i++)
	{
		rectangle(debugImage, faces[i], 1234);
	}
	//-- Show what you got
	//if (faces.size() > 0) {
		//findEyes(frame_gray, faces[0]);
	//}
}
