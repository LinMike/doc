#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
using namespace cv;
int main()
{
	Mat image = imread("image.jpg");
	if (image.empty()) {
		std::cout<<"read image failed..."<<std::endl;
	}
	Mat gray_image, cnv_image;
	cvtColor(image, gray_image, CV_BGR2GRAY);
	Mat temp(image.size(), CV_32FC1, Scalar(0));

	std::cout<<gray_image.depth()<<", "<<gray_image.channels()<<std::endl;
	gray_image.convertTo(cnv_image, CV_32FC1, 1.0/255);
//	imshow("cnv", cnv_image);
//	waitKey(1000);

	Mat U, W, V;
	SVD::compute(cnv_image, W, U, V);
	cnv_image.convertTo(cnv_image, CV_8UC1);

	Mat w(cnv_image.rows, cnv_image.rows, CV_32FC1, Scalar(0));
	for (int i = 0;i < 20;i++) {
		w.at<float>(i, i) = W.at<float>(i, 0);
		if(i == 19) std::cout<<std::endl;
		else std::cout<<W.at<float>(i, 0)<<" ,";
	}

	temp = U*w*V;
	std::cout<<U<<std::endl;
	temp.convertTo(temp, CV_8UC1, 255.0);
	imshow("1", temp);
	waitKey(6000);
	return 0;
}
