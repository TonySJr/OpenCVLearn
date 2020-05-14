// OpenCVLearn.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <chrono>
//#include <vector>


class Timer
{
public:
	Timer()
	{
		m_StartTimepoint = std::chrono::high_resolution_clock::now();
	}
	~Timer()
	{
		Stop();
	}
	void Stop()
	{
		auto endTimepoint = std::chrono::high_resolution_clock::now();

		auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
		auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();
		auto duration = end - start;
		auto ms = duration * 0.001;
		auto sec = ms * 0.001;
		auto fps = 1 / sec;
		std::cout << duration << " us (" << ms << " ms)\n";
		std::cout << fps << " frames/sec\n";
	}
private:
	std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimepoint;
};


using namespace std;
using namespace cv;


Mat matrix = (Mat_<uint8_t>(1, 4) << 6, 2, 4, 2); // mine

//Mat matrix = (Mat_<uint8_t>(1, 4) << 7, 1, 5, 3); // floyd
#define del 16
int8_t dirY[] = { 0,1,1,1 };
int8_t dirX[] = { 1,-1,0,1 };
Mat src,dst;
vector<Mat> bgr_planes;
const char* window_name1 = "src img";
const char* window_name2 = "pseudo ton";


void pseudoton(Mat& img);
//void floydditter(Mat& img);
int main(int argc, char* argv[])
{
	if (argc < 3) {
		cout << "usage:   Arg 1: image     | Path to image" << endl;
		cout << "\t Arg 2: color, 0 - black and white, 1 - rgb" << endl;
		cout << "\t Arg 3: webcam video, 0 - off , 1 - on" << endl;
		return -1;
	}
	
	string img_path = string(argv[1]);
	int color = atoi(argv[2]);
	int webcam = atoi(argv[3]);
	if (webcam == 0)
	{
		src = imread(img_path);
		if (src.empty())
		{
			cout << "Image can't load, check name \n";
			return 0;
		}
		Mat src_colored;
		//resize(src, src_resized, Size(), 3, 3, 3);

		string jpg = img_path.substr(img_path.length() - 4); // .jpg
		img_path = img_path.substr(0, img_path.length() - 4);

		// black and white
		if (color == 0)
		{
			cvtColor(src, src_colored, COLOR_BGR2GRAY);
			pseudoton(src_colored);
			dst = src_colored;
			img_path += "BW";
		}
		// colored
		else
		{
			split(src, bgr_planes);
			for (int i = 0; i < 3; i++)
				pseudoton(bgr_planes[i]);
			merge(bgr_planes, dst);
			img_path += "colored";
		}
		//resize(dst, dst, Size(), 0.333, 0.333, 3);

		imshow(window_name1, src);
		imshow(window_name2, dst);
		while (true)
		{
			if (waitKey(5) >= 0) break;
		}

		img_path += jpg;
		imwrite(img_path, dst);
		return EXIT_SUCCESS;
	}
	else
	{
		VideoCapture cap;
		int deviceID = 0;             // 0 = open default camera, 1 = open next camera
		int apiID = cv::CAP_ANY;      // 0 = autodetect default API
		cap.open(deviceID + apiID);
		for (;;)
		{
			{
				Timer timer1;
				cap.read(src);
				if (src.empty()) {
					std::cerr << "ERROR! blank frame grabbed\n";
					return 0;
				}
				Mat src_colored;
				if (color == 0)
				{
					cvtColor(src, src_colored, COLOR_BGR2GRAY);
					pseudoton(src_colored);
					dst = src_colored;
				}
				else
				{
					split(src, bgr_planes);
					for (int i = 0; i < 3; i++)
						pseudoton(bgr_planes[i]);
					merge(bgr_planes, dst);
				}
				imshow(window_name2, dst);
				if (waitKey(5) >= 0) break;
			}
		}
		return EXIT_SUCCESS;
	}
}
/*
void floydditter(Mat& img)
{
	Timer timer1;
	// pseudoton
	int8_t error = 0;

	for (int y = 0; y < img.rows - 1; y++)
	{
		for (int x = 1; x < (img.cols - 1); x++)
		{
			uint8_t oldpixel = img.at<uint8_t>(y, x);
			uint8_t newpixel = (uint8_t)(round(oldpixel/255.0)*255);
			img.at<uint8_t>(y, x) = newpixel;
			error = (oldpixel - newpixel) / del;
			
			for (int count = 0; count < matrix.cols; count++)
			{
				uint8_t nextpixel = img.at<uint8_t>(y + dirY[count], x + dirX[count]);
				if (nextpixel + error > 255) img.at<uint8_t>(y + dirY[count], x + dirX[count]) = 255;
				else if (nextpixel + error < 0) img.at<uint8_t>(y + dirY[count], x + dirX[count]) = 0;
				else img.at<uint8_t>(y + dirY[count], x + dirX[count]) += (matrix.at<uint8_t>(count) * error);
 			}

		}
		//error = 0;
	}

}
*/
void pseudoton(Mat& img)
{
	// pseudoton
	int8_t error = 0;

	for (int y = 0; y < img.rows - 1; y++)
	{
		for (int x = 1; x < (img.cols - 1); x++)
		{
			uint8_t pixel = img.at<uint8_t>(y, x);
			if (pixel > 128)
			{
				error = (int8_t)((pixel - 255) / del);

				img.at<uint8_t>(y, x) = 255;
			}
			else
			{
				error = (int8_t)(pixel / del);
				img.at<uint8_t>(y, x) = 0;
			}

			for (int count = 0; count < matrix.cols; count++)
			{
				uint8_t nextpixel = img.at<uint8_t>(y + dirY[count], x + dirX[count]);


				if (nextpixel + error > 255) img.at<uint8_t>(y + dirY[count], x + dirX[count]) = 255;
				else if (nextpixel + error < 0) img.at<uint8_t>(y + dirY[count], x + dirX[count]) = 0;
				//if (nextpixel + error > 255) img.at<uint8_t>(y, x + 1) = 255;
				//else if (nextpixel + error < 0) img.at<uint8_t>(y, x + 1) = 0;
				else img.at<uint8_t>(y + dirY[count], x + dirX[count]) += (matrix.at<uint8_t>(count) * error);
			}

		}
		error = 0;
	}

}