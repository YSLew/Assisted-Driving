//Robert

#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>

//f�r Feature-Tracking
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

// I ist BGR Bild, O is grauwert
Mat look_for_red(const Mat &I)
{
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

			q[j] = g / 2 - r / 2 - b / 2 + 128;
		}
	}
	return O;
}


int main(int argc, char *argv[])
{
	int Wait_Time = 10;

	std::cout << "Hello OpenCV" << std::endl;

	/* //f�r Webcam!
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
	bool new_match = true;

	//tracking
	int minHessian = 100;
	Ptr<Feature2D> detector = SURF::create(minHessian);

	//matching
	FlannBasedMatcher matcher;
	std::vector<DMatch> matches;
	std::vector<DMatch> good_matches;
	double max_dist = 0; double min_dist = 50;
	int match_method = 2; //2: knm-Match

	std::vector<Point2f> obj; //Punkte der Matchpartner
	std::vector<Point2f> scene; //Punkte der Matchpartner

	std::vector<KeyPoint> obj_keypoints; // Merkmalspunkte
	Mat obj_descriptors; // Merkmalsvektoren

	std::vector<KeyPoint> scene_keypoints; // Szenenspunkte
	Mat scene_descriptors; // Szenenvektoren

	//mouse callback
	//namedWindow("Input", WINDOW_AUTOSIZE);
	//namedWindow("Output", WINDOW_AUTOSIZE);
	cvSetMouseCallback("Input", my_mouse_callback, 0);


	cv::Mat input;
	cv::Mat input_image; //Mat=Matrix
	cv::Mat input_image_clone;
	cv::Mat output_image;
	cv::Mat input_image_clone_sw;

	Rect region_of_interest2;

	//while (1)
	//{

	//f�r Kamera!
	//videoCapture >> input_image;
	input = imread(argv[1]);
	//imshow("Orginal", input);

	//color convertion
	input_image = look_for_red(input);
	//input_image = look_for_yellow(input);

	/*
	if (input_image.empty())
	{
	std::cerr << "no camera image" << std::endl;
	break;
	}*/

	input_image_clone = input_image.clone();

	//Morph um Flimmern zu vermeiden!
	//Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(5, 5));
	//opening
	//morphologyEx(input_image_clone, input_image_clone, MORPH_CLOSE, kernel);
	//morphologyEx(Maske_f_sw_er, Maske_f_sw, 3, kernel);

	/*operation: The kind of morphology transformation to be performed. Note that we have 5 alternatives:
	Opening: MORPH_OPEN : 2
	Closing: MORPH_CLOSE: 3
	Gradient: MORPH_GRADIENT: 4
	Top Hat: MORPH_TOPHAT: 5
	Black Hat: MORPH_BLACKHAT: 6
	*/

	//imshow("Nach Rot-Suche und Morphologie", input_image_clone);


	//ROI fuer Quadrat anlegen
	region_of_interest2 = box;

	if (new_obj)
	{

		global_box = imread(argv[2]);

		if (global_box.empty())
		{
			std::cerr << "no image found" << std::endl; //automatisches Beenden wenn keine Bilder mehr vorhanden!
			return 1;
		}

		//moegliche Konvertierung
		//cvtColor(global_box, global_box, CV_BGR2GRAY);
		//threshold(global_box, global_box, 128, 255, THRESH_BINARY);
		global_box = look_for_red(global_box);

		//Objekt nach Features durchsuchen
		detector->detectAndCompute(global_box, Mat(), obj_keypoints, obj_descriptors);
		std::cout << "Objekt-Features: " << obj_keypoints.size() << std::endl;
	}

	if (new_match)
	{
		//Szene nach Features durchsuchen
		output_image = input_image_clone.clone();

		//nicht mehr n�tig!
		//cvtColor(input_image_clone, input_image_clone_sw, CV_BGR2GRAY);
		input_image_clone_sw = input_image_clone;

		detector->detectAndCompute(input_image_clone_sw, Mat(), scene_keypoints, scene_descriptors);
		std::cout << "Szenen-Features: " << scene_keypoints.size() << std::endl;

		//Matches finden
		matcher.match(obj_descriptors, scene_descriptors, matches);
		std::cout << "found " << matches.size() << " matches" << std::endl;

		if (match_method == 1)
		{

			//Matches finden
			matcher.match(obj_descriptors, scene_descriptors, matches);
			std::cout << "found " << matches.size() << " matches" << std::endl;

			//Gute Matches aussortieren

			for (int i = 0; i < matches.size(); i++)
			{
				double dist = matches[i].distance;
				if (dist < min_dist) min_dist = dist;
				if (dist > max_dist) max_dist = dist;
			}

			good_matches.clear();
			for (int i = 0; i < matches.size(); i++)
			{
				if (matches[i].distance < 3 * min_dist)
					good_matches.push_back(matches[i]);
			}
		}

		else
		{

			//knn-match: //klappt nicht!
			std::vector<std::vector<cv::DMatch>> matches;
			matcher.knnMatch(obj_descriptors, scene_descriptors, matches, 2); // finde die 2 nahesten Nachbarn 

			good_matches.clear();
			for (int i = 0; i < matches.size(); i++)
			{
				if (matches[i][0].distance < 0.6*(matches[i][1].distance))
					good_matches.push_back(matches[i][0]);
			}

		}

		//Keypoints der besten Matches finden
		obj.clear();
		scene.clear();
		for (unsigned int i = 0; i < good_matches.size(); i++)
		{
			//was passiert hier? was ist queryIdx?
			obj.push_back(obj_keypoints[good_matches[i].queryIdx].pt);
			scene.push_back(scene_keypoints[good_matches[i].trainIdx].pt);
		}

		//matches zeichnen
		drawMatches(global_box, obj_keypoints, input_image_clone_sw, scene_keypoints, good_matches, output_image);

		//Abbildungsmatrix finden und Ecken bestimmen (was passiert hier genau?)

		Mat H = findHomography(obj, scene, CV_RANSAC);
		if (!H.empty())
		{
			//Bestimmung der Ecken der ausgew�hlten Szene (einfach nur �ber Breite und H�he des gew�hlten Ausschnittes)
			std::vector<Point2f> obj_corners(4);
			obj_corners[0] = cvPoint(0, 0);
			obj_corners[1] = cvPoint(global_box.cols, 0);
			obj_corners[2] = cvPoint(global_box.cols, global_box.rows);
			obj_corners[3] = cvPoint(0, global_box.rows);

			std::vector<Point2f> scene_corners(4);

			//Uebertragen der Ecken auf Szene (+ bestm�gliche Verzerrung)
			perspectiveTransform(obj_corners, scene_corners, H);


			//Rahmen zeichnen

			//warum nicht 0,0 f�r p0?
			Point2f p0 = Point2f(global_box.cols, 0);
			Point2f p00 = Point2f(0, 0);

			line(output_image, p0 + scene_corners[0], p0 + scene_corners[1], Scalar(0, 255, 0), 3);
			line(output_image, p0 + scene_corners[1], p0 + scene_corners[2], Scalar(0, 255, 0), 3);
			line(output_image, p0 + scene_corners[2], p0 + scene_corners[3], Scalar(0, 255, 0), 3);
			line(output_image, p0 + scene_corners[3], p0 + scene_corners[0], Scalar(0, 255, 0), 3);

			line(input, p00 + scene_corners[0], p00 + scene_corners[1], Scalar(0, 255, 0), 3);
			line(input, p00 + scene_corners[1], p00 + scene_corners[2], Scalar(0, 255, 0), 3);
			line(input, p00 + scene_corners[2], p00 + scene_corners[3], Scalar(0, 255, 0), 3);
			line(input, p00 + scene_corners[3], p00 + scene_corners[0], Scalar(0, 255, 0), 3);
		}


		//cv::imshow("Output", output_image);
		//cv::imshow("Final", input);
	}


	//Quadrat zeichnen
	cv::rectangle(input_image_clone, region_of_interest2, Scalar(0, 0, 255, 0));

	new_obj = false;
	new_match = false;

	std::string Str_Input = argv[1];
	std::string Str_Scene = argv[2];

	std::string result = Str_Input + "_matched_with_" + Str_Scene + ".png";

	std::cout << result << std::endl;

	cv::imwrite(result, output_image);

	return 0;
}