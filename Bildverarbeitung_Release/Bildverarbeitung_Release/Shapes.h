#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#include "Geometry.h"
#include "Floating_Average.h"

#define MIN_SIZE 1000
//defines
#define RED 0
#define YELLOW 1


typedef struct
{
	tFloatAvgFilter stop;
	tFloatAvgFilter	vf_gw;
	tFloatAvgFilter	vf_str;

} sign_struct;

typedef struct
{
	bool bstop;
	bool b_vf_gw;
	bool b_vf_str;
} str_found_signs_this_round;

using namespace cv;

Mat find_shapes(const Mat& in, const Mat& original, int colour, float approx_factor, sign_struct * a_sing_struct);
void setLabel(cv::Mat& im, const std::string label, std::vector<cv::Point>& contour);

UMat find_shapes(const UMat& in, const UMat& original, int colour, float approx_factor, sign_struct * a_sing_struct);
void setLabel(cv::UMat& im, const std::string label, std::vector<cv::Point>& contour);



// Why does a static function has to be declared inline ?
static double angle(cv::Point pt1, cv::Point pt2, cv::Point pt0)
{
	double dx1 = pt1.x - pt0.x;
	double dy1 = pt1.y - pt0.y;
	double dx2 = pt2.x - pt0.x;
	double dy2 = pt2.y - pt0.y;
	return (dx1*dx2 + dy1*dy2) / sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}