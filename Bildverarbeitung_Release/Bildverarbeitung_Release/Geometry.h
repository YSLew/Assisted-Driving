#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>


using namespace cv;


int triangle_check(cv::Point pt0, cv::Point pt1, cv::Point pt2);
int rectangle_check(cv::Point pt0, cv::Point pt1, cv::Point pt2, cv::Point pt3);

/**
* Helper function to find a cosine of angle between vectors
* from pt0->pt1 and pt0->pt2
*found here:
*/


