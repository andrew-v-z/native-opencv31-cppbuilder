#include "opencv2/videoio/videoio.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <ctype.h>
#include <stdio.h>
#include <iostream>
#include <dir.h>
//#include <winuser.h>

using namespace cv;
using namespace std;

static void help()
{
	cout <<
			"\nThis program demonstrates Laplace point/edge detection using OpenCV function Laplacian()\n"
			"It captures from the camera of your choice: 0, 1, ... default 0\n"
			"Call:\n"
			"./laplace -c=<camera #>\n"
			"\t-i=<path to images>  default ../../../UFSIN-DATA/Pics\n"
			"\t-p=<index of the frame to be decoded/captured next>\n" << endl;
}

enum {GAUSSIAN, BLUR, MEDIAN};

int sigma = 3;
int smoothType = GAUSSIAN;

int main( int argc, char** argv )
{
	VideoCapture cap;
	const char * ipath;
	ffblk * sr;
	int iAttributes = FA_NORMAL;
	int  simp_files = 1;
	bool first_find = false;

	cv::CommandLineParser parser(argc, argv,
		"{help h||}"
		"{ c|| }"
		"{ i|../../../UFSIN-DATA/Pics| }"
		"{ p|| }");
	if ( parser.has("help") )
	{
		help();
		return 0;
	}

	if( parser.get<String>("c").size() == 1 && isdigit(parser.get<String>("c")[0]) )
		cap.open(parser.get<int>("c"));
	else if ( parser.get<String>("i").size()>1 ) {
		simp_files = 0;
		ipath = (parser.get<String>("i")).c_str();
	}
	else
		cap.open(parser.get<String>("c"));
	if( cap.isOpened() )
	{
		cout << "Video " << (parser.get<String>("c")).c_str() <<
			": width=" << cap.get(CAP_PROP_FRAME_WIDTH) <<
			", height=" << cap.get(CAP_PROP_FRAME_HEIGHT) <<
			", nframes=" << cap.get(CAP_PROP_FRAME_COUNT) << endl;
		if( parser.has("p") )
		{
			int pos = parser.get<int>("p");
			if (!parser.check())
			{
				parser.printErrors();
				return -1;
			}
			cout << "seeking to frame #" << pos << endl;
			cap.set(CAP_PROP_POS_FRAMES, pos);
		}
	}

	if( !cap.isOpened() && simp_files>0)
	{
		cout << "Could not initialize capturing...\n";
		return -1;
	}
	else if (simp_files==0) {
		char  p[111];
		p[0] = 0;
		strcat(p,ipath);
		strcat(p,"/");
		strcat(p,"*.*");
		simp_files = findfirst(p, sr, iAttributes);
		first_find = true;
	}

	namedWindow( "Laplacian", 0 );
	createTrackbar( "Sigma", "Laplacian", &sigma, 15, 0 );

	Mat smoothed, laplace, result;
	bool next_file = false, prev_file = false;
	int FileNumber;
	Mat frame;

	for(;;)
	{
	  if (simp_files==0)
	  {
			if (first_find) {
				char  p[111];
				p[0] = 0;
				strcat(p,ipath);
				strcat(p,"/");
				FileNumber = 0;
				frame = imread((String) strcat(p,sr->ff_name), 1);
				first_find = false;
			} else {
				if (next_file) {
					char  p[111];
					p[0] = 0;
					strcat(p,ipath);
					strcat(p,"/");
					if (findnext(sr)==0) {
						if (sr->ff_attrib != FA_DIREC) {
							frame = imread((String) strcat(p,sr->ff_name), 1);
						}
					}
				} else 	if ( prev_file) {
					char  p[111];
					p[0] = 0;
					strcat(p,ipath);
					strcat(p,"/");
					strcat(p,"*.*");
					simp_files = findfirst(p, sr, iAttributes);
					int i=0;
					do {
					} while (findnext(sr)==0 && ++i<FileNumber);
					if (sr->ff_attrib != FA_DIREC) {
						p[0] = 0;
						strcat(p,ipath);
						strcat(p,"/");
						frame = imread((String) strcat(p,sr->ff_name), 1);
					}
				}
			}
	  }
	  else
	  {
		 cap >> frame;
		 if( frame.empty() )
			break;
	  }

	  int ksize = (sigma*5)|1;
	  if(smoothType == GAUSSIAN)
		GaussianBlur(frame, smoothed, Size(ksize, ksize), sigma, sigma);
	  else if(smoothType == BLUR)
		blur(frame, smoothed, Size(ksize, ksize));
	  else
		medianBlur(frame, smoothed, ksize);

	  Laplacian(smoothed, laplace, CV_16S, 5);
	  convertScaleAbs(laplace, result, (sigma+1)*0.25);
	  imshow("Laplacian", result);

	  int c = waitKey(30);
	  if (simp_files==0){
		 for(;;) {
			c = waitKey(30);
			if( c == ' ' ) {
				break;
			}
			if( c == 'q' || c == 'Q' || (c & 255) == 27 )
			   break;
			if( (c>>16) == 0x25 ) {
				prev_file = true;
				next_file = false;
				if (FileNumber>0) FileNumber--;
				break;
			}
			if( (c>>16) == 0x27 ) {
				prev_file = false;
				next_file = true;
				FileNumber++;
				break;
			}
		 }
	  }
	  if( c == ' ' ) {
		prev_file = false;
		next_file = false;
		smoothType = smoothType == GAUSSIAN ? BLUR : smoothType == BLUR ? MEDIAN : GAUSSIAN;
	  }
	  if( c == 'q' || c == 'Q' || (c & 255) == 27 )
		break;
	}

	findclose(sr);
	return 0;
}
