#include <opencv2/opencv.hpp>
#include <opencv2/nonfree/features2d.hpp>
//using namespace std;
using namespace cv;
int main()
{
	Mat src, srcGray;

	src = imread("lena.jpg");
	cvtColor(src, srcGray, CV_BGR2GRAY);

	cv::SIFT detector;
	return 0;
}
