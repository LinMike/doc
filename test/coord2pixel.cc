//opencv2.4.9 vs2012
#include <opencv2/core.hpp>
#include <fstream>
#include <iostream>

using namespace std;
using namespace cv;

cv::Mat P1 = (cv::Mat_<float>(3,4) << 1.8345925815398712e+03, 0., 9.2414773559570312e+02,
	       -1.6439895059938432e+05, 0., 1.8345925815398712e+03,
	       4.2869120407104492e+02, 0., 0., 0., 1., 0. );

cv::Mat P2 = (cv::Mat_<float>(3,4) << 1.8345925815398712e+03, 0., 9.2414773559570312e+02,
	       -1.6439895059938432e+05, 0., 1.8345925815398712e+03,
	       4.2869120407104492e+02, 0., 0., 0., 1., 0. );

cv::Mat ext = (cv::Mat_<float>(3,4) << -6.71268627e-02, -3.84259820e-02, 1.02752936e+00,
	       -5.68801880e+02, -9.99789059e-01, 1.30822207e-03, 3.49677168e-02,
	       -5.93658495e+00, 3.77779128e-03, -9.89011824e-01, -9.54022110e-02,
	       1.58256790e+02);

Mat xyz2uv(Point3f worldPoint, cv::Mat P) {
	cv::Mat coord = (cv::Mat_<float>(4,1) << worldPoint.x, worldPoint.y, worldPoint.z, 1);
	cout<<"coord = "<<coord<<endl;
	cout<<"P = "<<P<<endl;
	cv::Mat temp = ext * coord;
	cout<<"temp = "<<temp<<endl;
	cv::Mat coord_t = (cv::Mat_<float>(4,1) << temp.at<float>(0,0), temp.at<float>(0,1), temp.at<float>(0,2), 1);
	cv::Mat uv = P * coord_t;
	return uv;
}

int main()
{
	float x,y,z;
	while(cin>>x>>y>>z)
	{
		cv::Point3f pt(x,y,z);
		cv::Mat pixel_pt = xyz2uv(pt, P1);
		cout<<"world point to pixel point is " << pixel_pt<<endl;
	}
	return 0;
}
