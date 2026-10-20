#pragma hdrstop

#include <tchar.h>
//---------------------------------------------------------------------------
#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
#include "opencv2/core/core_c.h"

using namespace std;
using namespace cv;

static void help()
{
	cout << "\nThis program demonstrates the cascade recognizer. Now you can use Haar or LBP features.\n"
            "This classifier can recognize many kinds of rigid objects, once the appropriate classifier is trained.\n"
			"It's most known use is for faces.\n"
            "Usage:\n"
            "./facedetect [--cascade=<cascade_path> this is the primary trained classifier such as frontal face]\n"
               "   [--nested-cascade[=nested_cascade_path this an optional secondary classifier such as eyes]]\n"
               "   [--scale=<image scale greater or equal to 1, try 1.3 for example>]\n"
			   "   [--try-flip]\n"
               "   [filename|camera_index]\n\n"
            "see facedetect.cmd for one call:\n"
            "./facedetect --cascade=\"../../data/haarcascades/haarcascade_frontalface_alt.xml\" --nested-cascade=\"../../data/haarcascades/haarcascade_eye_tree_eyeglasses.xml\" --scale=1.3\n\n"
            "During execution:\n\tHit any key to quit.\n"
            "\tUsing OpenCV version " << CV_VERSION << "\n" << endl;
}

void detectAndDraw( Mat& img, CascadeClassifier& cascade,
                    CascadeClassifier& nestedCascade,
                    double scale, bool tryflip );

String cascadeName;                             ///ggg///
String nestedCascadeName;                        ///ggg///

//---------------------------------------------------------------------------

#pragma argsused
int _tmain(int argc, _TCHAR* argv[]) {
	VideoCapture capture;
	Mat frame, image;                             ///ggg///
    String inputName;
    bool tryflip;
    CascadeClassifier cascade, nestedCascade;
    double scale;

	cv::CommandLineParser parser(argc, argv,
		"{help h||}"
		"{cascade|../../data/haarcascades/haarcascade_frontalface_alt.xml|}"
		"{nested-cascade|../../data/haarcascades/haarcascade_eye_tree_eyeglasses.xml|}"
		"{scale|1|}{try-flip||}{@filename||}"
	);
	if (parser.has("help"))	{
		help();
		return 0;
	}

	cascadeName = parser.get<String>("cascade");                       ///ggg///
	nestedCascadeName = parser.get<String>("nested-cascade");
	scale = parser.get<double>("scale");                               ///ggg///
	if (scale < 1)
		scale = 1;
	tryflip = parser.has("try-flip");
	inputName = parser.get<String>("@filename");                       ///ggg///
	if (!parser.check()) {
		parser.printErrors();
		return 0;
	}
	if ( !nestedCascade.load( nestedCascadeName ) )
		cerr << "WARNING: Could not load classifier cascade for nested objects" << endl;
	if( !cascade.load( cascadeName ) ) {
		cerr << "ERROR: Could not load classifier cascade" << endl;
		help();
		return -1;
	}
	if( inputName.empty() || (isdigit(inputName[0]) && inputName.size() == 1) ) {
		int c = inputName.empty() ? 0 : inputName[0] - '0';
		if(!capture.open(c))
			cout << "Capture from camera #" <<  c << " didn't work" << endl;
	} else if( inputName.size() ) {
		image = imread( inputName, 1 );
		if( image.empty() )	{
			if(!capture.open( inputName ))
				cout << "Could not read " << inputName.c_str() << endl;
			image = imread( "../../sources/samples/data/lena.jpg", 1 );
			if(image.empty()) cout << "Couldn't read ../../samples/data/lena.jpg" << endl;
		}
	}

	if( capture.isOpened() ) {
		cout << "Video capturing has been started ..." << endl;

		for(;;)	{
			capture >> frame;
			if( frame.empty() )
				break;

			Mat frame1 = frame.clone();
			flip(frame1, frame1, 0);
			detectAndDraw( frame1, cascade, nestedCascade, scale, tryflip );

			int c = waitKey(10);
			if( c == 27 || c == 'q' || c == 'Q' )
				break;
		}
	} else {
		cout << "Detecting face(s) in " << inputName.c_str() << endl;
		if( !image.empty() ) {
			detectAndDraw( image, cascade, nestedCascade, scale, tryflip );
			waitKey(0);
		} else if( !inputName.empty() )	{
			/* assume it is a text file containing the
			list of the image filenames to be processed - one per line */
			FILE* f = fopen( inputName.c_str(), "rt" );
			if( f )	{
				char buf[1000+1];
				while( fgets( buf, 1000, f ) ) {
					int len = (int)strlen(buf), c;
					while( len > 0 && isspace(buf[len-1]) )
						len--;
					buf[len] = '\0';
					cout << "file " << buf << endl;
					image = imread( buf, 1 );
					if( !image.empty() )
					{
						detectAndDraw( image, cascade, nestedCascade, scale, tryflip );
						c = waitKey(0);
						if( c == 27 || c == 'q' || c == 'Q' )
							break;
					} else {
						cerr << "Aw snap, couldn't read image " << buf << endl;
					}
				}
				fclose(f);
			}
		}
	}

	return 0;
}

void detectAndDraw( Mat& img, CascadeClassifier& cascade,
					CascadeClassifier& nestedCascade,
					double scale, bool tryflip ) {
	double t = 0;
	vector<Rect> faces, faces2, faces_flip;
	const static Scalar colors[] =
	{
		Scalar(255,0,0),
		Scalar(255,128,0),
		Scalar(255,255,0),
		Scalar(0,255,0),
		Scalar(0,128,255),
		Scalar(0,255,255),
		Scalar(0,0,255),
		Scalar(255,0,255)
	};
	Mat gray, smallImg, smallImg_flip;

	double fx = 1 / scale;
	resize( img, img, Size(), fx, fx, INTER_LINEAR );
	cvtColor( img, gray, COLOR_BGR2GRAY );
	smallImg = gray;

//	cvtColor( img, gray, COLOR_BGR2GRAY );
//	double fx = 1 / scale;
//	resize( gray, smallImg, Size(), fx, fx, INTER_LINEAR );
	equalizeHist( smallImg, smallImg );

	t = (double)cvGetTickCount();
    cascade.detectMultiScale( smallImg, faces,
		1.1, 2, 0
        //|CASCADE_FIND_BIGGEST_OBJECT
        //|CASCADE_DO_ROUGH_SEARCH
        |CASCADE_SCALE_IMAGE,
        Size(30, 30) );
	if( tryflip ) {
		flip(smallImg, smallImg_flip, 1);
		cascade.detectMultiScale( smallImg_flip, faces2,
                                 1.1, 2, 0
                                 //|CASCADE_FIND_BIGGEST_OBJECT
								 //|CASCADE_DO_ROUGH_SEARCH
                                 |CASCADE_SCALE_IMAGE,
                                 Size(30, 30) );
		for( vector<Rect>::const_iterator ri = faces2.begin(); ri != faces2.end(); ri++ )
			faces_flip.push_back(Rect(ri->x, ri->y, ri->width, ri->height));

	}
	t = (double)cvGetTickCount() - t;
	printf( "detection time = %g ms\n", t/((double)cvGetTickFrequency()*1000.) );

	for ( size_t i = 0; i < faces.size(); i++ )	{
		Rect r = faces[i];
		Mat smallImgROI;
		vector<Rect> nestedObjects;
		Point center;
		Scalar color = colors[i%8];
		int radius;

		double aspect_ratio = (double)r.width/r.height;
		if( 0.85 < aspect_ratio && aspect_ratio < 1.18 ) {
//			center.x = cvRound((r.x + r.width*0.5)*scale);
//			center.y = cvRound((r.y + r.height*0.5)*scale);
//			radius = cvRound((r.width + r.height)*0.25*scale);
			center.x = cvRound((r.x + r.width*0.5));
			center.y = cvRound((r.y + r.height*0.5));
			radius = cvRound((r.width + r.height)*0.25);
			circle( img, center, radius, color, 3, 8, 0 );
		}
		else
			rectangle( img, cvPoint(cvRound(r.x), cvRound(r.y)),
					   cvPoint(cvRound((r.x + r.width-1)), cvRound((r.y + r.height-1))),
					   color, 3, 8, 0);
		if( nestedCascade.empty() )
			continue;
		smallImgROI = smallImg( r );
		nestedCascade.detectMultiScale( smallImgROI, nestedObjects,
			1.1, 2, 0
			//|CASCADE_FIND_BIGGEST_OBJECT
			//|CASCADE_DO_ROUGH_SEARCH
			//|CASCADE_DO_CANNY_PRUNING
			|CASCADE_SCALE_IMAGE,
			Size(3, 30) );
		for ( size_t j = 0; j < nestedObjects.size(); j++ )	{
			Rect nr = nestedObjects[j];
			center.x = cvRound((r.x + nr.x + nr.width*0.5));
			center.y = cvRound((r.y + nr.y + nr.height*0.5));
			radius = cvRound((nr.width + nr.height)*0.25);
//			circle( img, center, radius, color, 3, 8, 0 );
			rectangle(img, Rect(Point(r.x + nr.x, r.y + nr.y), Size(nr.width, nr.height)), Scalar(color), 2);
		}
	}
	if ( tryflip ) {
	  for ( size_t i = 0; i < faces_flip.size(); i++ ) {
		Rect r = faces_flip[i];
		Mat smallImgROI;
		vector<Rect> nestedObjects;
		Point center;
		Scalar color = colors[i%8];
		int radius, rx;

		rx = smallImg.cols - r.x - r.width;
		double aspect_ratio = (double)r.width/r.height;
		if( 0.85 < aspect_ratio && aspect_ratio < 1.18 ) {
			center.x = cvRound((rx + r.width*0.5));
			center.y = cvRound((r.y + r.height*0.5));
			radius = cvRound((r.width + r.height)*0.25);
			circle( img, center, radius, color, 3, 8, 0 );
		} else
			rectangle( img, cvPoint(cvRound(r.x), cvRound(r.y)),
					   cvPoint(cvRound((r.x + r.width-1)), cvRound((r.y + r.height-1))),
					   color, 3, 8, 0);
		if( nestedCascade.empty() )
			continue;
		smallImgROI = smallImg_flip( r );
		nestedCascade.detectMultiScale( smallImgROI, nestedObjects,
			1.1, 2, 0
			//|CASCADE_FIND_BIGGEST_OBJECT
			//|CASCADE_DO_ROUGH_SEARCH
			//|CASCADE_DO_CANNY_PRUNING
			|CASCADE_SCALE_IMAGE,
			Size(3, 30) );
		for ( size_t j = 0; j < nestedObjects.size(); j++ ) {
			Rect nr = nestedObjects[j];
			center.x = cvRound(rx + r.width - nr.x - nr.width*0.5);
			center.y = cvRound((r.y + nr.y + nr.height*0.5));
			radius = cvRound((nr.width + nr.height)*0.25);
			circle( img, center, radius, color, 3, 8, 0 );
		}
	  }
	}

	imshow( "result", img );
}