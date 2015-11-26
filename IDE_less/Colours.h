#ifdef _WIN64
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#else
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#endif

#define RED_RANGE1 cv::Scalar(0, 80, 60)
#define RED_RANGE2 cv::Scalar(18, 255, 255)
#define RED_RANGE3 cv::Scalar(160, 80, 60)
#define RED_RANGE4 cv::Scalar(179, 255, 255)

#define YELLOW_RANGE1 cv::Scalar(15, 25, 25)
#define YELLOW_RANGE2 cv::Scalar(32, 250, 255)

using namespace cv;

//old
Mat look_for_red(const Mat &I);
Mat look_for_yellow(const Mat &I);

//new
Mat check_red_range(const Mat& in);
Mat check_yellow_range(const Mat& in);

//override
UMat check_red_range(const UMat& in);
UMat check_yellow_range(const UMat& in);


