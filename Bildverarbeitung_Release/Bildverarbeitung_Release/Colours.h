/** Header-File for colour detection (.h)
*
* Contains functions for colour detection and separation.
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

//////////////////////////////////////////////////////////////////////////////////////
//DEFINES
//////////////////////////////////////////////////////////////////////////////////////

// need to know:
// HSV: Hue, Saturation, Value
// Hue: 0 .. 179 (Farbton)
// Saturation: 0 .. 255 (Sättigung)
// Value: 0 .. 255 (Helligkeit)

// attention: value = 255: minimum value, Saturation = 255: maximum saturation
// in paint it's different!

// OpenCV_Value=PAINT_Value * 0,75 (only for HUE!)

#define RED_RANGE1 cv::Scalar(0, 80, 60)
#define RED_RANGE2 cv::Scalar(18, 255, 255)
#define RED_RANGE3 cv::Scalar(160, 80, 60)
#define RED_RANGE4 cv::Scalar(179, 255, 255)

#define YELLOW_RANGE1 cv::Scalar(15, 25, 25)
#define YELLOW_RANGE2 cv::Scalar(32, 250, 255)

//////////////////////////////////////////////////////////////////////////////////////
//NAMESPACES
//////////////////////////////////////////////////////////////////////////////////////

using namespace cv;

//////////////////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////////

Mat check_red_range(const Mat& in);
Mat check_yellow_range(const Mat& in);

UMat check_red_range(const UMat& in);
UMat check_yellow_range(const UMat& in);


