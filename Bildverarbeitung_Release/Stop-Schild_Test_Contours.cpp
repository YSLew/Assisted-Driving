//Robert

#include <iostream>
#include <omp.h>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>

#include <opencv2/imgproc/imgproc.hpp>

//für Feature-Tracking
#include <opencv2/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>

#include <opencv2/calib3d.hpp>

using namespace cv;
using namespace xfeatures2d;

//draw a box
CvRect box = cvRect(50, 50, 100, 100);
bool drawing_box = false;

cv::Mat global_box;

// Implement mouse callback
void my_mouse_callback(int event, int x, int y, int flags, void* param){

	switch (event){
	case CV_EVENT_MOUSEMOVE:
		if (drawing_box){
			box.width = x - box.x;
			box.height = y - box.y;
		}
		break;

	case CV_EVENT_LBUTTONDOWN:
		drawing_box = true;
		box = cvRect(x, y, 0, 0);
		break;

	case CV_EVENT_LBUTTONUP:
		drawing_box = false;
		if (box.width < 0){
			box.x += box.width;
			box.width *= -1;
		}
		if (box.height < 0){
			box.y += box.height;
			box.height *= -1;
		}

		/*std::cout << x << std::endl;
		std::cout << y << std::endl;
		std::cout << box.width << std::endl;
		std::cout << box.height << std::endl;*/

	}
}

///////////////


int triangle_check(cv::Point pt0, cv::Point pt1, cv::Point pt2)
{
	int i;
	int min = 1000;
	int array[3];

	int A, B, C;

	array[0] = pt0.y;
	array[1] = pt1.y;
	array[2] = pt2.y;

	for (i = 0; i<3; i++)
	{
		if (array[i]<min)
		{
			min = array[i];
		}
	}

	//Untere Ecke "C" sei minimum
	if (min == array[0])
	{
		C = pt0.y; A = pt1.y, B = pt2.y;
	}
	if (min == array[1])
	{
		C = pt1.y; A = pt0.y, B = pt2.y;
	}

	if (min == array[2])
	{
		C = pt2.y; A = pt0.y, B = pt1.y;
	}

	//strecken bestimmen
	int AC = abs(A - C);
	int AB = abs(A - B);
	int BC = abs(B - C);

	if ((AC > AB) && (BC > AB))
	{
		//attention: when using image coordinates 0|0 is in the upper left corner!
		//this means inverting logic!
		printf("Spitze oben");
		return 1;
	}
	else
	{
		printf("Spitze unten");
		return 0;
	}

}

//////////////

// I ist BGR Bild, O is grauwert
Mat look_for_red(const Mat &I)
{
	int channels = I.channels();
	//printf("%d", channels);
	assert(I.channels() == 3);

	int nRows = I.rows;
	int nCols = I.cols;

	if (I.isContinuous())
	{
		nCols *= nRows;
		nRows = 1;
	}

	Mat O = Mat(I.size(), CV_8UC1); //Ausgabematrix initialisieren 

	int i, j;
	const uchar* p;
	uchar* q;

	for (i = 0; i < nRows; ++i)
	{
		p = I.ptr<uchar>(i); //Pointer anlegen, der auf Anfang der Matrix zeigt
		q = O.ptr<uchar>(i);
		for (j = 0; j < nCols; ++j)
		{
			int b = p[3 * j + 0];
			int g = p[3 * j + 1];
			int r = p[3 * j + 2];

			//if (r > 110 && g < 90 && b < 90) //nur rote Elemente auf Wei� setzen
			//q[j] = 255;
			//else
			//q[j] = 0;

			q[j] = r / 2 - g / 2 - b / 2 + 128;
		}
	}
	return O;
}

/**
* Helper function to find a cosine of angle between vectors
* from pt0->pt1 and pt0->pt2
*/
static double angle(cv::Point pt1, cv::Point pt2, cv::Point pt0)
{
	double dx1 = pt1.x - pt0.x;
	double dy1 = pt1.y - pt0.y;
	double dx2 = pt2.x - pt0.x;
	double dy2 = pt2.y - pt0.y;
	return (dx1*dx2 + dy1*dy2) / sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
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


int main()
{
	int Wait_Time = 10;

	std::cout << "Hello OpenCV" << std::endl;

	/* //für Webcam!
	cv::VideoCapture videoCapture(0); //interne Wiedergabe der 1. Quelle (Webcam)!

	videoCapture.set(CAP_PROP_FRAME_WIDTH, 800);
	videoCapture.set(cv::CAP_PROP_FRAME_HEIGHT, 600);

	int w = videoCapture.get(CAP_PROP_FRAME_WIDTH);
	int h = videoCapture.get(cv::CAP_PROP_FRAME_HEIGHT);
	*/


	/*bool ok = videoCapture.open(0);
	if (!ok)
	{
	std::cerr << "no camera found, exit" << std::endl;
	}*/


	bool new_obj = true;
	bool new_match = false;

	//mouse callback
	//namedWindow("Input", WINDOW_AUTOSIZE);
	//namedWindow("Output", WINDOW_AUTOSIZE);


	cv::Mat input;
	cv::Mat input_image; //Mat=Matrix
	cv::Mat input_image_clone;
	cv::Mat output_image;
	cv::Mat input_image_clone_sw;

	Rect region_of_interest2;


	while (1)
	{
		
		//für Kamera!
		//videoCapture >> input_image;

		//input = imread("Dreieck_Scene.JPG");
		//input = imread("Dreieck_Scene_2.JPG");
		input = imread("Testbild1.png"); 
		//input = imread("STOP_Scene_e.jpg");
		//look_for_red(input);

		//ROI für Quadrat anlegen
		region_of_interest2 = box;

		//draw rectangle
		input_image_clone = input.clone();
		cv::rectangle(input_image_clone, region_of_interest2, Scalar(0, 0, 255, 0));
		//imshow("Input", input_image_clone);

	
		//auf rot beschränken!
		input_image = look_for_red(input);

		//imshow("Look for red", input_image);

		//ganz wichtig: filtern, um massenhaft Kleinview-Contouren zu eliminieren
		//Nicht übertreiben! Size(21,21) klappt bei einigen Bildern, bei anderen ist 5,5 schon zu viel! Testen!
		GaussianBlur(input_image, input_image, Size(3, 3), 0, 0);
		//imshow("Filtered", input_image);

		

		// Convert to binary image using Canny
		cv::Mat bw;
		cv::Canny(input_image, bw, 0, 50, 5);

		

		/*operation: The kind of morphology transformation to be performed. Note that we have 5 alternatives:
		Opening: MORPH_OPEN : 2
		Closing: MORPH_CLOSE: 3
		Gradient: MORPH_GRADIENT: 4
		Top Hat: MORPH_TOPHAT: 5
		Black Hat: MORPH_BLACKHAT: 6
		*/

		//show input
		//imshow("Orginal", input);

		//for mouse-control!
		//cvSetMouseCallback("Orginal", my_mouse_callback, 0);

		if (new_obj)
		{

			// Find contours
			std::vector<std::vector<cv::Point> > contours;
			cv::findContours(bw.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

			

			std::vector<cv::Point> approx;
			cv::Mat dst = input.clone();
			Scalar color = Scalar(0, 0, 255, 0);
			

			for (int i = 0; i < contours.size(); i++)
			{
				// Approximate contour with accuracy proportional
				// to the contour perimeter
				//factor 0,01, original 0,02! //bestimmt Maß der Aproximierung
				cv::approxPolyDP(cv::Mat(contours[i]), approx, cv::arcLength(cv::Mat(contours[i]), true)*0.012, true); //(0.012)

				// Skip small or non-convex objects 
				if (std::fabs(cv::contourArea(contours[i])) < 1000 || !cv::isContourConvex(approx))
					continue;

				//draw contours
				Scalar color = Scalar(0, 0, 255, 0);
				drawContours(dst, contours, i, color);

				if (approx.size() == 3)
				{
					//check direction here
					if((triangle_check(approx[0], approx[1], approx[2]))==0)
						setLabel(dst, "TRI DOWN", contours[i]);    // Triangles (Schild)
					else
						setLabel(dst, "TRI UP", contours[i]);    // Triangles (kein Schild)
				}
				else if (approx.size() >= 4 && approx.size() <= 8)
				{
					// Number of vertices of polygonal curve
					int vtc = approx.size();

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
					printf("Max: %.2f Min: %.2f VTC: %d \n", maxcos, mincos, vtc);

					/*Original
					if (vtc == 4 && mincos >= -0.1 && maxcos <= 0.3)
					setLabel(dst, "RECT", contours[i]);
					else if (vtc == 5 && mincos >= -0.34 && maxcos <= -0.27)
					setLabel(dst, "PENTA", contours[i]);
					else if (vtc == 6 && mincos >= -0.55 && maxcos <= -0.45)
					setLabel(dst, "HEXA", contours[i]);
					*/

					if (vtc == 4 && mincos >= -0.1 && maxcos <= 0.3) //95°-72°
						setLabel(dst, "RECT", contours[i]);
					else if (vtc == 5 && mincos >= -0.36 && maxcos <= -0.21) //109° - 105° //0,309 +- 0,03 //changed for practical reasons
						setLabel(dst, "PENTA", contours[i]);
					//deactivated for testing!
					//Test-image failes with HEXA-forms => spread of angles get to high! (max -0.36, min -0.71)
					//reason: two small angles... Should be no problem for traffic signs!

					else if (vtc == 6 )//&& mincos >= -0.55 && maxcos <= -0.45)// 123° - 116°  //-0,5 +- 0,05
						setLabel(dst, "HEXA", contours[i]);
					else if (vtc == 7 )//&& mincos >= -0.67 && maxcos <= -0.57)// 128°+- 3,2° // -0,62 +- 0,05
						setLabel(dst, "HEPTA", contours[i]);
					else if (vtc == 8)// && mincos >= -0.75 && maxcos <= -0.68)// 135° +-2,5° //-0,71 +- 0,03
						setLabel(dst, "OCTA", contours[i]);
				}
				else
				{
					// Detect and label circles
					double area = cv::contourArea(contours[i]);
					cv::Rect r = cv::boundingRect(contours[i]);
					int radius = r.width / 2;

					if (std::abs(1 - ((double)r.width / r.height)) <= 0.2 &&
						std::abs(1 - (area / (CV_PI * std::pow(radius, 2)))) <= 0.2)
						setLabel(dst, "CIR", contours[i]);
				}
			}

			//cv::imshow("src", input);
			cv::imshow("dst", dst);
			//cv::imshow("bw", bw);

	
		}

		new_obj = false;
		new_match = false;
		int c = cv::waitKey(Wait_Time); //entspricht "auffrischen" des Fensters: fragt nachrichten ab, erlaubt OS das Auffrischen des Fensters dazwischen

		switch (c)
		{
		case 'q':	/*videoWriter.release();*/  return 0;
		case 'f':	Wait_Time = 1; break;
		case 'l':	Wait_Time = 500; break;
		case 'e':	Wait_Time = 0; break;
		case 's':
		{
					//cv::imwrite("Testfile.png", input_image);
					new_obj = true;
					break;
		}
		case 'm':
		{
					new_match = true;
					break;
		}
		}
	}

	return 0;
}