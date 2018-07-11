#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>

int main()
{
	cv::Mat srcColor = cv::imread("fast.jpg");
	if(srcColor.empty()){
		std::cout<<"read image failed ..."<<std::endl;
		assert(false);
	}
	
	cv::MSER ms(4,60,1600,0.05f,0.4f);

	cv::Mat srcGray;
	cv::cvtColor(srcColor, srcGray, CV_BGR2GRAY);

	std::vector<std::vector<cv::Point> > regions;
	ms(srcGray, regions, cv::Mat());

	for(int i=0;i<regions.size();i++)
	{
		cv::ellipse(srcColor, cv::fitEllipse(regions[i]), cv::Scalar(0,0,255), 1);
	}
	cv::imshow("MSER",srcColor);
	cv::waitKey(0);
	return 0;
}
