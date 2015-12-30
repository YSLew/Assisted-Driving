/** Header-File for shape detection (.h)
*
* Contains functions for shape detection and traffic sign recognition.
*
∗ @author Max Wahl, Oleg Tydynyan, Robert Ledwig
*/

//////////////////////////////////////////////////////////////////////////////////////
//HEADER
//////////////////////////////////////////////////////////////////////////////////////

#ifdef _WIN64
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#else
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#endif

#include "Geometry.h"
#include "Floating_Average.h"

//////////////////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////////////////

#define MIN_SIZE 1000
//defines
#define RED 0
#define YELLOW 1

//////////////////////////////////////////////////////////////////////////////////////
//TYPEDEF
//////////////////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////////////////
//NAMESPACES
//////////////////////////////////////////////////////////////////////////////////////

using namespace cv;

//////////////////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////////

Mat find_shapes(const Mat& in, const Mat& original, int colour, float approx_factor, sign_struct * a_sing_struct);
void setLabel(cv::Mat& im, const std::string label, std::vector<cv::Point>& contour);

UMat find_shapes(const UMat& in, const UMat& original, int colour, float approx_factor, sign_struct * a_sing_struct);
void setLabel(cv::UMat& im, const std::string label, std::vector<cv::Point>& contour);

/** Calculates angle
*
* Cosinus angle value will be calculated from given points.
* This function is static: it has to be declared inline.
*
* @param Point  pt1, pt2, pt3 - given Points
∗ @return double - cosinus angle value
∗ @author Max Wahl, Oleg Tydynyan, Robert Ledwig
*/
static double angle(cv::Point pt1, cv::Point pt2, cv::Point pt0)
{
	double dx1 = pt1.x - pt0.x;
	double dy1 = pt1.y - pt0.y;
	double dx2 = pt2.x - pt0.x;
	double dy2 = pt2.y - pt0.y;
	return (dx1*dx2 + dy1*dy2) / sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}