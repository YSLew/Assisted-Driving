//Sources:

//http://opencv-code.com/tutorials/detecting-simple-shapes-in-an-image/
//http://www.codeproject.com/Articles/850023/Color-Based-Object-Detection

//own header
#include "Colours.h"
#include "Geometry.h"
#include "Shapes.h"

#include <iostream>
#include <omp.h>
#include <stdio.h>

#ifdef _WIN64
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
//#include <opencv2/core/ocl.hpp>
#else
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#endif
//#include <opencv2/core/ocl.hpp>
//#include <opencv2/videoio.hpp>


//#include <opencv2/imgproc/imgproc.hpp>

//für Feature-Tracking
//#include <opencv2/features2d.hpp>
//#include <opencv2/xfeatures2d.hpp>

//#include <opencv2/calib3d.hpp>

using namespace cv;
//using namespace xfeatures2d;


/*
cv::inRange(hsv_image, cv::Scalar(0, 80, 60), cv::Scalar(18, 255, 255), lower_red_hue_range);
cv::inRange(hsv_image, cv::Scalar(160, 80, 60), cv::Scalar(179, 255, 255), upper_red_hue_range); //HUE betweeen 0 and 179
cv::inRange(hsv_image, cv::Scalar(15, 25, 25), cv::Scalar(32, 250, 255), out); //14,60,40 ... 32, 255, 255
*/


#define RED_APPROX 0.012
#define YELLOW_APPROX 0.028


//FLAGS

#define CAMERA
#define OUTPUT
//#define DEBUG_OUTPUT
#define GPU



//global functions


//draw a box
//CvRect box = cvRect(50, 50, 100, 100);
//bool drawing_box = false;

//cv::Mat global_box;

// Implement mouse callback
/*
void my_mouse_callback(int event, int x, int y, int flags, void* param){

	switch (event){
	case CV_EVENT_MOUSEMOVE:
		if (drawing_box){
			box.width = x - box.x;
			box.height = y - box.y;
		}
		break;

	case CV_EVENT_LBUTTONDOWN:
		drawing_box = true;
		box = cvRect(x, y, 0, 0);
		break;

	case CV_EVENT_LBUTTONUP:
		drawing_box = false;
		if (box.width < 0){
			box.x += box.width;
			box.width *= -1;
		}
		if (box.height < 0){
			box.y += box.height;
			box.height *= -1;
		}

		std::cout << x << std::endl;
		std::cout << y << std::endl;
		std::cout << box.width << std::endl;
		std::cout << box.height << std::endl;

	}
}
*/
///////////////


int main(int argc, char *argv[])
{
	int Wait_Time = 10;
	int num = 0;
	std::cout << "Hello OpenCV" << std::endl;

	//für Webcam!
#ifdef CAMERA

	cv::VideoCapture videoCapture(0); //interne Wiedergabe der 1. Quelle (Webcam)!
	//cv::VideoCapture videoCapture("http://docs.gstreamer.com/media/sintel_cropped_multilingual.webm");
	//alternative: festes Video!
	//cv::VideoCapture videoCapture("Vorfahrt2.mp4");

	videoCapture.set(CAP_PROP_FRAME_WIDTH, 1920);
	videoCapture.set(cv::CAP_PROP_FRAME_HEIGHT, 1080);

	//int w = videoCapture.get(CAP_PROP_FRAME_WIDTH);
	//int h = videoCapture.get(cv::CAP_PROP_FRAME_HEIGHT);

	/*bool ok = videoCapture.open(0);
	if (!ok)
	{
	std::cerr << "no camera found, exit" << std::endl;
	return  -1;
	}*/
#endif;



	bool new_obj = true;
	bool new_match = true;

	//mouse callback
	//namedWindow("Input", WINDOW_AUTOSIZE);
	//namedWindow("Output", WINDOW_AUTOSIZE);

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


	// Convert input image to HSV
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

	Rect region_of_interest2;
	// Threshold the HSV image, keep only the red pixels

	volatile int counter = 0;
	volatile double mittel = 0;
	volatile int tsum = 0;

	//Strukturen fuer gleitenden Mittelwert
	tFloatAvgFilter stop_avg;
	tFloatAvgFilter vf_gw_avg;
	tFloatAvgFilter vf_str_avg;

	InitFloatAvg(&stop_avg, false);
	InitFloatAvg(&vf_gw_avg, false);
	InitFloatAvg(&vf_str_avg, false);


	sign_struct traffics_signs = { stop_avg, vf_gw_avg, vf_str_avg };

	while (1)
	{

		double t = (double)getTickCount();
		// do something ...
		

#ifdef CAMERA

			videoCapture >> input;


#else 
		if (argc == 1)
		{
			//input = imread("Dreieck_Scene_2.JPG");
			//input = imread("Dreieck_93.JPG");
			//input = imread("Dreieck_3.JPG");

			//input = imread("Testbild_Color.png"); 
			imread("Viereck_7.JPG").copyTo(input);
			//input = imread("Vorfahrtsfehler/Viereck_90.JPG");

			//input = imread("STOP_Scene_e.jpg");
			//look_for_red(input);
		}
		else
			imread(argv[1]).copyTo(input);
#endif;	


		input_image = input.clone();

		//Farbfilterung
		input_image_red = check_red_range(input_image);
		input_image_yellow = check_yellow_range(input_image);

		if (argc == 1)
		{
#ifdef DEBUG_OUTPUT
			imshow("Nach Gelb-Threshold", input_image_yellow);
			imshow("Nach Rot-Threshold", input_image_red);
#endif
		}


		//GAUSS-Filter

		//offenbar besser ohne!
		//GaussianBlur(input_image, input_image, Size(1, 1), 0, 0);

		//Kantenerkennung
		cv::Canny(input_image_red, bw_red, 0, 50, 5, true); //true= more accurate filter, 3 better than 5? found here:
		//http://docs.opencv.org/modules/imgproc/doc/feature_detection.html?highlight=canny#canny
		//#endif


		//kantenerkennung bei Gelb eher kontraproduktiv. klug?
		//cv::Canny(input_image_yellow, bw_yellow, 0, 50, 5, true);
		bw_yellow = input_image_yellow;

		//find contours
		//if (new_obj == true)
		//{
		dst = find_shapes(bw_red, input, RED, RED_APPROX, &traffics_signs); //bei rot ggf schwächer approximieren
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
		//}


		//for mouse-control!
		//cvSetMouseCallback("Orginal", my_mouse_callback, 0);

		new_obj = false;
		new_match = false;

#ifndef CAMERA
		if (argc != 1) //Konsolenvariante muss hier enden + Ausgabe
		{
			std::string Str_Input = argv[1];
			std::string result_y = Str_Input + "_tested.png";
			//std::string result = Str_Input + "_tested.png";
			std::cout << result_y << std::endl;
			cv::imwrite(result_y, dst_y);
			//cv::imwrite(result, dst);
			return 0;
		}
#endif

		int c = cv::waitKey(Wait_Time); //entspricht "auffrischen" des Fensters: fragt nachrichten ab, erlaubt OS das Auffrischen des Fensters dazwischen

		switch (c)
		{
		case 'q':	/*videoWriter.release();*/  return 0;
		case 'f':	Wait_Time = 1; break;
		case 'l':	Wait_Time = 500; break;
		case 'e':	Wait_Time = 0; break;
		case 's':
		{
					std::string mystring = "Testfile_" + num;
					mystring = mystring + ".png";
					cv::imwrite(mystring, bw_yellow);
					num++;
					new_obj = true;
					break;
		}
		case 'm':
		{
					new_match = true;
					break;
		}
		}
		t = ((double)getTickCount() - t) / getTickFrequency();

		
		/*if (counter > 10)
		{
			counter = 1;
			tsum = 0;
		}*/
		//printf("\rMittel: %0.3f  STOP: %0.3f  FV_GW: %0.3f  FW_STR: %0.3f", mittel,t,tsum,counter);

		printf("\rTime per Frame: %0.3f STOP: %d VF_GW: %d VF_STR: %d", t,
			CheckAVG(&(&traffics_signs)->stop), CheckAVG(&(&traffics_signs)->vf_gw), CheckAVG(&(&traffics_signs)->vf_str));

	}

	return 0;
}