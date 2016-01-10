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



class Parallel_process : public cv::ParallelLoopBody
{

private:
	cv::Mat img;
	cv::Mat& retVal;
	int size;
	int diff;

public:
	Parallel_process(cv::Mat inputImgage, cv::Mat& outImage,
		int sizeVal, int diffVal)
		: img(inputImgage), retVal(outImage),
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
			cv::GaussianBlur(in, out, cv::Size(size, size), 0);
			cv::cvtColor(out, out, cv::COLOR_BGR2HSV);
			cv::cvtColor(out, in, cv::COLOR_HSV2BGR);
			cv::cvtColor(out, out, cv::COLOR_BGR2HSV);
			cv::cvtColor(out, in, cv::COLOR_HSV2BGR);
		}
	}
};

int main(int argc, char* argv[])
{
	
	

	cv::Mat img, out;
	img = cv::imread("Viereck_7.JPG");
	out = cv::Mat::zeros(img.size(), CV_8UC3);

	int thread_count = 4;
	cv::setNumThreads(thread_count);

	while (1)
	{
		double t = (double)getTickCount();
		img = cv::imread("IMG_2825.JPG");
		out = cv::Mat::zeros(img.size(), CV_8UC3);
		// create 8 threads and use TBB
		cv::parallel_for_(cv::Range(0, thread_count), Parallel_process(img, out, 5, thread_count));

		cv::imshow("image", img);
		cv::imshow("blur", out);
		t = ((double)getTickCount() - t) / getTickFrequency();
		printf("\rTime: %0.4f (%d)", t, cv::getNumThreads());
		cv::waitKey(1);

	}

	return(1);
}