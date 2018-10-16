#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <stdio.h>
using namespace cv;
 
 
int main()
{
	//源图像
	Mat scr = imread("balls_0.jpg");
	Mat rst;
	imshow("原图像",scr);
 
	//Mat kernel(3,3,CV_32F,Scalar(-1)); 
	//Mat kernel = (Mat_<int>(3,3) << 0,-1,0,-1,5,-1,0,-1,0);
	Mat kernel = (Mat_<int>(3,3) << 0,-1,0,-1,5,-1,0,-1,0);
	// 分配像素置
//	kernel.at<float>(1,1) = 8;
//	kernel.at<float>(1,1) = 5;
	filter2D(scr,rst,scr.depth(),kernel);
	imshow("锐化结果",rst);
	waitKey(0);
	return 0;
}
