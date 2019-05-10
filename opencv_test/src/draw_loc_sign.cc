#include <opencv2/opencv.hpp>

int main()
{
	int width = 800, len = 6; //width = 600 ~ 1000, len = 5, 6;
	width =  width + (width % 2 == 0 ? 1 : 0);
	int center = width / 2;
	int w = width / len;

	cv::Mat img(width, width, CV_8UC1, cv::Scalar(255));
	std::cout<<"(len==6?w:w/2) = "<<(len==6?w:w/2)<<std::endl;
	cv::circle(img, cv::Point(center, center), w+(len==6?w:w/2)/*180,200*/, cv::Scalar(0),-1);
	cv::circle(img, cv::Point(center, center), (len==6?w:w/2)/*60,100*/, cv::Scalar(255), -1);
	cv::imshow("img", img);
	int key = cv::waitKey(0) & 0xff;
	if(key == 's') {
		std::string path("/home/robot/loc_sign.png");
		cv::imwrite(path, img);
		std::cout<<"save image file to path "<<path<<std::endl;
	}
	return 0;
}
