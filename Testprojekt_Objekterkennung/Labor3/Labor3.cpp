
#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>

//für Feature-Tracking
#include <opencv2/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>

#include <opencv2/calib3d.hpp>

using namespace cv;
using namespace xfeatures2d;

//draw a box
CvRect box = cvRect(50,50,500,500);
bool drawing_box = false;

cv::Mat global_box;

void draw_box(cv::Mat img, CvRect rect){

}

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

int main()
{
	int Wait_Time = 10;

	std::cout << "Hello OpenCV" << std::endl;
	cv::VideoCapture videoCapture(0); //interne Wiedergabe der 1. Quelle (Webcam)!



	videoCapture.set(CAP_PROP_FRAME_WIDTH, 1200);
	videoCapture.set(cv::CAP_PROP_FRAME_HEIGHT, 720);

	int w = videoCapture.get(CAP_PROP_FRAME_WIDTH);
	int h = videoCapture.get(cv::CAP_PROP_FRAME_HEIGHT);


	/*bool ok = videoCapture.open(0);
	if (!ok)
	{
	std::cerr << "no camera found, exit" << std::endl;
	}*/


	//cv::VideoWriter videoWriter("Testvideo.avi", cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 25.0, cv::Size(w, h));

	bool new_obj = true;
	bool new_match = false;

	//tracking
	int minHessian = 400;
	Ptr<Feature2D> detector = SURF::create(minHessian);

	//matching
	FlannBasedMatcher matcher;
	std::vector<DMatch> matches;
	std::vector<DMatch> good_matches;
	double max_dist = 0; double min_dist = 100;
	int match_method = 1;

	std::vector<Point2f> obj; //Punkte der Matchpartner
	std::vector<Point2f> scene; //Punkte der Matchpartner

	std::vector<KeyPoint> obj_keypoints; // Merkmalspunkte
	Mat obj_descriptors; // Merkmalsvektoren

	std::vector<KeyPoint> scene_keypoints; // Szenenspunkte
	Mat scene_descriptors; // Szenenvektoren

	//mouse callback
	namedWindow("Input", WINDOW_AUTOSIZE);
	namedWindow("Output", WINDOW_AUTOSIZE);
	cvSetMouseCallback("Input", my_mouse_callback, 0);



	cv::Mat input_image; //Mat=Matrix
	cv::Mat input_image_clone;
	cv::Mat output_image;
	cv::Mat input_image_clone_sw;

	Rect region_of_interest2;

	while (1)
	{

		videoCapture >> input_image;

		/*
		if (input_image.empty())
		{
			std::cerr << "no camera image" << std::endl;
			break;
		}*/

		input_image_clone = input_image.clone();

		//ROI für Quadrat anlegen
		region_of_interest2 = box;

		if (new_obj)
		{

			global_box = input_image_clone(region_of_interest2).clone();
			cvtColor(global_box, global_box, CV_BGR2GRAY);
			//Objekt nach Features durchsuchen
			detector->detectAndCompute(global_box, Mat(), obj_keypoints, obj_descriptors);
			std::cout << "Objekt-Features: " << obj_keypoints.size() << std::endl;
		}

		if (new_match)
		{
			//Szene nach Features durchsuchen
			output_image = input_image_clone.clone();

			cvtColor(input_image_clone, input_image_clone_sw, CV_BGR2GRAY);
			detector->detectAndCompute(input_image_clone_sw, Mat(), scene_keypoints, scene_descriptors);
			std::cout << "Szenen-Features: " << scene_keypoints.size() << std::endl;

			//Matches finden
			matcher.match(obj_descriptors, scene_descriptors, matches);
			std::cout << "found " << matches.size() << " matches" << std::endl;

			//knn-match: //klappt nicht!
			//std::vector<std::vector<cv::DMatch>> matches;
			//matcher->knnMatch(scene_descriptors, obj_descriptors, matches, 2); // finde die 2 nahesten Nachbarn 
			

			//Gute Matches aussortieren
			
			if (match_method == 1)
			{
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
			
			//Keypoints der besten Matches finden
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
				//Bestimmung der Ecken der ausgewählten Szene (einfach nur über Breite und Höhe des gewählten Ausschnittes)
				std::vector<Point2f> obj_corners(4);
				obj_corners[0] = cvPoint(0, 0);
				obj_corners[1] = cvPoint(global_box.cols, 0);
				obj_corners[2] = cvPoint(global_box.cols, global_box.rows);
				obj_corners[3] = cvPoint(0, global_box.rows);

				std::vector<Point2f> scene_corners(4);

				//übertragen der Ecken auf Szene (+ bestmögliche Verzerrung)
				perspectiveTransform(obj_corners, scene_corners, H);


				//Rahmen zeichnen

				//warum nicht 0,0 für p0?
				Point2f p0 = Point2f(global_box.cols, 0);

				line(output_image, p0 + scene_corners[0], p0 + scene_corners[1], Scalar(0, 255, 0), 3);
				line(output_image, p0 + scene_corners[1], p0 + scene_corners[2], Scalar(0, 255, 0), 3);
				line(output_image, p0 + scene_corners[2], p0 + scene_corners[3], Scalar(0, 255, 0), 3);
				line(output_image, p0 + scene_corners[3], p0 + scene_corners[0], Scalar(0, 255, 0), 3);
			}

			



			cv::imshow("Output", output_image);
		}


		//Quadrat zeichnen
		cv::rectangle(input_image_clone, region_of_interest2, Scalar(0, 0, 255, 0));

		//Bilder darstellen
		cv::imshow("Input", input_image_clone);
		//cv::imshow("Scene", input_image_clone_sw);
		//cv::imshow("Objekt", global_box);
		

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