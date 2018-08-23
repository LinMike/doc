//opencv2.4.9 vs2012
#include <opencv2/core.hpp>
#include <fstream>
#include <iostream>

using namespace std;
using namespace cv;

cv::Mat Q = (cv::Mat_<float>(4,4) << 1., 0., 0., -6.2239459228515625e+02, 0., 1., 0.,
	       -5.3164443969726562e+02, 0., 0., 0., 7.4425893686310792e+02, 0.,
	       0., 1.4236485105431270e-02, 1.0621361542388574e-01);

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

cv::Mat CM1 = (cv::Mat_<float>(3,3) << 8.3353640130311862e+02, 0., 6.1670705777419641e+02, 0.,
       8.3109868246436565e+02, 5.3530289250231147e+02, 0., 0., 1.);

Mat xyz2uv(Point3f worldPoint, cv::Mat Q) {
	cv::Mat coord = (cv::Mat_<float>(3,1) << worldPoint.x, worldPoint.y, worldPoint.z);
//	cout<<"inv = "<<ext.inv(DECOMP_SVD)<<endl;
	cv::Mat ext_inv = ext.inv(DECOMP_SVD);
	cv::Mat tm = ext_inv * coord;
	std::cout<<"tm = "<<tm<<std::endl;
	cv::Mat c_coord = (cv::Mat_<float>(3,1) << tm.at<float>(0,0)/tm.at<float>(0,3), tm.at<float>(0,1)/tm.at<float>(0,3),tm.at<float>(0,2)/tm.at<float>(0,3));
	cv::Mat uv = P1.inv(DECOMP_SVD) * c_coord;
//	cv::Mat tmp = (cv::Mat_<float>(4,1) << tm.at<float>(0,0), tm.at<float>(0,1), tm.at<float>(0,2), 1);
// 	cv::Mat c_coord = CM1 * P1 * tmp;
//	c_coord.at<float>(0,0) = c_coord.at<float>(0,0) / c_coord.at<float>(0,3);
//	c_coord.at<float>(0,1) = c_coord.at<float>(0,1) / c_coord.at<float>(0,3);
//	c_coord.at<float>(0,2) = c_coord.at<float>(0,2) / c_coord.at<float>(0,3);
//	c_coord.at<float>(0,3) = c_coord.at<float>(0,3) / c_coord.at<float>(0,3);
//	cout<<"c_coord = "<<c_coord<<endl;
//	cout<<"Q = "<<Q<<endl;
//	cv::Mat temp = ext * coord;
//	cout<<"temp = "<<temp<<endl;
//	cv::Mat coord_t = (cv::Mat_<float>(4,1) << temp.at<float>(0,0), temp.at<float>(0,1), temp.at<float>(0,2), 1);
//	cv::Mat uv = Q.inv() * c_coord;
//	cv::Mat P_INV = P1.inv(DECOMP_SVD);
//	cout<<"p inv = "<<P_INV<<endl;
//	cv::Mat uv = P_INV * coord;
//	cv::Mat uv = P1 * coord;
	return uv;
}

int main()
{
//	float x,y,z;
//	while(cin>>x>>y>>z)
	{
		cv::Point3f pt(600,-200,60);
		cv::Mat pixel_pt = xyz2uv(pt, Q);
		cout<<"world point to pixel point is " << pixel_pt<<endl;
	}
	return 0;
}
