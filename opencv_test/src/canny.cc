#include <opencv2/opencv.hpp>
int main(int arvc, char* argv[])
{
	std::vector<std::string> files;
	cv::glob(argv[1], files);
	for(int i=0;i<files.size();i++)
	{
		cv::Mat img = cv::imread(files[i]);
		cv::Mat blur_img, gray;
		cv::cvtColor(img, gray, CV_BGR2GRAY);
		cv::GaussianBlur(gray, blur_img, cv::Size(3, 3), 0, 0, 4);
		cv::Mat edges;
                int lowThreshold = 30;
		cv::Canny(gray, edges, lowThreshold, lowThreshold * 3, 3);
		cv::imshow("canny", edges);
		cv::waitKey(0);
		std::vector<std::vector<cv::Point> > contours;
		cv::findContours(edges, contours, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
	//	cv::Mat result(img.size(), CV_8UC1);
	//	result = cv::Scalar::all(0);
		cv::Point2f center;
		float radius;
		std::vector<cv::Point> hull;
		for(int i=0;i<contours.size();i++)
		{
			if(contours[i].size() <= 5 || contours[i].size()>200) continue;
	//		cv::minEnclosingCircle(contours[i], center, radius);
	//		cv::RotatedRect rect = cv::minAreaRect(contours[i]);
		    cv::convexHull(contours[i], hull);
		    if(hull.size()<5) continue;
		    cv::RotatedRect rect = cv::fitEllipse(hull);
		    float w_h = rect.size.width/rect.size.height;
			float contourArea = cv::contourArea(hull);//radius * radius * M_PI;
			if(contourArea < 11 /*&& radius > 10*/ && (w_h < 1.5 && w_h > 0.5) )//&& contours[i].size()>0.5*M_PI*radius && contours[i].size()<2.5*M_PI*radius
			{
				cv::drawContours(img, contours, i, cv::Scalar(0,0,255));
				std::cout<<"area = "<<contourArea<<std::endl;
				std::cout<<"hull size = "<<hull.size()<<",contours size = "<<contours[i].size()<<std::endl;
			}
		}
		cv::imshow("result", img);
		cv::waitKey(0);
	}

	return 0;
}
