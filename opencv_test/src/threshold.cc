#include <opencv2/opencv.hpp>
int main(int argc, char* argv[])
{
	cv::Mat blur_img, bin, gray;
	std::vector<std::string> files;
	cv::glob(argv[1], files);

	for(int i=0;i<files.size();i++)
	{
		cv::Mat img = cv::imread(files[i]);
		cv::cvtColor(img, gray, CV_BGR2GRAY);
		cv::GaussianBlur(gray, blur_img, cv::Size(3,3), 1);
		//	cv::threshold(blur_img, bin, 135, 255, CV_THRESH_BINARY);
		cv::adaptiveThreshold(blur_img, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 11, -50);
		cv::imshow("bin", bin);
		cv::imshow("raw", img);
		cv::waitKey(0);
	}
	return 0;
}
