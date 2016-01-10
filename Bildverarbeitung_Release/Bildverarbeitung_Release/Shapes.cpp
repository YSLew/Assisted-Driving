/** File for shape detection (.cpp)
*
* Contains functions for shape detection and traffic sign recognition.
*
∗ @author Max Wahl, Oleg Tydynyan, Robert Ledwig
*/

//////////////////////////////////////////////////////////////////////////////////////
//HEADER
//////////////////////////////////////////////////////////////////////////////////////

#include "Shapes.h"

//////////////////////////////////////////////////////////////////////////////////////
//FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////////

/** type2str
*
* Shows type of Mat variable
*
* @param type - type of Mat variable (Mat.type())
∗ @return String - type of Mat as String
∗ @copied from: http://stackoverflow.com/questions/10167534/how-to-find-
* out-what-type-of-a-mat-object-is-with-mattype-in-opencv
*/
String type2str(int type) {
	String r;

	uchar depth = type & CV_MAT_DEPTH_MASK;
	uchar chans = 1 + (type >> CV_CN_SHIFT);

	switch (depth) {
	case CV_8U:  r = "8U"; break;
	case CV_8S:  r = "8S"; break;
	case CV_16U: r = "16U"; break;
	case CV_16S: r = "16S"; break;
	case CV_32S: r = "32S"; break;
	case CV_32F: r = "32F"; break;
	case CV_64F: r = "64F"; break;
	default:     r = "User"; break;
	}

	r += "C";
	r += (chans + '0');

	return r;
}

//TODO: integrate both shape funtions in one template!
//template<typename T_Mat>

/** Find Shapes
*
* Finds shapes in image. Approximates shapes. Searchs for traffic signs.
*
* @param Point  in - input Matrix, original - original input matrix, colour - colour of shapes to search for, 
*  approx_factor - approximation strenght, a_sign_struct - struct for detection results
∗ @return Mat - output image with integrated found signs
∗ @author Max Wahl, Oleg Tydynyan, Robert Ledwig
*/
Mat find_shapes(const Mat& in, const Mat& original, int colour, float approx_factor, sign_struct * a_sing_struct)
{
	Mat bw = in;
	Mat input = original;

	std::vector<std::vector<cv::Point> > contours;

	//used algorithm:
	//http://docs.opencv.org/modules/imgproc/doc/structural_analysis_and_shape_descriptors.html?highlight=findcontours#findcontours
	String ty = type2str(bw.type());
	String ty2 = type2str(input.type());

	cv::findContours(bw.clone(), contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);

	std::vector<cv::Point> approx;
	cv::Mat dst = input.clone();
	Scalar color = Scalar(0, 0, 255, 0);
	cv::Rect rec;

	str_found_signs_this_round actual_round = { false, false, false };

	for (int i = 0; i < contours.size(); i++)
	{
		// Approximate contour with accuracy proportional
		// to the contour perimeter
		cv::approxPolyDP(cv::Mat(contours[i]), approx, cv::arcLength(cv::Mat(contours[i]), true)*approx_factor, true); //(0.012 optimal, 0.02 original)

		// Skip small or non-convex objects 
		if ((std::fabs(cv::contourArea(contours[i])) < MIN_SIZE) || (!cv::isContourConvex(approx)))
			continue;

		// Number of vertices of polygonal curve
		int vtc = approx.size();

		// Get the cosines of all corners
		std::vector<double> cos;
		for (int j = 2; j < vtc + 1; j++)
			cos.push_back(angle(approx[j%vtc], approx[j - 2], approx[j - 1]));

		// Sort ascending the cosine values
		std::sort(cos.begin(), cos.end());

		// Get the lowest and the highest cosine
		double mincos = cos.front();
		double maxcos = cos.back();

		// Use the degrees obtained above and the number of vertices
		// to determine the shape of the contour
		//if sign is found in correct colour draw the contour and the border and break the loop

		if (approx.size() == 3 && colour == RED && mincos >= 0.17 && maxcos <= 0.766) //30° - 90° (60°)
		{
			//check direction here
			if ((triangle_check(approx[0], approx[1], approx[2])) == 0)
			{
				setLabel(dst, "VF_GW", contours[i]);    
				drawContours(dst, contours, i, color);
				rec = cv::boundingRect(contours[i]);
				cv::rectangle(dst, rec, Scalar(0, 0, 255, 0));
				(&actual_round)->b_vf_gw = true;
				//break;			
			}
			else
			{
				setLabel(dst, "WARN", contours[i]);    
				drawContours(dst, contours, i, color);
				rec = cv::boundingRect(contours[i]);
				cv::rectangle(dst, rec, Scalar(0, 0, 255, 0));
				//break;
			}
		}
		else if (approx.size() >= 4 && approx.size() <= 8)
		{

			if (vtc == 4 && colour == YELLOW && mincos >= -0.34 && maxcos <= 0.34) //70°-110°
			{

				if (!(rectangle_check(approx[0], approx[1], approx[2], approx[3]))) 
				{
					setLabel(dst, "VF_STR", contours[i]);
					drawContours(dst, contours, i, color);
					rec = cv::boundingRect(contours[i]);
					cv::rectangle(dst, rec, Scalar(0, 0, 255, 0));
					(&actual_round)->b_vf_str = true;
				}
			}

			//for stop signs angle check is disabled (better results)
			else if (vtc == 8 && colour == RED)// && mincos >= -0.75 && maxcos <= -0.68)
			{
				setLabel(dst, "STOP", contours[i]);
				drawContours(dst, contours, i, color);
				rec = cv::boundingRect(contours[i]);
				cv::rectangle(dst, rec, Scalar(0, 0, 255, 0));
				(&actual_round)->bstop = true;
				//break;
			}
		}
		else if (colour == RED && approx.size() >= 20)
		{
			setLabel(dst, "CIR", contours[i]);
			drawContours(dst, contours, i, color);
			rec = cv::boundingRect(contours[i]);
			cv::rectangle(dst, rec, Scalar(0, 0, 255, 0));
		}
	}

		AddToFloatAvg(&(a_sing_struct->vf_gw), (&actual_round)->b_vf_gw);
		AddToFloatAvg(&(a_sing_struct->stop), (&actual_round)->bstop);
		AddToFloatAvg(&(a_sing_struct->vf_str), (&actual_round)->b_vf_str);
	
	return dst;

}

/** Find Shapes (UMat)
*
* Finds shapes in image. Approximates shapes. Searchs for traffic signs.
* This function uses the GPU.
*
* @param Point  in - input Matrix, original - original input matrix, colour - colour of shapes to search for,
*  approx_factor - approximation strenght, a_sign_struct - struct for detection results
∗ @return Mat - output image with integrated found signs
∗ @author Max Wahl, Oleg Tydynyan, Robert Ledwig
*/
UMat find_shapes(const UMat& in, const UMat& original, int colour, float approx_factor, sign_struct * a_sing_struct)
{
	UMat bw = in;
	UMat input = original;
	std::vector<std::vector<cv::Point> > contours;
	cv::findContours(bw.clone(), contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);

	std::vector<cv::Point> approx;
	cv::UMat dst = input.clone();
	Scalar color = Scalar(0, 0, 255, 0);
	cv::Rect rec;

	str_found_signs_this_round actual_round = { false, false, false };

	for (int i = 0; i < contours.size(); i++)
	{
		cv::approxPolyDP(cv::Mat(contours[i]), approx, cv::arcLength(cv::Mat(contours[i]), true)*approx_factor, true); //(0.012 optimal, 0.02 original)

		// Skip small or non-convex objects 
		if ((std::fabs(cv::contourArea(contours[i])) < MIN_SIZE) || (!cv::isContourConvex(approx)))
			continue;

		// Number of vertices of polygonal curve
		int vtc = approx.size();

		// Get the cosines of all corners
		std::vector<double> cos;
		for (int j = 2; j < vtc + 1; j++)
			cos.push_back(angle(approx[j%vtc], approx[j - 2], approx[j - 1]));

		// Sort ascending the cosine values
		std::sort(cos.begin(), cos.end());

		// Get the lowest and the highest cosine
		double mincos = cos.front();
		double maxcos = cos.back();

		//look for signs
		//if sign is found in correct colour draw the contour and the border and break the loop

		if (approx.size() == 3 && colour == RED && mincos >= 0.17 && maxcos <= 0.766) //30° - 90° (60°)
		{
			//check direction here
			if ((triangle_check(approx[0], approx[1], approx[2])) == 0)
			{
				setLabel(dst, "VF_GW", contours[i]);    
				drawContours(dst, contours, i, color);
				rec = cv::boundingRect(contours[i]);
				//cv::rectangle(dst, rec, Scalar(0, 0, 255, 0));
				(&actual_round)->b_vf_gw = true;
				//break;
			}
			else
			{
				setLabel(dst, "WARN", contours[i]);  
				drawContours(dst, contours, i, color);
				rec = cv::boundingRect(contours[i]);
				//cv::rectangle(dst, rec, Scalar(0, 0, 255, 0));
				//break;
			}
		}
		else if (approx.size() >= 4 && approx.size() <= 8)
		{


			if (vtc == 4 && colour == YELLOW && mincos >= -0.34 && maxcos <= 0.34) 
			{

				if (!(rectangle_check(approx[0], approx[1], approx[2], approx[3]))) 
					//else
				{
					setLabel(dst, "VF_STR", contours[i]);
					drawContours(dst, contours, i, color);
					rec = cv::boundingRect(contours[i]);
					(&actual_round)->b_vf_str = true;
					//cv::rectangle(dst, rec, Scalar(0, 0, 255, 0));
					//break;
				}
			}

			else if (vtc == 8 && colour == RED)// && mincos >= -0.75 && maxcos <= -0.68)// 135° +-2,5° //-0,71 +- 0,03
			{
				setLabel(dst, "STOP", contours[i]);
				drawContours(dst, contours, i, color);
				rec = cv::boundingRect(contours[i]);
				//cv::rectangle(dst, rec, Scalar(0, 0, 255, 0));
				(&actual_round)->bstop = true;
				//break;
			}
		}
		else if (colour == RED && approx.size() >= 20)
		{
			setLabel(dst, "CIR", contours[i]);
			drawContours(dst, contours, i, color);
			rec = cv::boundingRect(contours[i]);
			//cv::rectangle(dst, rec, Scalar(0, 0, 255, 0));
		}
	}

	if (colour == RED)
	{
	AddToFloatAvg(&(a_sing_struct->vf_gw), (&actual_round)->b_vf_gw);
	AddToFloatAvg(&(a_sing_struct->stop), (&actual_round)->bstop);
	}
	else
	AddToFloatAvg(&(a_sing_struct->vf_str), (&actual_round)->b_vf_str);
	//GetValue(&(a_sing_struct->vf_str));

	return dst;

}


/**
* Helper function to display text in the center of a contour
*/
void setLabel(cv::Mat& im, const std::string label, std::vector<cv::Point>& contour)
{
	int fontface = cv::FONT_HERSHEY_SIMPLEX;
	double scale = 0.4;
	int thickness = 1;
	int baseline = 0;

	cv::Size text = cv::getTextSize(label, fontface, scale, thickness, &baseline);
	cv::Rect r = cv::boundingRect(contour);

	cv::Point pt(r.x + ((r.width - text.width) / 2), r.y + ((r.height + text.height) / 2));
	cv::rectangle(im, pt + cv::Point(0, baseline), pt + cv::Point(text.width, -text.height), CV_RGB(255, 255, 255), CV_FILLED);
	cv::putText(im, label, pt, fontface, scale, CV_RGB(0, 0, 0), thickness, 8);
}

/**
* Helper function to display text in the center of a contour (optimized for UMat)
*/
void setLabel(cv::UMat& im, const std::string label, std::vector<cv::Point>& contour)
{
	int fontface = cv::FONT_HERSHEY_SIMPLEX;
	double scale = 0.4;
	int thickness = 1;
	int baseline = 0;

	cv::Size text = cv::getTextSize(label, fontface, scale, thickness, &baseline);
	cv::Rect r = cv::boundingRect(contour);

	cv::Point pt(r.x + ((r.width - text.width) / 2), r.y + ((r.height + text.height) / 2));
	cv::rectangle(im, pt + cv::Point(0, baseline), pt + cv::Point(text.width, -text.height), CV_RGB(255, 255, 255), CV_FILLED);
	cv::putText(im, label, pt, fontface, scale, CV_RGB(0, 0, 0), thickness, 8);
}

