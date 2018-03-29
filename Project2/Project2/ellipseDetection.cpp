#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;

int main()
{
	// Load image
	Mat img = imread("capture2.png");

	// Convert to grayscale. Binarize if needed
	Mat1b bin;
	cvtColor(img, bin, COLOR_BGR2GRAY);

	// Find contours
	vector<vector<Point>> contours;
	findContours(bin.clone(), contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);//RETR_EXTERNAL
	drawContours(img, contours, 1, Scalar(0, 255, 0));
	imshow("Result", img);
	waitKey();


	return 0;
	// For each contour
	for (int i = 1; i < contours.size(); ++i)
	{
		// Find ellipse
		RotatedRect ell = fitEllipse(contours[i]);

		// Draw contour
		Mat1b maskContour(img.rows, img.cols, uchar(0));
		drawContours(maskContour, contours, i, Scalar(255), 2);

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
		circle(img, ell.center, 5, Scalar(0, 255, 0));

	}

	imshow("Result", img);
	waitKey();


	return 0;
}