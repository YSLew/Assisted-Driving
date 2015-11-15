#include "Shapes.h"


Mat find_shapes(const Mat& in, const Mat& original, int colour, float approx_factor, sign_struct * a_sing_struct)
{
	Mat bw = in;
	Mat input = original;
	// Find contours
	std::vector<std::vector<cv::Point> > contours;
	cv::findContours(bw.clone(), contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
	//http://docs.opencv.org/modules/imgproc/doc/structural_analysis_and_shape_descriptors.html?highlight=findcontours#findcontours

	std::vector<cv::Point> approx;
	cv::Mat dst = input.clone();
	Scalar color = Scalar(0, 0, 255, 0);
	cv::Rect rec;

	str_found_signs_this_round actual_round = { false, false, false };


	for (int i = 0; i < contours.size(); i++)
	{
		// Approximate contour with accuracy proportional
		// to the contour perimeter
		//factor 0,01, original 0,02! //bestimmt Maß der Aproximierung
		//DIESEN PARAMTER OPTIMIEREN!
		cv::approxPolyDP(cv::Mat(contours[i]), approx, cv::arcLength(cv::Mat(contours[i]), true)*approx_factor, true); //(0.012 optimal, 0.02 original)


		// Skip small or non-convex objects 
		if ((std::fabs(cv::contourArea(contours[i])) < MIN_SIZE) || (!cv::isContourConvex(approx)))
			continue;

		// Number of vertices of polygonal curve
		int vtc = approx.size();

		//Winkelbetrachtung ggf. überflüssig für gestauchte/gestreckte Schilder? Zeitfrage!
		// Get the cosines of all corners
		std::vector<double> cos;
		for (int j = 2; j < vtc + 1; j++)
			//TODO: find out, how this can work!
			//atm this is not logical!
			cos.push_back(angle(approx[j%vtc], approx[j - 2], approx[j - 1]));

		// Sort ascending the cosine values
		std::sort(cos.begin(), cos.end());

		// Get the lowest and the highest cosine
		double mincos = cos.front();
		double maxcos = cos.back();

		// Use the degrees obtained above and the number of vertices
		// to determine the shape of the contour

		//any form with n corners: sum of corners is (n-2)*180°
		//e.g. pentagram: (5-2)*180=540; one corner: 540/5=108°
		//all rounded! TODO: set better values using Excel
		printf("\nMax: %.2f Min: %.2f VTC: %d \n", maxcos, mincos, vtc);

		//look for signs
		//if sign is found in correct colour draw the contour and the border and break the loop

		if (approx.size() == 3 && colour == RED && mincos >= 0.17 && maxcos <= 0.766) //30° - 90° (60°)
		{
			//check direction here
			if ((triangle_check(approx[0], approx[1], approx[2])) == 0)
			{
				setLabel(dst, "VF_GW", contours[i]);    // Triangles (Schild)
				drawContours(dst, contours, i, color);
				rec = cv::boundingRect(contours[i]);
				cv::rectangle(dst, rec, Scalar(0, 0, 255, 0));
				(&actual_round)->b_vf_gw = true;
				//break; //bei einem Dreieck erkannt: abbruch!
				

			}
			else
			{
				setLabel(dst, "WARN", contours[i]);    // Triangles (kein Schild)
				drawContours(dst, contours, i, color);
				rec = cv::boundingRect(contours[i]);
				cv::rectangle(dst, rec, Scalar(0, 0, 255, 0));
				//break;
			}
		}
		else if (approx.size() >= 4 && approx.size() <= 8)
		{



			/*Original
			if (vtc == 4 && mincos >= -0.1 && maxcos <= 0.3)
			setLabel(dst, "RECT", contours[i]);
			else if (vtc == 5 && mincos >= -0.34 && maxcos <= -0.27)
			setLabel(dst, "PENTA", contours[i]);
			else if (vtc == 6 && mincos >= -0.55 && maxcos <= -0.45)
			setLabel(dst, "HEXA", contours[i]);
			*/

			if (vtc == 4 && colour == YELLOW && mincos >= -0.34 && maxcos <= 0.34) //70°-110°
			{

				if (!(rectangle_check(approx[0], approx[1], approx[2], approx[3]))) //setLabel(dst, "RECT", contours[i]);
					//else
				{
					setLabel(dst, "VF_STR", contours[i]);
					drawContours(dst, contours, i, color);
					rec = cv::boundingRect(contours[i]);
					cv::rectangle(dst, rec, Scalar(0, 0, 255, 0));
					(&actual_round)->b_vf_str = true;
					//break;
				}
			}
			/*else if (vtc == 5 && mincos >= -0.36 && maxcos <= -0.21) //109° - 105° //0,309 +- 0,03 //changed for practical reasons
			setLabel(dst, "PENTA", contours[i]);*/

			//deactivated for testing!
			//Test-image failes with HEXA-forms => spread of angles get to high! (max -0.36, min -0.71)
			//reason: two small angles... Should be no problem for traffic signs!

			/*else if (vtc == 6)//&& mincos >= -0.55 && maxcos <= -0.45)// 123° - 116°  //-0,5 +- 0,05
			setLabel(dst, "HEXA", contours[i]);
			else if (vtc == 7)//&& mincos >= -0.67 && maxcos <= -0.57)// 128°+- 3,2° // -0,62 +- 0,05
			setLabel(dst, "HEPTA", contours[i]);*/

			else if (vtc == 8 && colour == RED)// && mincos >= -0.75 && maxcos <= -0.68)// 135° +-2,5° //-0,71 +- 0,03
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
			// Detect and label circles //TODO: find a better way!
			//double area = cv::contourArea(contours[i]);
			//cv::Rect r = cv::boundingRect(contours[i]);
			//int radius = r.width / 2;

			//if (std::abs(1 - ((double)r.width / r.height)) <= 0.2 &&
			//std::abs(1 - (area / (CV_PI * std::pow(radius, 2)))) <= 0.2)
			//simplified
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

UMat find_shapes(const UMat& in, const UMat& original, int colour, float approx_factor, sign_struct * a_sing_struct)
{
	UMat bw = in;
	UMat input = original;
	// Find contours
	std::vector<std::vector<cv::Point> > contours;
	cv::findContours(bw.clone(), contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
	//http://docs.opencv.org/modules/imgproc/doc/structural_analysis_and_shape_descriptors.html?highlight=findcontours#findcontours

	std::vector<cv::Point> approx;
	cv::UMat dst = input.clone();
	Scalar color = Scalar(0, 0, 255, 0);
	cv::Rect rec;

	str_found_signs_this_round actual_round = { false, false, false };


	for (int i = 0; i < contours.size(); i++)
	{
		// Approximate contour with accuracy proportional
		// to the contour perimeter
		//factor 0,01, original 0,02! //bestimmt Maß der Aproximierung
		//DIESEN PARAMTER OPTIMIEREN!
		cv::approxPolyDP(cv::Mat(contours[i]), approx, cv::arcLength(cv::Mat(contours[i]), true)*approx_factor, true); //(0.012 optimal, 0.02 original)


		// Skip small or non-convex objects 
		if ((std::fabs(cv::contourArea(contours[i])) < MIN_SIZE) || (!cv::isContourConvex(approx)))
			continue;

		// Number of vertices of polygonal curve
		int vtc = approx.size();

		//Winkelbetrachtung ggf. überflüssig für gestauchte/gestreckte Schilder? Zeitfrage!
		// Get the cosines of all corners
		std::vector<double> cos;
		for (int j = 2; j < vtc + 1; j++)
			//TODO: find out, how this can work!
			//atm this is not logical!
			cos.push_back(angle(approx[j%vtc], approx[j - 2], approx[j - 1]));

		// Sort ascending the cosine values
		std::sort(cos.begin(), cos.end());

		// Get the lowest and the highest cosine
		double mincos = cos.front();
		double maxcos = cos.back();

		// Use the degrees obtained above and the number of vertices
		// to determine the shape of the contour

		//any form with n corners: sum of corners is (n-2)*180°
		//e.g. pentagram: (5-2)*180=540; one corner: 540/5=108°
		//all rounded! TODO: set better values using Excel
		//printf("\nMax: %.2f Min: %.2f VTC: %d \n", maxcos, mincos, vtc);

		//look for signs
		//if sign is found in correct colour draw the contour and the border and break the loop

		if (approx.size() == 3 && colour == RED && mincos >= 0.17 && maxcos <= 0.766) //30° - 90° (60°)
		{
			//check direction here
			if ((triangle_check(approx[0], approx[1], approx[2])) == 0)
			{
				setLabel(dst, "VF_GW", contours[i]);    // Triangles (Schild)
				drawContours(dst, contours, i, color);
				rec = cv::boundingRect(contours[i]);
				//cv::rectangle(dst, rec, Scalar(0, 0, 255, 0));
				(&actual_round)->b_vf_gw = true;
				//break; //bei einem Dreieck erkannt: abbruch!
			}
			else
			{
				setLabel(dst, "WARN", contours[i]);    // Triangles (kein Schild)
				drawContours(dst, contours, i, color);
				rec = cv::boundingRect(contours[i]);
				//cv::rectangle(dst, rec, Scalar(0, 0, 255, 0));
				//break;
			}
		}
		else if (approx.size() >= 4 && approx.size() <= 8)
		{



			/*Original
			if (vtc == 4 && mincos >= -0.1 && maxcos <= 0.3)
			setLabel(dst, "RECT", contours[i]);
			else if (vtc == 5 && mincos >= -0.34 && maxcos <= -0.27)
			setLabel(dst, "PENTA", contours[i]);
			else if (vtc == 6 && mincos >= -0.55 && maxcos <= -0.45)
			setLabel(dst, "HEXA", contours[i]);
			*/

			if (vtc == 4 && colour == YELLOW && mincos >= -0.34 && maxcos <= 0.34) //70°-110°
			{

				if (!(rectangle_check(approx[0], approx[1], approx[2], approx[3]))) //setLabel(dst, "RECT", contours[i]);
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
			/*else if (vtc == 5 && mincos >= -0.36 && maxcos <= -0.21) //109° - 105° //0,309 +- 0,03 //changed for practical reasons
			setLabel(dst, "PENTA", contours[i]);*/

			//deactivated for testing!
			//Test-image failes with HEXA-forms => spread of angles get to high! (max -0.36, min -0.71)
			//reason: two small angles... Should be no problem for traffic signs!

			/*else if (vtc == 6)//&& mincos >= -0.55 && maxcos <= -0.45)// 123° - 116°  //-0,5 +- 0,05
			setLabel(dst, "HEXA", contours[i]);
			else if (vtc == 7)//&& mincos >= -0.67 && maxcos <= -0.57)// 128°+- 3,2° // -0,62 +- 0,05
			setLabel(dst, "HEPTA", contours[i]);*/

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
			// Detect and label circles //TODO: find a better way!
			//double area = cv::contourArea(contours[i]);
			//cv::Rect r = cv::boundingRect(contours[i]);
			//int radius = r.width / 2;

			//if (std::abs(1 - ((double)r.width / r.height)) <= 0.2 &&
			//std::abs(1 - (area / (CV_PI * std::pow(radius, 2)))) <= 0.2)
			//simplified
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

