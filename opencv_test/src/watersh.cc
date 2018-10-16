#include <opencv2/opencv.hpp>
int main(int argc,char* argv[])
{
	cv::Mat img = cv::imread(argv[1]);
	cv::Mat gray, blur;
//	std::vector<cv::Vec4i> h;
	std::vector<std::vector<cv::Point2i> > contours;

	std::vector<cv::Mat> mat_vec;
	cv::cvtColor(img, gray, CV_BGR2GRAY);
	cv::GaussianBlur(gray, blur, cv::Size(3,3), 0);
	cv::Canny(blur, blur, 80, 120);
//	cv::imshow("blur", blur);
//	cv::waitKey(0);
	cv::findContours(blur, contours, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
//	merkers.resize(blur.rows*blur.cols, cv::Scalar(0,0,0));
//	merkers.reshape(CV_32S);
//	cv::Mat merkers(blur.size(), CV_32S, cv::Scalar(0));
	cv::Mat marks(img.size(),CV_32S);   //Opencv分水岭第二个矩阵参数
	marks=cv::Scalar::all(0);
	for(int i=0;i<contours.size();i++)
	{
		cv::drawContours(marks, contours, i, cv::Scalar::all(100+i+1));
	}
	cv::watershed(img, marks);

	cv::Mat temp;
	cv::convertScaleAbs(marks, temp);
	cv::imshow("watershed", temp);
	cv::waitKey(0);
	return 0;
}
