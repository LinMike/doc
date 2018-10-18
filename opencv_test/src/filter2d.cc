#include <opencv2/opencv.hpp>
#include <iomanip>
int main(int argc, char* argv[])
{
	std::vector<std::string> files;
	cv::glob(argv[1], files);
	cv::Mat kernel = (cv::Mat_<int>(3, 3) << 0, 1, 0, 1, -4, 1, 0, 1, 0);
//	cv::Mat kernel = (cv::Mat_<int>(2,2) << -2, 1, 1, 0);
	cv::Mat dst, gray, bin, blur;
	for(int i=0;i<files.size();i++)
	{
		cv::Mat src = cv::imread(files[i]);
		cv::cvtColor(src, gray, CV_BGR2GRAY);

//		std::vector<cv::Mat> rgb;
//		std::vector<cv::Mat> f_rgb;
//		cv::split(src, rgb);
//		cv::convertScaleAbs(rgb[0], temp);
//		std::cout<<rgb[0].channels()<<std::endl;
//		cv::imshow("temp", rgb[0]);
		cv::filter2D(gray, dst, CV_64F, kernel);

//		std::cout<<"after split"<<std::endl;
//		cv::filter2D(rgb[1], f_rgb[1], rgb[1].depth(), kernel);
//		cv::filter2D(rgb[2], f_rgb[2], rgb[2].depth(), kernel);
//	cv::merge(f_rgb, dst);
//		cv::GaussianBlur(gray, blur, cv::Size(3,3), 0);
//		cv::GaussianBlur(src, blur, cv::Size(3,3), 0);
//		cv::filter2D(blur, dst, src.depth(), kernel);

		cv::convertScaleAbs(dst, dst);
//		std::cout<<"[19, 18] = "<<static_cast<int>(dst.at<uchar>(19,20))<<std::endl;

//		std::cout<<std::endl;
//		for(int i=0;i<src.rows;i++)
//		{
//			for(int j=0;j<src.cols;j++)
//			{
//				std::cout<<std::setw(3)<<std::setfill(' ')<<static_cast<int>(gray.at<uchar>(i, j))<<",";
//			}
//			std::cout<<std::endl;
//		}

		std::cout<<std::endl;
		for(int i=0;i<src.rows;i++)
		{
			for(int j=0;j<src.cols;j++)
			{
				std::cout<<std::setw(3)<<std::setfill(' ')<<static_cast<int>(dst.at<uchar>(i, j))<<",";
			}
			std::cout<<std::endl;
		}

		cv::Mat temp;
		cv::threshold(dst, bin, 50, 255, CV_THRESH_BINARY);
//		cv::resize(bin, temp, cv::Size(dst.rows*4, dst.cols*4));
		cv::imshow("filter", bin);
		cv::waitKey(0);
	}
	return 0;
}
