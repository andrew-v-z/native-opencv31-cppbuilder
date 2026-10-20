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
	cout << "\nUsage:\n"
			"camera [--try-flip]\n\n"
			"see camera.cmd for one call:\n"
			"camera --try-flip\n\n"
			"During execution:\n\tHit any key to quit.\n"
			"\tUsing OpenCV version " << CV_VERSION << "\n" << endl;
}

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
		"{try-flip||}"
	);
	if (parser.has("help"))	{
		help();
		return 0;
	}

	tryflip = parser.has("try-flip");
	if (!parser.check()) {
		parser.printErrors();
		return 0;
	}
	if(!capture.open(1))
		cout << "Capture from camera #" <<  1 << " didn't work" << endl;

	if( capture.isOpened() ) {
		cout << "Video capturing has been started ..." << endl;

		for(;;)	{
			capture >> frame;
			if( frame.empty() )
				break;

			Mat frame1 = frame.clone();
			if( tryflip )	flip(frame1, frame1, 0);
			imshow( "Camera", frame1 );

			int c = waitKey(10);
			if( c == 27 || c == 'q' || c == 'Q' )
				break;
		}
	}

	return 0;
}

