#include <opencv2/opencv.hpp>
#include <iomanip>
int main(int argc, char* argv[])
{
	std::vector<std::string> files;
	cv::glob(argv[1], files);
	cv::Mat kernel = (cv::Mat_<int>(3, 3) << 0, 1, 0, 1, -4, 1, 0, 1, 0);
	cv::Mat dst, gray, bin;
	for(int i=0;i<files.size();i++)
	{
		cv::Mat src = cv::imread(files[i]);
		cv::cvtColor(src, gray, CV_BGR2GRAY);
		cv::GaussianBlur(gray, gray, cv::Size(3,3), 0);
		cv::filter2D(gray, dst, src.depth(), kernel);
		cv::threshold(dst, bin, 5, 255, CV_THRESH_BINARY);

		std::cout<<std::endl;
		for(int i=0;i<src.rows;i++)
		{
			for(int j=0;j<src.cols;j++)
			{
				std::cout<<std::setw(3)<<std::setfill(' ')<<static_cast<int>(gray.at<uchar>(i, j))<<",";
			}
			std::cout<<std::endl;
		}

		std::cout<<std::endl;
		for(int i=0;i<src.rows;i++)
		{
			for(int j=0;j<src.cols;j++)
			{
				std::cout<<std::setw(3)<<std::setfill(' ')<<static_cast<int>(dst.at<uchar>(i, j))<<",";
			}
			std::cout<<std::endl;
		}

		cv::imshow("filter", bin);
		cv::waitKey(0);
	}
	return 0;
}
