
#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>



int main()
{
	int Wait_Time = 10;

	std::cout << "Hello OpenCV" << std::endl;
	cv::VideoCapture videoCapture(2); //interne Wiedergabe der 1. Quelle (Webcam)!

	//cv::VideoCapture videoCapture("V:/TestVideo/BigBuckBunny/big_buck_bunny_480p_h264.mp4"); //Datei als Quelle WICHTIG: left Slash (von allen System erkannt!)

	//cv::VideoCapture videoCapture("http://docs.gstreamer.com/media/sintel_cropped_multilingual.webm"); //Stream als Quelle


	bool ok = videoCapture.open(2);
	if (!ok)
	{
		std::cerr << "no camera found, exit" << std::endl;
	}


	//cv::VideoWriter videoWriter("Testvideo.avi", cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 25.0, cv::Size(w, h));
	

	videoCapture.set(cv::CAP_PROP_FRAME_WIDTH, 1920.0);
	videoCapture.set(cv::CAP_PROP_FRAME_HEIGHT, 1080.0);

	int w = videoCapture.get(cv::CAP_PROP_FRAME_WIDTH);
	int h = videoCapture.get(cv::CAP_PROP_FRAME_HEIGHT);

	std::cout << w << " " << h << std::endl;

	while (1)
	{
		cv::Mat image; //Mat=Matrix
		cv::Mat filter_image;

		videoCapture >> image;
		if (image.empty())
		{
			std::cerr << "no camera image" << std::endl;
			break;
		}

		cv::namedWindow("Hello", 0);
		cv::imshow("Hello", image); //Fenster öffnen
		
		//cv::GaussianBlur(image, filter_image, cv::Size(5, 5), 1.0); //gefiltertes Bild öffnen!

		//cv::imshow("Filter", filter_image);
		
		int c = cv::waitKey(Wait_Time); //entspricht "auffrischen" des Fensters: fragt nachrichten ab, erlaubt OS das Auffrischen des Fensters dazwischen
		
		//videoWriter.write(image);
		
		
		switch (c)
		{
		case 'q': /*videoWriter.release(); */ return 0;
			case 'f': Wait_Time = 1; break;
			case 'l': Wait_Time = 500; break;
			case 'e': Wait_Time = 0; break;
			case 's':
			{
				cv::imwrite("Testfile.png", image);
				//cv::imwrite("Testfile_filter.png", filter_image);
				break;
			}
		}
	}

	return 0;
}