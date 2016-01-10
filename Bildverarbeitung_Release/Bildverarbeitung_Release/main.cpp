/** main file (.cpp)
*
* Contains main function, globals, flags.
* Algortihms are used from:
* http://opencv-code.com/tutorials/detecting-simple-shapes-in-an-image/
* http://www.codeproject.com/Articles/850023/Color-Based-Object-Detection
*
∗ @author Max Wahl, Oleg Tydynyan, Robert Ledwig
*/

//////////////////////////////////////////////////////////////////////////////////////
//HEADER
//////////////////////////////////////////////////////////////////////////////////////

// header for other used files
#include "Colours.h"
#include "Geometry.h"
#include "Shapes.h"

// header for used system libraries
#include <iostream>
#include <omp.h>
#include <stdio.h>

// header for used opencv libraries
#ifdef _WIN64
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

// for unix systems paths may variate
#else
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#endif

//////////////////////////////////////////////////////////////////////////////////////
//NAMESPACES
//////////////////////////////////////////////////////////////////////////////////////

using namespace cv;

//////////////////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////////////////

// approximation values for shape detection. Used by fuction find_shapes.
#define RED_APPROX 0.012
#define YELLOW_APPROX 0.028

//////////////////////////////////////////////////////////////////////////////////////
//FLAGS
//////////////////////////////////////////////////////////////////////////////////////

//use webcam
#define CAMERA

//show live output images
#define OUTPUT

//show live output debug images
//#define DEBUG_OUTPUT

//use GPU support
//#define GPU

//use MULTICORE SUPPORT
//#define MULTICORE

//atm: no gpu and multicore at the same time

//////////////////////////////////////////////////////////////////////////////////////
//CLASSES (INLINE)
//////////////////////////////////////////////////////////////////////////////////////

class Parallel_process : public cv::ParallelLoopBody
{

private:
	cv::Mat img;
	cv::Mat& retVal;
	void(*func_to_run)(const Mat&, const Mat&);
	int size;
	int diff;

public:
	Parallel_process(cv::Mat inputImage, cv::Mat& outImage, void(*func)(const Mat&, const Mat&),
		int sizeVal, int diffVal)
		: img(inputImage), retVal(outImage), func_to_run(func),
		size(sizeVal), diff(diffVal){}

	virtual void operator()(const cv::Range& range) const
	{
		for (int i = range.start; i < range.end; i++)
		{
			/* divide image in 'diff' number
			of parts and process simultaneously */

			cv::Mat in(img, cv::Rect(0, (img.rows / diff)*i,
				img.cols, img.rows / diff));
			cv::Mat out(retVal, cv::Rect(0, (retVal.rows / diff)*i,
				retVal.cols, retVal.rows / diff));

			func_to_run(in, out);
			//check_red_range(in, out);

		}
	}
};

//////////////////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////////

/** Main function for traffic sign regognition
*
* Main routine, start of program. Contains Loop, which will be performed one time per image.
*
* @param argc - number of arguments, argv - array of arguments. First argument can be used image.
*  This image will be used, if it is defined, otherwise, a static image or the camera will be used.
∗ @return Always 0.
∗ @author Max Wahl, Oleg Tydynyan, Robert Ledwig
*/

int main(int argc, char *argv[])
{
	int Wait_Time = 10;
	int num = 0;
	std::cout << "Hello OpenCV" << std::endl;

	#ifdef CAMERA
		//use webcam (first video device in system)
		cv::VideoCapture videoCapture(0); 

		//alternative: use a static video
		//cv::VideoCapture videoCapture("Vorfahrt2.mp4");

		//set size of read images
		videoCapture.set(CAP_PROP_FRAME_WIDTH, 1920);
		videoCapture.set(cv::CAP_PROP_FRAME_HEIGHT, 1080);
	#endif;

	cv::Mat input_Mat;

	#ifndef GPU	
		cv::Mat input;
		cv::Mat input_image;
		cv::Mat input_image_red;
		cv::Mat input_image_yellow;
		cv::Mat output_image;

		cv::Mat bw_red;
		cv::Mat bw_yellow;
		Mat dst, dst_y;

		cv::Mat hsv_image;
	#else
		cv::UMat input;
		cv::UMat input_image;
		cv::UMat input_image_red;
		cv::UMat input_image_yellow;
		cv::UMat output_image;

		cv::UMat bw_red;
		cv::UMat bw_yellow;
		cv::UMat dst, dst_y;
	#endif

	//structures for floating average 
	tFloatAvgFilter stop_avg;
	tFloatAvgFilter vf_gw_avg;
	tFloatAvgFilter vf_str_avg;

	InitFloatAvg(&stop_avg, false);
	InitFloatAvg(&vf_gw_avg, false);
	InitFloatAvg(&vf_str_avg, false);

	sign_struct traffics_signs = { stop_avg, vf_gw_avg, vf_str_avg };

	while (1)
	{
		//start global system timer
		double t = (double)getTickCount();

		#ifdef CAMERA
			videoCapture >> input;
		#else 
			if (argc == 1)
			{
				// in case of no given arguments: take a fixed image
				imread("Viereck_7.JPG").copyTo(input);
			}
			else
				// in case of given arguments: take the name of the given picture
				imread(argv[1]).copyTo(input);
		#endif;	

		input_image = input.clone();


		#ifdef MULTICORE

		int thread_count = 8;
		cv::setNumThreads(thread_count);

		// create 8 threads and use TBB

		void(*cyr)(const Mat&, const Mat&) = check_yellow_range;
		void(*crr)(const Mat&, const Mat&) = check_red_range;

		cv::Mat out = cv::Mat::zeros(input_image.size(), CV_8UC1);
		cv::Mat out2 = cv::Mat::zeros(input_image.size(), CV_8UC1);

		cv::parallel_for_(cv::Range(0, thread_count), Parallel_process(input_image, out, crr, 5, thread_count));
		input_image_red = out;

		cv::parallel_for_(cv::Range(0, thread_count), Parallel_process(input_image, out2, cyr, 5, thread_count));
		input_image_yellow = out2;

		#else		
		
		//fill empty image
		input_image_red = cv::Mat::zeros(input_image.size(), CV_8UC1);
		input_image_yellow = cv::Mat::zeros(input_image.size(), CV_8UC1);

		//colour filter
		check_red_range(input_image, input_image_red);
		check_yellow_range(input_image, input_image_yellow);
		#endif

		if (argc == 1)
		{
		#ifdef DEBUG_OUTPUT
				imshow("Nach Gelb-Threshold", input_image_yellow);
				imshow("Nach Rot-Threshold", input_image_red);
		#endif
		}

		//Gaussian filter can help: not used at the moment
		//GaussianBlur(input_image, input_image, Size(1, 1), 0, 0);

		//detect edges
		//used algorithm: 
		//http://docs.opencv.org/modules/imgproc/doc/feature_detection.html?highlight=canny#canny 
		cv::Canny(input_image_red, bw_red, 0, 50, 5, true); 
		

		//Experiments showed: canny works bad for yellow contours. Disabled at the moment.
		//cv::Canny(input_image_yellow, bw_yellow, 0, 50, 5, true);
		bw_yellow = input_image_yellow;
		//only necessary if canny is not used
		bw_yellow.convertTo(bw_yellow, CV_8UC1);

		//find contours
		dst = find_shapes(bw_red, input, RED, RED_APPROX, &traffics_signs); 
		dst_y = find_shapes(bw_yellow, dst, YELLOW, YELLOW_APPROX, &traffics_signs);

		if (argc == 1)
		{
		#ifdef DEBUG_OUTPUT
				imshow("BBW", bw_red);
				cv::imshow("src", input);
				cv::imshow("dst", dst);
				cv::imshow("bw", bw_red);
				cv::imshow("bw_y", bw_yellow);
		#endif

		#ifdef OUTPUT
				cv::imshow("dst_y", dst_y);
				cv::imshow("bw_y", bw_yellow);
		#endif
		}

		#ifndef CAMERA
				if (argc != 1) 
				{
					std::string Str_Input = argv[1];
					std::string result_y = Str_Input + "_tested.png";
					std::cout << result_y << std::endl;
					cv::imwrite(result_y, dst_y);
					//if arguments were given, the program finishes here.
					return 0;
				}
		#endif

		int c = cv::waitKey(Wait_Time); 

		// input detection
		switch (c)
		{
			case 'q':   return 0;
			case 'f':	Wait_Time = 1; break;
			case 'l':	Wait_Time = 500; break;
			case 'e':	Wait_Time = 0; break;
			case 's':
			{
						std::string mystring = "Testfile_" + num;
						mystring = mystring + ".png";
						cv::imwrite(mystring, bw_yellow);
						num++;
						break;
			}
		}

		// stop intern system tímer
		t = ((double)getTickCount() - t) / getTickFrequency();

		printf("\rTime per Frame: %0.3f STOP: %d VF_GW: %d VF_STR: %d", t,
			CheckAVG(&(&traffics_signs)->stop), CheckAVG(&(&traffics_signs)->vf_gw), CheckAVG(&(&traffics_signs)->vf_str));
	}

	return 0;
}