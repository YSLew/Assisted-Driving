#include <iostream>
#include <cassert>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>

using namespace cv;
using namespace std;

// I ist BGR Bild, O is grauwert
Mat my_convert(const Mat &I)
{
	assert(I.channels() == 3);

	int nRows = I.rows;
	int nCols = I.cols;

	if (I.isContinuous())
	{
		nCols *= nRows;
		nRows = 1;
	}

	Mat O = Mat(I.size(), CV_8UC1); // 

	int i, j;
	const uchar* p;	
	uchar* q;
	for (i = 0; i < nRows; ++i)
	{
		p = I.ptr<uchar>(i);
		q = O.ptr<uchar>(i);
		for (j = 0; j < nCols; ++j)
		{
			int b = p[3 * j + 0];
			int g = p[3 * j + 1];
			int r = p[3 * j + 2];

			if (r > 180 && g < 100)
				q[j] = 255;
			else
				q[j] = 0;
		}
	}
	return O;
}

int main()
{
	Mat I;
	Mat O;

	I = imread("hs.png");
	O = my_convert(I);

	imshow("image", O);
	waitKey(0);

	return 0;
}
