#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
using namespace cv;
using namespace std;
int main()
{
	cv::StarDetector detector;
	std::vector<KeyPoint> keyPts;
	cv::Mat src = imread("fast.jpg"), srcGray;
	if(src.empty())
	{
		std::cout<<"load image failed..."<<std::endl;
		assert(false);
	}

	cvtColor(src, srcGray, CV_BGR2GRAY);
	detector(srcGray, keyPts);
	
	for (int i=0;i<keyPts.size();i++)
	{
		circle(src, keyPts[i].pt, keyPts[i].size/2, Scalar(0,255,0));
	}
	imshow("star detector", src);
	waitKey(0);
	return 0;
}
