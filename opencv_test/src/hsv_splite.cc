#include <opencv2/opencv.hpp>
int main(int argc, char* argv[])
{
	std::vector<std::string> files;
	cv::glob(argv[1], files);
	for(int i=0;i<files.size();i++)
	{
		cv::Mat srcImage = cv::imread(files[i]);
		cv::Mat img_h, img_s, img_v, imghsv;
		std::vector<cv::Mat> hsv_vec;
		cv::GaussianBlur(srcImage, srcImage, cv::Size(3,3),1);
		cv::cvtColor(srcImage, imghsv, CV_BGR2HSV);
//		cv::imshow("hsv", imghsv);
//		cv::waitKey(0);
		// 分割hsv通道
		cv::split(imghsv, hsv_vec);
		img_h = hsv_vec[2];
		img_s = hsv_vec[1];
		img_v = hsv_vec[0];
		img_h.convertTo(img_h, CV_32F);
		img_s.convertTo(img_s, CV_32F);
		img_v.convertTo(img_v, CV_32F);
		double max_s, max_h, max_v;
		cv::minMaxIdx(img_h, 0, &max_h);
		cv::minMaxIdx(img_s, 0, &max_s);
		cv::minMaxIdx(img_v, 0, &max_v);
		// 各个通道归一化
		img_h /= max_h;
		img_s /= max_s;
		img_v /= max_v;

		cv::imshow("h", img_h);
//		cv::waitKey(0);
//		cv::imshow("s", img_s);
//		cv::waitKey(0);
		cv::Mat temp;
		cv::imshow("v", img_v);
		cv::threshold(img_v, temp, 100.0/255, 255, CV_THRESH_BINARY_INV);
		std::cout<<"element = "<<img_v.at<float>(20,20)<<std::endl;
//		cv::imshow("temp", temp);
		cv::Mat kernel = cv::getStructuringElement(2, cv::Size(3,3));

		cv::erode(temp, temp, kernel, cv::Point(-1,-1),1);
		cv::dilate(temp, temp, kernel, cv::Point(-1,-1),1);
		cv::imshow("temp2", temp);
		cv::waitKey(0);
	}
	return 0;
}
