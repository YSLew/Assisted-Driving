#include "Colours.h"

using namespace cv;


Mat look_for_red(const Mat &I)
{
	int channels = I.channels();
	//printf("%d", channels);
	assert(I.channels() == 3);

	int nRows = I.rows;
	int nCols = I.cols;

	if (I.isContinuous())
	{
		nCols *= nRows;
		nRows = 1;
	}

	Mat O = Mat(I.size(), CV_8UC1); //Ausgabematrix initialisieren 

	int i, j;
	const uchar* p;
	uchar* q;

	for (i = 0; i < nRows; ++i)
	{
		p = I.ptr<uchar>(i); //Pointer anlegen, der auf Anfang der Matrix zeigt
		q = O.ptr<uchar>(i);
		for (j = 0; j < nCols; ++j)
		{
			int b = p[3 * j + 0];
			int g = p[3 * j + 1];
			int r = p[3 * j + 2];

			//if (r > 110 && g < 90 && b < 90) //nur rote Elemente auf Wei� setzen
			//q[j] = 255;
			//else
			//q[j] = 0;

			q[j] = r / 2 - g / 2 - b / 2 + 128;
		}
	}
	return O;
}


Mat look_for_yellow(const Mat &I)
{
	int channels = I.channels();
	//printf("%d", channels);
	assert(I.channels() == 3);

	int nRows = I.rows;
	int nCols = I.cols;

	if (I.isContinuous())
	{
		nCols *= nRows;
		nRows = 1;
	}

	Mat O = Mat(I.size(), CV_8UC1); //Ausgabematrix initialisieren 

	int i, j;
	const uchar* p;
	uchar* q;

	for (i = 0; i < nRows; ++i)
	{
		p = I.ptr<uchar>(i); //Pointer anlegen, der auf Anfang der Matrix zeigt
		q = O.ptr<uchar>(i);
		for (j = 0; j < nCols; ++j)
		{
			int b = p[3 * j + 0];
			int g = p[3 * j + 1];
			int r = p[3 * j + 2];

			//if (r > 110 && g < 90 && b < 90) //nur rote Elemente auf Wei� setzen
			//q[j] = 255;
			//else
			//q[j] = 0;

			q[j] = g / 2 - r / 2 - b / 2 + 128;
		}
	}
	return O;
}


/**
* Helper function to find red areals
*/
Mat check_red_range(const Mat& in)
{
	cv::Mat hsv_image;
	cv::Mat out;
	cv::cvtColor(in, hsv_image, cv::COLOR_BGR2HSV);
	cv::Mat lower_red_hue_range;
	cv::Mat upper_red_hue_range;

	//original: (0,100,100) bis (10, 255, 255); (160,100,100) bis (179, 255, 255),
	//testwerte:
	cv::inRange(hsv_image, RED_RANGE1, RED_RANGE2, lower_red_hue_range);
	cv::inRange(hsv_image, RED_RANGE3, RED_RANGE4, upper_red_hue_range); //HUE betweeen 0 and 179
	cv::Mat red_hue_image;
	cv::addWeighted(lower_red_hue_range, 1.0, upper_red_hue_range, 1.0, 0.0, out);

	return out;

}

/**
* Helper function to find red areals (optimized for UMat)
*/
UMat check_red_range(const UMat& in)
{
	cv::UMat hsv_image;
	cv::UMat out;
	cv::cvtColor(in, hsv_image, cv::COLOR_BGR2HSV);
	cv::UMat lower_red_hue_range;
	cv::UMat upper_red_hue_range;

	//original: (0,100,100) bis (10, 255, 255); (160,100,100) bis (179, 255, 255),
	//testwerte:
	cv::inRange(hsv_image, RED_RANGE1, RED_RANGE2, lower_red_hue_range);
	cv::inRange(hsv_image, RED_RANGE3, RED_RANGE4, upper_red_hue_range); //HUE betweeen 0 and 179
	cv::UMat red_hue_image;
	cv::addWeighted(lower_red_hue_range, 1.0, upper_red_hue_range, 1.0, 0.0, out);

	return out;

}

/**
* Helper function to find yellow areals
*/
Mat check_yellow_range(const Mat& in)
{
	cv::Mat hsv_image;
	cv::Mat out;
	cv::cvtColor(in, hsv_image, cv::COLOR_BGR2HSV);
	cv::inRange(hsv_image, YELLOW_RANGE1, YELLOW_RANGE2, out); //14,60,40 ... 32, 255, 255
	//invertierte Werte für Value ggü Paint! (Value= 255 =>  minimale Helligkeit, Saturation = 255 => max. Sättigung!)

	return out;

}

/**
* Helper function to find yellow areals (optimized for UMat)
*/
UMat check_yellow_range(const UMat& in)
{
	cv::UMat hsv_image;
	cv::UMat out;
	cv::cvtColor(in, hsv_image, cv::COLOR_BGR2HSV);
	cv::inRange(hsv_image, YELLOW_RANGE1, YELLOW_RANGE2, out); //14,60,40 ... 32, 255, 255
	//invertierte Werte für Value ggü Paint! (Value= 255 =>  minimale Helligkeit, Saturation = 255 => max. Sättigung!)

	return out;

}

