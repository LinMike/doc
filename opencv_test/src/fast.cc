#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <iostream>
#include <string>
using namespace cv;
using namespace std;
int main()
{
	string img_path;
	cout<<"input img path:"<<std::endl;
	while(cin>>img_path)	
	{
		cv::Mat src;
		src = cv::imread(img_path.c_str());
		if (!src.data)
			return -1;
		std::vector < KeyPoint > keyPoints;
		//创建对象，阈值设为55  
		FastFeatureDetector fast(55);
		//特征点检测  
		fast.detect(src, keyPoints);
		//在原图上画出特征点  
		drawKeypoints(src, keyPoints, src, Scalar(0, 0, 255),
				DrawMatchesFlags::DRAW_OVER_OUTIMG);
		imshow("FAST feature", src);
		waitKey(0);
	}
	return 0;
}
