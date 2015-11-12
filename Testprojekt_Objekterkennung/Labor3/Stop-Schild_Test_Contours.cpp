//Sources:

//http://opencv-code.com/tutorials/detecting-simple-shapes-in-an-image/
//http://www.codeproject.com/Articles/850023/Color-Based-Object-Detection


#include <iostream>
#include <omp.h>
#include <stdio.h>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/ocl.hpp>
//#include <opencv2/videoio.hpp>



//#include <opencv2/imgproc/imgproc.hpp>

//für Feature-Tracking
//#include <opencv2/features2d.hpp>
//#include <opencv2/xfeatures2d.hpp>

//#include <opencv2/calib3d.hpp>

using namespace cv;
//using namespace xfeatures2d;


//defines
#define RED 0
#define YELLOW 1

#define RED_RANGE1 cv::Scalar(0, 80, 60)
#define RED_RANGE2 cv::Scalar(18, 255, 255)
#define RED_RANGE3 cv::Scalar(160, 80, 60)
#define RED_RANGE4 cv::Scalar(179, 255, 255)

#define YELLOW_RANGE1 cv::Scalar(15, 25, 25)
#define YELLOW_RANGE2 cv::Scalar(32, 250, 255)

/*
cv::inRange(hsv_image, cv::Scalar(0, 80, 60), cv::Scalar(18, 255, 255), lower_red_hue_range);
cv::inRange(hsv_image, cv::Scalar(160, 80, 60), cv::Scalar(179, 255, 255), upper_red_hue_range); //HUE betweeen 0 and 179
cv::inRange(hsv_image, cv::Scalar(15, 25, 25), cv::Scalar(32, 250, 255), out); //14,60,40 ... 32, 255, 255
*/


#define RED_APPROX 0.012
#define YELLOW_APPROX 0.028

#define MIN_SIZE 1000


//FLAGS

//#define CAMERA
//#define DEBUG_OUTPUT
#define GPU


//gloabals functions


//draw a box
//CvRect box = cvRect(50, 50, 100, 100);
//bool drawing_box = false;

//cv::Mat global_box;

// Implement mouse callback
/*
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

		std::cout << x << std::endl;
		std::cout << y << std::endl;
		std::cout << box.width << std::endl;
		std::cout << box.height << std::endl;

	}
}
*/
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
		//printf("Spitze oben\n");
		return 1;
	}
	else
	{
		//printf("Spitze unten\n");
		return 0;
	}

}

int rectangle_check(cv::Point pt0, cv::Point pt1, cv::Point pt2, cv::Point pt3)
{
	int i, j;
	int min = 1000;
	int array[4];
	int swap;

	int A, B, C, D;

	array[0] = pt0.y;
	array[1] = pt1.y;
	array[2] = pt2.y;
	array[3] = pt3.y;

	for (i = 0; i< 4; i++)
	{
		for (j = i + 1; j<4; j++)
		{
			if (array[i] > array[j]) /* For decreasing order use < */
			{
				swap = array[i];
				array[i] = array[j];
				array[j] = swap;
			}
		}
	}

	A = array[3];
	B = array[2];
	C = array[1];
	D = array[0]; //biggest

	int AD = abs(A - D);
	int DC = abs(D - C);

	if (DC > (AD / 4))
	{
		//printf("Quadrat\n");
		return 0;

	}
	else
	{
		//printf("Raute \n");
		return 1;

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

// I ist BGR Bild, O is grauwert
Mat look_for_yellow(const Mat &I)
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

			q[j] = g / 2 - r / 2 - b / 2 + 128;
		}
	}
	return O;
}


/**
* Helper function to find a cosine of angle between vectors
* from pt0->pt1 and pt0->pt2
*found here:
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

/**
* Helper function to find red areals
*/
Mat check_red_range(const Mat& in)
{
	cv::Mat hsv_image;
	cv::Mat out;
	cv::cvtColor(in, hsv_image, cv::COLOR_BGR2HSV);
	cv::Mat lower_red_hue_range;
	cv::Mat upper_red_hue_range;

	//original: (0,100,100) bis (10, 255, 255); (160,100,100) bis (179, 255, 255),
	//testwerte:
	cv::inRange(hsv_image, RED_RANGE1, RED_RANGE2, lower_red_hue_range);
	cv::inRange(hsv_image, RED_RANGE3, RED_RANGE4, upper_red_hue_range); //HUE betweeen 0 and 179
	cv::Mat red_hue_image;
	cv::addWeighted(lower_red_hue_range, 1.0, upper_red_hue_range, 1.0, 0.0, out);

	return out;

}

/**
* Helper function to find yellow areals
*/
Mat check_yellow_range(const Mat& in)
{
	cv::Mat hsv_image;
	cv::Mat out;
	cv::cvtColor(in, hsv_image, cv::COLOR_BGR2HSV);
	cv::inRange(hsv_image, YELLOW_RANGE1, YELLOW_RANGE2, out); //14,60,40 ... 32, 255, 255
	//invertierte Werte für Value ggü Paint! (Value= 255 =>  minimale Helligkeit, Saturation = 255 => max. Sättigung!)

	return out;

}

Mat find_shapes(const Mat& in, const Mat& original, int colour, float approx_factor)
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

	return dst;

}



int main(int argc, char *argv[])
{
	int Wait_Time = 10;
	int num = 0;
	std::cout << "Hello OpenCV" << std::endl;

	//für Webcam!
#ifdef CAMERA

	cv::VideoCapture videoCapture(1); //interne Wiedergabe der 1. Quelle (Webcam)!
	//cv::VideoCapture videoCapture("http://docs.gstreamer.com/media/sintel_cropped_multilingual.webm");
	//alternative: festes Video!
	//cv::VideoCapture videoCapture("Vorfahrt2.mp4");

	videoCapture.set(CAP_PROP_FRAME_WIDTH, 1920);
	videoCapture.set(cv::CAP_PROP_FRAME_HEIGHT, 1080);

	//int w = videoCapture.get(CAP_PROP_FRAME_WIDTH);
	//int h = videoCapture.get(cv::CAP_PROP_FRAME_HEIGHT);

	/*bool ok = videoCapture.open(0);
	if (!ok)
	{
	std::cerr << "no camera found, exit" << std::endl;
	return  -1;
	}*/
#endif;



	bool new_obj = true;
	bool new_match = true;

	//mouse callback
	//namedWindow("Input", WINDOW_AUTOSIZE);
	//namedWindow("Output", WINDOW_AUTOSIZE);

	cv::Mat input;
	cv::Mat input_image;
	cv::Mat input_image_red;
	cv::Mat input_image_yellow;
	cv::Mat input_image_clone;
	cv::Mat output_image;
	cv::Mat input_image_clone_sw;

	Rect region_of_interest2;

	// Convert input image to HSV
	cv::Mat hsv_image;

	// Threshold the HSV image, keep only the red pixels


	while (1)
	{
		double t = (double)getTickCount();
		// do something ...
		

#ifdef CAMERA
		videoCapture >> input;


#endif;



#ifndef CAMERA
		if (argc == 1)
		{
			//input = imread("Dreieck_Scene_2.JPG");
			//input = imread("Dreieck_93.JPG");
			//input = imread("Dreieck_3.JPG");

			//input = imread("Testbild_Color.png"); 
			input = imread("Viereck_7.JPG");
			//input = imread("Vorfahrtsfehler/Viereck_90.JPG");

			//input = imread("STOP_Scene_e.jpg");
			//look_for_red(input);
		}
		else
			input = imread(argv[1]);
#endif;	

		input_image = input.clone();

		input_image_clone = input.clone();

		//Farbfilterung

		//Variante B
		input_image_red = check_red_range(input_image);
		input_image_yellow = check_yellow_range(input_image);

		if (argc == 1)
		{
#ifdef DEBUG_OUTPUT
			imshow("Nach Gelb-Threshold", input_image_yellow);
			imshow("Nach Rot-Threshold", input_image_red);
#endif
		}


		//GAUSS-Filter

		//offenbar besser ohne!
		//GaussianBlur(input_image, input_image, Size(1, 1), 0, 0);
		//imshow("Filtered", input_image);


		//Kantenerkennung

		// Convert to binary image using Canny //sehr fraglich! Teilweise bessere Ergenisse ohne Kantendedektion (nur Filter + Find Contours)
		//bewtrifft jeodch nur wenige Sonderfälle!
		//bisher beste Ergebnisse mit weniger strengen Farbfiltern, keinem Gauss-Filter, aber Canny!
		cv::Mat bw_red;// = input_image;

		//#ifdef GPU
		//oclMat alpha(imgThresholded);
		//ocl::oclMat alpha(bw_red); 
		//ocl::colMat beta;
		//ocl::Canny(input_image_red, bw_red, 0, 50, 5, true);
		//bw_red = Mat(beta);
		//#else
		cv::Canny(input_image_red, bw_red, 0, 50, 5, true); //true= more accurate filter, 3 better than 5? found here:
		//http://docs.opencv.org/modules/imgproc/doc/feature_detection.html?highlight=canny#canny
		//#endif


		cv::Mat bw_yellow; //kantenerkennung bei Gelb eher kontraproduktiv. klug?
		//cv::Canny(input_image_yellow, bw_yellow, 0, 50, 5, true);
		bw_yellow = input_image_yellow;

		//find contours

		Mat dst, dst_y;
		//if (new_obj == true)
		//{
		dst = find_shapes(bw_red, input, RED, RED_APPROX); //bei rot ggf schwächer approximieren
		dst_y = find_shapes(bw_yellow, dst, YELLOW, YELLOW_APPROX);
		if (argc == 1)
		{
#ifdef DEBUG_OUTPUT
			imshow("BBW", bw_red);
			cv::imshow("src", input);
			cv::imshow("dst", dst);
			cv::imshow("bw", bw_red);
			cv::imshow("bw_y", bw_yellow);
#endif
			cv::imshow("dst_y", dst_y);
			cv::imshow("bw_y", bw_yellow);
		}
		//}


		//for mouse-control!
		//cvSetMouseCallback("Orginal", my_mouse_callback, 0);

		new_obj = false;
		new_match = false;

#ifndef CAMERA
		if (argc != 1) //Konsolenvariante muss hier enden + Ausgabe
		{
			std::string Str_Input = argv[1];
			std::string result_y = Str_Input + "_tested.png";
			//std::string result = Str_Input + "_tested.png";
			std::cout << result_y << std::endl;
			cv::imwrite(result_y, dst_y);
			//cv::imwrite(result, dst);
			return 0;
		}
#endif

		int c = cv::waitKey(Wait_Time); //entspricht "auffrischen" des Fensters: fragt nachrichten ab, erlaubt OS das Auffrischen des Fensters dazwischen

		switch (c)
		{
		case 'q':	/*videoWriter.release();*/  return 0;
		case 'f':	Wait_Time = 1; break;
		case 'l':	Wait_Time = 500; break;
		case 'e':	Wait_Time = 0; break;
		case 's':
		{
					std::string mystring = "Testfile_" + num;
					mystring = mystring + ".png";
					cv::imwrite(mystring, bw_yellow);
					num++;
					new_obj = true;
					break;
		}
		case 'm':
		{
					new_match = true;
					break;
		}
		}
		t = ((double)getTickCount() - t) / getTickFrequency();
		printf("%0.2f\n", t);
	}

	return 0;
}