#include <opencv2/opencv.hpp>

int main(int argc,char* argv[])
{
	std::vector<std::string> files;
	cv::glob(argv[1], files);
	cv::Mat dst, gray;
	for(int i=0;i<files.size();i++)
	{
		cv::Mat src = cv::imread(files[i]);
		cv::cvtColor(src, gray, CV_BGR2GRAY);
//		std::vector<std::vector<cv::Point> > contours;
		cv::Mat edges;
		cv::Canny(gray, edges, 50, 100);
		cv::threshold(edges, edges, 50, 255, CV_THRESH_BINARY);
		cv::GaussianBlur(gray, dst, cv::Size(3,3),0);
		std::vector<cv::Vec3f> circles;
		cv::HoughCircles(edges, circles, CV_HOUGH_GRADIENT, 15, edges.rows/2, 150, 80, 25, 5);
		for (size_t i = 0; i < circles.size(); i++) {
			//提取出圆心坐标
			cv::Point center(round(circles[i][0]), round(circles[i][1]));
			//提取出圆半径
			int radius = round(circles[i][2]);
			//圆心
			cv::circle(src, center, 3, cv::Scalar(0, 255, 0), -1, 4, 0);
			//圆
			cv::circle(src, center, radius, cv::Scalar(0, 0, 255), 3, 4, 0);
		}
		cv::imshow("circle", src);
		cv::waitKey(0);
	}
	return 0;
}
