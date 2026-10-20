/*Wiener deconvolution.
Sample shows how DFT can be used to perform Weiner deconvolution [1]
of an image with user-defined point spread function (PSF)
*/


#include <signal.h>
#include "opencv2/photo.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/core.hpp"
#include <iostream.h>
#include <stdlib.h>

using namespace std;
using namespace cv;

char * win = "deconvolution";
bool defocus;
Mat img, IMG, img1;

static float deg2rad(float degrees) {return degrees * M_PI / 180;}

Mat blur_edge(Mat im, int d=31) {
	int h = im.rows, w  = im.cols;
	Mat img_pad;
	copyMakeBorder(im, img_pad, d, d, d, d, BORDER_WRAP);
	img_pad = im.clone();
	GaussianBlur(img_pad, img_pad, Size(2*d+1, 2*d+1), -1);
	Mat img_blur;
	img_blur = img_pad.rowRange(d, h-d).colRange(d, w-d);
	int y=h, x=w;

/*	int type = im.type(), rows = img.rows;
	size_t step = im.step;
	const uchar* m = im.ptr();
	float mmm=99999;
	for( int i = 0; i < im.cols*rows; i++ )
	{
		float mem = ((float*)(m + i*sizeof(float)))[0];
		mmm = min(mmm,mem);
	}

	double dist = mmm;
	float w1 = min(dist/d, 1.0);
//	return (im.rowRange(d, h-d).colRange(d, w-d)*w1 + img_blur*(1-w1)).operator Mat();
	return im.rowRange(d, h-d).colRange(d, w-d);
*/
	Mat pmid = img.rowRange(d, h-d).colRange(d, w-d);
//	Mat dist = (Mat::ones(pmid.rows, 1, CV_16U)).operator Mat();
	Mat dist = (Mat::ones(pmid.rows, pmid.cols, CV_16U)).operator Mat();
 /*	size_t step = pmid.step;
	const uchar* m = pmid.ptr();
	int minaxis=99999, oldm=minaxis;
	for( int i = 0; i < pmid.rows; i++ ) {
		for( int j = 0; j < pmid.cols; j++ )
		{
			int mem = ((int*)(m + i*step))[j];
			if (oldm>mem) {
				oldm = min(oldm, mem);
				minaxis = j;
			}
		}
	}
	dist = pmid.col(minaxis);
*/
	dist = pmid;
	Mat cmin;
	dist.convertTo(cmin, CV_32F);
	const uchar* m = cmin.ptr();
	size_t step = cmin.step;
	for( int i = 0; i < cmin.rows; i++ ) {
	for( int j = 0; j < cmin.cols; j++ ) {
		float mem = ((float*)(m+i*step))[j];
		mem /= d;
		((float*)(m+i*step))[j] = mem<1.0 ? mem : 1.0;
		if (mem<1.0) {
			mem=mem;
		}
	}
	}

	Mat vyx = im.rowRange(d, h-d).colRange(d, w-d);
	const uchar *mv = vyx.ptr();
	const uchar *mb = img_blur.ptr();
	for( int i = 0; i < vyx.rows; i++ ) {
		for( int j = 0; j < vyx.cols; j++ ) {
			float par = ((float*)(m+i*step))[j];
			if ( par<1.0 )
				((float*)(mv+i*vyx.step))[j] =
							((float*)(mv+i*vyx.step))[j]*par +
							((float*)(mb+i*vyx.step))[j]*(1-par);
		}
	}
	return vyx;
//	return im.rowRange(d, h-d).colRange(d, w-d);
//	return (img_blur);
//	return (im.rowRange(d, h-d).colRange(d, w-d)*cmin + img_blur*(1-0.9)).operator Mat();
}

 Mat motion_kernel(double angle, int d, int sz=65){
	if (d==0) d=1;
	Mat kern = (Mat::ones(1, d, CV_32F)).operator Mat();
	float c = cos(angle), s = sin(angle);
	Mat A = (Mat_<float>(2,3)<<c, -s, 0, s, c, 0);
	int sz2 = sz / 2;
	Mat B = A.rowRange(0, 2).colRange(0, 2);
	Mat C = (Mat_<float>(2,2)<< (d-1)*0.5, 0.0,0,0);
	Mat D = (B*C).operator Mat();
	float* ptrA = (float*)(A.data);
	float* ptrD = (float*)(D.data);
	ptrA[2] = sz2 - ptrD[0];
	ptrA[5] = sz2 - ptrD[2];
	warpAffine(kern, kern, A, Size(sz, sz), INTER_CUBIC);
	return kern;
}

Mat defocus_kernel(int d, int sz=65) {
	Mat kern = (Mat::zeros(sz, sz, CV_8U)).operator Mat();
	circle(kern, Size(sz, sz), d, 255, -1, LINE_AA, 1);
	Mat kern1;
	kern.convertTo(kern1, CV_32F);
	kern1 = kern1 / 255.0;
	return kern1;
}

void update(int ii, void* vv){
	Mat psf;
	float ang = deg2rad( getTrackbarPos("angle", win) );
	int d = getTrackbarPos("d", win);
	float noise = pow(10,(-0.1*getTrackbarPos("SNR (db)", win)));

	if (defocus)
		psf = defocus_kernel(d);
	else
		psf = motion_kernel(ang, d);
	imshow("psf", psf);

	int rows = psf.rows;
	size_t step = psf.step;
	const uchar* m = psf.ptr();
	float mmm=0;
	for( int i = 0; i < psf.cols*rows; i++ )
	{
		float s = ((float*)(m + i*sizeof(float)))[0];
		if (s!=0) {
			mmm += s;
		}
	}
	psf /= mmm;

	Mat psf_pad = Mat::zeros( img1.rows, img1.cols, img1.type() ).operator Mat();
	int kh = psf.rows, kw = psf.cols;
	Mat q0(psf_pad, Rect(0, 0, kw, kh));
	psf.copyTo(q0);

	Mat PSF;
	dft(psf_pad, PSF, DFT_COMPLEX_OUTPUT, kh);

	Mat PSF2;
	pow(PSF,2,PSF2);

	Mat dl = Mat::zeros(PSF2.rows, 1, PSF2.type()).operator Mat();
	const uchar* md = dl.data;
	const uchar* m2 = PSF2.data;
	step 			= dl.step;
	size_t step2 	= PSF2.step;
	for (int i = 0; i < PSF2.rows; i++) {
		for (int j = 0; j < PSF2.cols; j++) {
			float s  = ((float*)(m2 + i*step2))[j];
			float s1 = ((float*)(m2 + i*step2+sizeof(float)))[j];
//printf("|%.2f %.2f",s,s1);
			((float*)(md + i*step))[0] += s;
			((float*)(md + i*step+sizeof(float)))[0] += s1;
		}
//printf("\n\n");
	}

	Mat rr = (dl + noise).operator Mat();
	Mat onesy = Mat::ones(1, PSF.cols, CV_32FC2).operator Mat();
	step = onesy.step;
	m = onesy.data;
	for( int i = 0; i < onesy.rows; i++ ) {
		for( int j = 0; j < onesy.cols; j++ )
		{
			float s = ((float*)(m + i*step+j*8))[0];
//			((float*)(m + i*step+j*8))[0] = 1.0;
//			printf("|%.2f ",s);
			s = ((float*)(m + i*step+j*8))[1];
//			((float*)(m + i*step+j*8))[1] = 1.0;
//			printf("%.2f",s);
		}
//		printf("\n\n");
	}

	Mat iPSF;
	divide(PSF, rr * onesy, iPSF);
	Mat RES;
	mulSpectrums( IMG, iPSF, RES, 0);
	Mat res;
	idft(RES, res, DFT_SCALE | DFT_REAL_OUTPUT );

	res = res * 15;

//	res = roll(res, -kh/2, 0);
//	res = np.roll(res, -kw/2, 1);
	imshow(win, res);
}

static void help()
{
	cout << endl;
	cout << "Usage:" << endl;
	cout << "deconvolution  [--circle]" << endl;
	cout << "[--angle=<degrees>]" << endl;
	cout << "[--d=<diameter>]" << endl;
	cout << "[--snr=<signal/noise ratio in db>]" << endl;
	cout << "[<input image>]" << endl;
	cout << "Use sliders to adjust PSF paramitiers." << endl;
	cout << "Keys:" << endl;
	cout << "SPACE - switch btw linear/cirular PSF" << endl;
	cout << "ESC   - exit" << endl;
	cout << endl;
	cout << "(image source: http://www.topazlabs.com/infocus/_images/licenseplate_compare.jpg)" << endl;
	cout << "deconvolution --angle=135 --d=22  ../../sources/samples/data/licenseplate_motion.jpg" << endl;
	cout << "deconvolution --angle=86 --d=31  ../../sources/samples/data/text_motion.jpg" << endl;
	cout << "deconvolution --circle --d=19  ../../sources/samples/data/text_defocus.jpg" << endl;
	cout << endl;

}

#pragma argsused
int _tmain(int argc, _TCHAR* argv[])
{
	cv::CommandLineParser parser(argc, argv,
		"{circle||}"
		"{angle||}"
		"{d||}"
		"{snr||}"
		"{@filename||}"
	);
	help();

	String fn = parser.get<String>("@filename");
	bool circle = parser.has("circle");
	int d = 22;
	if (circle) {
		d = parser.get<int>("d");
	}
	int angle = 135;
	if (parser.has("angle")) {
		angle = parser.get<int>("angle");
    }
	int snr = 25;
	if (parser.has("snr")) {
		snr = parser.get<int>("snr");
	}
	if (!parser.check())
	{
		parser.printErrors();
		return 0;
	}
	if( fn.size() )
	{
		img = imread( fn, 0 );
		if( img.empty() )
		{
			cout << "Could not read " << fn.c_str() << endl;
			img = imread( "../../sources/samples/data/licenseplate_motion.jpg", 0 );
			if(img.empty()) cout << "Couldn't read ../../sources/samples/data/licenseplate_motion.jpg" << endl;
			return 0;
		}
	} else {
		img = imread( "../../sources/samples/data/licenseplate_motion.jpg", 0 );
		if(img.empty()) {
			cout << "Couldn't read ../../sources/samples/data/licenseplate_motion.jpg" << endl;
			return 0;
		}
	}

	defocus = circle;

	img.convertTo(img1, CV_32F);
	img1 = img1/255.0;
	imshow("input", img1);

	img1 = blur_edge(img1);
	dft(img1, IMG, DFT_COMPLEX_OUTPUT);

	namedWindow(win);

	namedWindow("psf", 0);
	createTrackbar("angle", win, &angle, 180, &update);
	createTrackbar("d", win, &d, 50, &update);
	createTrackbar("SNR (db)", win, &snr, 50, &update);
	update(0,NULL);

	while (true) {
		char ch = waitKey() & 0xFF;
		if (ch == 27)	break;
		if (ch == ' ') {
			defocus = ! defocus;
			update(0,NULL);
		}
	}
}
