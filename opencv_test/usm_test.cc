#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
int main()
{
	cv::Mat image = cv::imread("lena.jpg");
//	cv::cvtColor(image, image, CV_BGR2GRAY);
	cv::Mat kernel = (cv::Mat_<double>(3,3) << 0,-1,0,-1,5,-1,0,-1,0);
	cv::Mat o_image = image.clone();
	cv::filter2D(o_image, image, o_image.depth(), kernel);
	cv::imshow("origin", o_image);
	cv::imshow("usm", image);
	cv::waitKey(0);
	return 0;
}
