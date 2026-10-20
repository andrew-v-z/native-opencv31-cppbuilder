#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <math.h>
#include <iostream>

using namespace cv;
using namespace std;

static void help()
{
	cout
	<< "\nThis program illustrates the use of findContours and drawContours\n"
	<< "The original image is put up along with the image of drawn contours\n"
	<< "Usage:\n"
	<< "./contours2\n"
	<< "\nA trackbar is put up which controls the contour level from -3 to 3\n"
	<< endl;
}

int w = 500;
int h = 500;
int levels = 0;

vector<vector<Point> > contours, contours0;
vector<Vec4i> hierarchy0;

static void on_trackbar(int, void*)
{
	int maxx=0,maxy=0,minx=999990,miny=99990;
	size_t k;
	Mat cnt_img;
	cnt_img = Mat::zeros(h, w, CV_8UC3);
	contours.resize(contours0.size());
	for( k=0; k < contours0.size(); k++ ) {
	  if (contours0[k].size()<3) {
//		contours0[k].~vector();
		continue;
	  }
	  for( size_t kv = 0; kv < contours0[k].size(); kv++ ) {
		if (contours0[k].at(kv).x >maxx) maxx=contours0[k].at(kv).x;
		if (contours0[k].at(kv).y >maxy) maxy=contours0[k].at(kv).y;
		if (contours0[k].at(kv).x <minx) minx=contours0[k].at(kv).x;
		if (contours0[k].at(kv).y <miny) miny=contours0[k].at(kv).y;
	  }
 	  if ((maxx-minx<31) && (maxy-miny<31)) continue;//contours[k].~vector();
	  approxPolyDP(Mat(contours0[k]), contours[k], levels/5+1, true);
	  if (contours[k].size()<3) contours[k].~vector();
	  maxx=0;maxy=0;minx=999990;miny=99990;
	  for( size_t kv = 0; kv < contours[k].size(); kv++ ) {
		if (contours[k].at(kv).x >maxx) maxx=contours[k].at(kv).x;
		if (contours[k].at(kv).y >maxy) maxy=contours[k].at(kv).y;
		if (contours[k].at(kv).x <minx) minx=contours[k].at(kv).x;
		if (contours[k].at(kv).y <miny) miny=contours[k].at(kv).y;
	  }
	  if ((maxx-minx<31) && (maxy-miny<31)) contours[k].~vector();
	}
	drawContours( cnt_img, contours, -1, Scalar(128,255,255), 1, LINE_AA, hierarchy0, levels/5==0 ? 2 : levels/5+2 );

	imshow("contours", cnt_img);
}

int main( int argc, char** argv)
{
	cv::CommandLineParser parser(argc, argv, "{help h||}");
	if (parser.has("help"))
	{
		help();
		return 0;
	}
	Mat img, grey;
	img = imread("c:/Users/admin/PROGS/CV/2.bmp", 1);
	w=img.cols;
	h=img.rows;
	if(img.empty())
	{
		printf("Cannot read image file");
		help();
		return -1;
	}
	cvtColor(img, grey, CV_BGR2GRAY);
	adaptiveThreshold(grey, grey, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, 1, 17, 12.0);

	namedWindow( "image", 1 );
	imshow( "image", grey );
	//Extract the contours so that
	findContours( grey, contours0, hierarchy0, RETR_TREE, CHAIN_APPROX_SIMPLE);

	namedWindow( "contours", 1 );
	createTrackbar( "levels", "contours", &levels, 50, on_trackbar );

	on_trackbar(0,0);
	waitKey();

	return 0;
}
