/** File for colour detection (.cpp)
*
* Contains functions for colour detection and separation.
*
∗ @author Max Wahl, Oleg Tydynyan, Robert Ledwig
*/

//////////////////////////////////////////////////////////////////////////////////////
//HEADER
//////////////////////////////////////////////////////////////////////////////////////

#include "Colours.h"


//////////////////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////////

/** Find red areals (Mat)
*
* Uses opencv-function inrange() to detect red areals. These will be set to white.
*
* @param in - opencv Mat-object
∗ @return manipulated Mat object.
∗ @author Max Wahl, Oleg Tydynyan, Robert Ledwig
*/
void check_red_range(const Mat& in, const Mat& out)
{
	cv::Mat hsv_image;
	//cv::Mat out;
	cv::cvtColor(in, hsv_image, cv::COLOR_BGR2HSV);
	cv::Mat lower_red_hue_range;
	cv::Mat upper_red_hue_range;

	// in hsv red is splitted in two areals. This function checks both of them...
	cv::inRange(hsv_image, RED_RANGE1, RED_RANGE2, lower_red_hue_range);
	cv::inRange(hsv_image, RED_RANGE3, RED_RANGE4, upper_red_hue_range); 

	//...and merges them.
	cv::addWeighted(lower_red_hue_range, 1.0, upper_red_hue_range, 1.0, 0.0, out);

	//return out;
}

/** Find red areals (UMat)
*
* Uses opencv-function inrange() to detect red areals. These will be set to white.
*
* @param in - opencv UMat-object
∗ @return manipulated UMat object.
∗ @author Max Wahl, Oleg Tydynyan, Robert Ledwig
*/
void check_red_range(const UMat& in, const UMat& out)
{
	cv::UMat hsv_image;
	//cv::Mat out;
	cv::cvtColor(in, hsv_image, cv::COLOR_BGR2HSV);
	cv::UMat lower_red_hue_range;
	cv::UMat upper_red_hue_range;

	// in hsv red is splitted in two areals. This function checks both of them...
	cv::inRange(hsv_image, RED_RANGE1, RED_RANGE2, lower_red_hue_range);
	cv::inRange(hsv_image, RED_RANGE3, RED_RANGE4, upper_red_hue_range);

	//...and merges them.
	cv::addWeighted(lower_red_hue_range, 1.0, upper_red_hue_range, 1.0, 0.0, out);

	//return out;
}

//only debug
Mat check_red_range(const Mat& in)
{
	cv::Mat hsv_image;
	cv::Mat out;
	cv::cvtColor(in, hsv_image, cv::COLOR_BGR2HSV);
	cv::Mat lower_red_hue_range;
	cv::Mat upper_red_hue_range;

	// in hsv red is splitted in two areals. This function checks both of them...
	cv::inRange(hsv_image, RED_RANGE1, RED_RANGE2, lower_red_hue_range);
	cv::inRange(hsv_image, RED_RANGE3, RED_RANGE4, upper_red_hue_range);

	//...and merges them.
	cv::addWeighted(lower_red_hue_range, 1.0, upper_red_hue_range, 1.0, 0.0, out);

	return out;
}

/** Find red areals (UMat)
*
* Uses opencv-function inrange() to detect red areals. These will be set to white.
* This function uses the GPU.
*
* @param in - opencv UMat-object
∗ @return manipulated UMat object.
∗ @author Max Wahl, Oleg Tydynyan, Robert Ledwig
*/
UMat check_red_range(const UMat& in)
{
	cv::UMat hsv_image;
	cv::UMat out;
	cv::cvtColor(in, hsv_image, cv::COLOR_BGR2HSV);
	cv::UMat lower_red_hue_range;
	cv::UMat upper_red_hue_range;

	// in hsv red is splitted in two areals. This function checks both of them...
	cv::inRange(hsv_image, RED_RANGE1, RED_RANGE2, lower_red_hue_range);
	cv::inRange(hsv_image, RED_RANGE3, RED_RANGE4, upper_red_hue_range); 

	//...and merges them.
	cv::addWeighted(lower_red_hue_range, 1.0, upper_red_hue_range, 1.0, 0.0, out);

	return out;
}

/** Find yellow areals (Mat)
*
* Uses opencv-function inrange() to detect yellow areals. These will be set to white.
*
* @param in - opencv Mat-object
∗ @return manipulated Mat object.
∗ @author Max Wahl, Oleg Tydynyan, Robert Ledwig
*/
void check_yellow_range(const Mat& in, const Mat& out)
{
	cv::Mat hsv_image;
	cv::cvtColor(in, hsv_image, cv::COLOR_BGR2HSV);
	cv::inRange(hsv_image, YELLOW_RANGE1, YELLOW_RANGE2, out); 

}

/** Find yellow areals (UMat)
*
* Uses opencv-function inrange() to detect yellow areals. These will be set to white.
*
* @param in - opencv UMat-object
∗ @return manipulated UMat object.
∗ @author Max Wahl, Oleg Tydynyan, Robert Ledwig
*/
void check_yellow_range(const UMat& in, const UMat& out)
{
	cv::UMat hsv_image;
	cv::cvtColor(in, hsv_image, cv::COLOR_BGR2HSV);
	cv::inRange(hsv_image, YELLOW_RANGE1, YELLOW_RANGE2, out);

}

/** Find yellow areals (UMat)
*
* Uses opencv-function inrange() to detect yellow areals. These will be set to white.
* This function uses the GPU.
*
* @param in - opencv UMat-object
∗ @return manipulated UMat object.
∗ @author Max Wahl, Oleg Tydynyan, Robert Ledwig
*/
UMat check_yellow_range(const UMat& in)
{
	cv::UMat hsv_image;
	cv::UMat out;
	cv::cvtColor(in, hsv_image, cv::COLOR_BGR2HSV);
	cv::inRange(hsv_image, YELLOW_RANGE1, YELLOW_RANGE2, out); 

	return out;

}

