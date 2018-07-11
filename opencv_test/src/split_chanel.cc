#include <opencv2/highgui/highgui.hpp>    
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
 
using namespace cv;
 
int main(int argc, char *argv[])
{
	Mat image = imread("Test.jpg", 1);
	if (image.empty())
	{
		std::cout << "打开图片失败,请检查" << std::endl;
		return -1;
	}
	imshow("原图像", image);
	Mat imageRGB[3];
	cv::TickMeter tm;
	tm.start();
	split(image, imageRGB);
	for (int i = 0; i < 3; i++)
	{
		equalizeHist(imageRGB[i], imageRGB[i]);
	}
	merge(imageRGB, 3, image);
	tm.stop();
	std::cout<<"sharpen image use time " << tm.getTimeMilli() <<std::endl;
	imshow("直方图均衡化图像增强效果", image);
	waitKey();
	return 0;
}
