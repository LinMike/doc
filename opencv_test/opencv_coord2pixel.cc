#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
using namespace cv;
using namespace std;
float leftIntrinsic[3][3] = { 8.3353640130311862e+02, 0., 6.1670705777419641e+02, 0.,
	       8.3109868246436565e+02, 5.3530289250231147e+02, 0., 0., 1.};  
//左相机畸变系数  
float leftDistortion[1][5] = {-1.0744033166614950e-01, 9.8058269118056207e-02,
	       1.7932574093279653e-05, 1.3429812257774641e-04,
	       -2.2878042267906362e-02};  
//左相机旋转矩阵    
float leftRotation[3][3] = {1,0,0,  0,1,0,  0,0,1 };  
//左相机平移向量    
float leftTranslation[1][3] = {0,0,0};

//右相机内参数矩阵  
float rightIntrinsic[3][3] = {8.3674208595625987e+02, 0., 6.4337563491587230e+02, 0.,
	       8.3399021658290224e+02, 5.3340277037276542e+02, 0., 0., 1.};  
//右相机畸变系数  
float rightDistortion[1][5] = {-9.7851177473849554e-02, 6.9231581219921318e-02,
	       -5.8606323876207885e-04, 6.5380664217726898e-04,
	       4.0113280073856674e-03};  
//右相机旋转矩阵  
float rightRotation[3][3] = {9.9988761935108406e-01, -4.8279954657359913e-03,
	       -1.4192925287078662e-02, 4.6695077346261323e-03,
	       9.9992661370117142e-01, -1.1178681032420817e-02,
	       1.4245854342159719e-02, 1.1111150790587396e-02,
	       9.9983678566162526e-01};  
//右相机平移向量  
float rightTranslation[1][3] = {-7.0234477128620568e+01, -6.1574166660373475e-01,
	       -8.2805402994638100e-01};  


/*Point2f xyz2uv(Point3f worldPoint,float intrinsic[3][3],float translation[1][3],float rotation[3][3])  
{  
   //    [fx s x0]                         [Xc]        [Xw]        [u]   1     [Xc]  
   //K = |0 fy y0|       TEMP = [R T]      |Yc| = TEMP*|Yw|        | | = —*K *|Yc|  
   //    [ 0 0 1 ]                         [Zc]        |Zw|        [v]   Zc    [Zc]  
   //                                                  [1 ]  
   Point3f c;  
   c.x = rotation[0][0]*worldPoint.x + rotation[0][1]*worldPoint.y + rotation[0][2]*worldPoint.z + translation[0][0]*1;  
   c.y = rotation[1][0]*worldPoint.x + rotation[1][1]*worldPoint.y + rotation[1][2]*worldPoint.z + translation[0][1]*1;  
   c.z = rotation[2][0]*worldPoint.x + rotation[2][1]*worldPoint.y + rotation[2][2]*worldPoint.z + translation[0][2]*1;  
 
   Point2f uv;  
   uv.x = (intrinsic[0][0]*c.x + intrinsic[0][1]*c.y + intrinsic[0][2]*c.z)/c.z;  
   uv.y = (intrinsic[1][0]*c.x + intrinsic[1][1]*c.y + intrinsic[1][2]*c.z)/c.z;  
 
   return uv;  
}

int main()
{
//	cv::Point3f xyz(800, 250, 60);
	Point3f xyz(700,220,530); 
	cv::Point2f uv1 = xyz2uv(xyz, leftIntrinsic, leftTranslation, leftRotation);
	cv::Point2f uv2 = xyz2uv(xyz, rightIntrinsic, rightTranslation, rightRotation);
	
	std::cout<<"left uv = "<<uv1<<", right uv = "<<uv2<<std::endl;
	return 0;
}*/
//左相机内参数矩阵  
/*
float leftIntrinsic[3][3] = {4037.82450,             0,     947.65449,  
                                     0,    3969.79038,     455.48718,  
                                     0,             0,             1};  
//左相机畸变系数  
float leftDistortion[1][5] = {0.18962, -4.05566, -0.00510, 0.02895, 0};  
//左相机旋转矩阵  
float leftRotation[3][3] = {0.912333,       -0.211508,       0.350590,   
                           0.023249,       -0.828105,      -0.560091,   
                           0.408789,        0.519140,      -0.750590};  
//左相机平移向量  
float leftTranslation[1][3] = {-127.199992, 28.190639, 1471.356768};  
 
//右相机内参数矩阵  
float rightIntrinsic[3][3] = {3765.83307,            0,     339.31958,  
                                       0,  3808.08469,     660.05543,  
                                       0,           0,             1};  
//右相机畸变系数  
float rightDistortion[1][5] = {-0.24195, 5.97763, -0.02057, -0.01429, 0};  
//右相机旋转矩阵  
float rightRotation[3][3] = {-0.134947,      0.989568,      -0.050442,   
                             0.752355,      0.069205,      -0.655113,   
                            -0.644788,     -0.126356,      -0.753845};  
//右相机平移向量  
float rightTranslation[1][3] = {50.877397, -99.796492, 1507.312197};  
*/


/*Point3f uv2xyz(Point2f uvLeft,Point2f uvRight)  
{  
   //  [u1]      |X|                     [u2]      |X|  
   //Z*|v1| = Ml*|Y|                   Z*|v2| = Mr*|Y|  
   //  [ 1]      |Z|                     [ 1]      |Z|  
   //            |1|                               |1|  
   Mat mLeftRotation = Mat(3,3,CV_32F,leftRotation);  
   Mat mLeftTranslation = Mat(3,1,CV_32F,leftTranslation);  
   Mat mLeftRT = Mat(3,4,CV_32F);//左相机M矩阵  
   hconcat(mLeftRotation,mLeftTranslation,mLeftRT);  
   Mat mLeftIntrinsic = Mat(3,3,CV_32F,leftIntrinsic);  
   Mat mLeftM = mLeftIntrinsic * mLeftRT;  
   //cout<<"左相机M矩阵 = "<<endl<<mLeftM<<endl;  
 
   Mat mRightRotation = Mat(3,3,CV_32F,rightRotation);  
   Mat mRightTranslation = Mat(3,1,CV_32F,rightTranslation);  
   Mat mRightRT = Mat(3,4,CV_32F);//右相机M矩阵  
   hconcat(mRightRotation,mRightTranslation,mRightRT);  
   Mat mRightIntrinsic = Mat(3,3,CV_32F,rightIntrinsic);  
   Mat mRightM = mRightIntrinsic * mRightRT;  
   //cout<<"右相机M矩阵 = "<<endl<<mRightM<<endl;  
 
   //最小二乘法A矩阵  
   Mat A = Mat(4,3,CV_32F);  
   A.at<float>(0,0) = uvLeft.x * mLeftM.at<float>(2,0) - mLeftM.at<float>(0,0);  
   A.at<float>(0,1) = uvLeft.x * mLeftM.at<float>(2,1) - mLeftM.at<float>(0,1);  
   A.at<float>(0,2) = uvLeft.x * mLeftM.at<float>(2,2) - mLeftM.at<float>(0,2);  
 
   A.at<float>(1,0) = uvLeft.y * mLeftM.at<float>(2,0) - mLeftM.at<float>(1,0);  
   A.at<float>(1,1) = uvLeft.y * mLeftM.at<float>(2,1) - mLeftM.at<float>(1,1);  
   A.at<float>(1,2) = uvLeft.y * mLeftM.at<float>(2,2) - mLeftM.at<float>(1,2);  
 
   A.at<float>(2,0) = uvRight.x * mRightM.at<float>(2,0) - mRightM.at<float>(0,0);  
   A.at<float>(2,1) = uvRight.x * mRightM.at<float>(2,1) - mRightM.at<float>(0,1);  
   A.at<float>(2,2) = uvRight.x * mRightM.at<float>(2,2) - mRightM.at<float>(0,2);  
 
   A.at<float>(3,0) = uvRight.y * mRightM.at<float>(2,0) - mRightM.at<float>(1,0);  
   A.at<float>(3,1) = uvRight.y * mRightM.at<float>(2,1) - mRightM.at<float>(1,1);  
   A.at<float>(3,2) = uvRight.y * mRightM.at<float>(2,2) - mRightM.at<float>(1,2);  
 
   //最小二乘法B矩阵  
   Mat B = Mat(4,1,CV_32F);  
   B.at<float>(0,0) = mLeftM.at<float>(0,3) - uvLeft.x * mLeftM.at<float>(2,3);  
   B.at<float>(1,0) = mLeftM.at<float>(1,3) - uvLeft.y * mLeftM.at<float>(2,3);  
   B.at<float>(2,0) = mRightM.at<float>(0,3) - uvRight.x * mRightM.at<float>(2,3);  
   B.at<float>(3,0) = mRightM.at<float>(1,3) - uvRight.y * mRightM.at<float>(2,3);  
 
   Mat XYZ = Mat(3,1,CV_32F);  
   //采用SVD最小二乘法求解XYZ  
   cv::solve(A,B,XYZ,DECOMP_SVD);  
 
   //cout<<"空间坐标为 = "<<endl<<XYZ<<endl;  
 
   //世界坐标系中坐标  
   Point3f world;  
   world.x = XYZ.at<float>(0,0);  
   world.y = XYZ.at<float>(1,0);  
   world.z = XYZ.at<float>(2,0);  
 
   return world;  
}  */
 
//************************************  
// Description: 将世界坐标系中的点投影到左右相机成像坐标系中  
// Method:    xyz2uv  
// FullName:  xyz2uv  
// Access:    public   
// Parameter: Point3f worldPoint  
// Parameter: float intrinsic[3][3]  
// Parameter: float translation[1][3]  
// Parameter: float rotation[3][3]  
// Returns:   cv::Point2f  
// Author:    小白  
// Date:      2017/01/10  
// History:  
//************************************  
Point2f xyz2uv(Point3f worldPoint,float intrinsic[3][3],float translation[1][3],float rotation[3][3])  
{  
   //    [fx s x0]                         [Xc]        [Xw]        [u]   1     [Xc]  
   //K = |0 fy y0|       TEMP = [R T]      |Yc| = TEMP*|Yw|        | | = —*K *|Yc|  
   //    [ 0 0 1 ]                         [Zc]        |Zw|        [v]   Zc    [Zc]  
   //                                                  [1 ]  
   Point3f c;  
   c.x = rotation[0][0]*worldPoint.x + rotation[0][1]*worldPoint.y + rotation[0][2]*worldPoint.z + translation[0][0]*1;  
   c.y = rotation[1][0]*worldPoint.x + rotation[1][1]*worldPoint.y + rotation[1][2]*worldPoint.z + translation[0][1]*1;  
   c.z = rotation[2][0]*worldPoint.x + rotation[2][1]*worldPoint.y + rotation[2][2]*worldPoint.z + translation[0][2]*1;  
   
   std::cout<<"camera coord = "<<c<<std::endl;
 
   Point2f uv;  
   uv.x = (intrinsic[0][0]*c.x + intrinsic[0][1]*c.y + intrinsic[0][2]*c.z)/c.z;  
   uv.y = (intrinsic[1][0]*c.x + intrinsic[1][1]*c.y + intrinsic[1][2]*c.z)/c.z;  
 
   return uv;  
}  


int main()  
{  
   //已知空间坐标求成像坐标  
   Point3f point(800,200,60);  
   cout<<"左相机中坐标："<<endl;  
   cout<<xyz2uv(point,leftIntrinsic,leftTranslation,leftRotation)<<endl;  
   cout<<"右相机中坐标："<<endl;  
   cout<<xyz2uv(point,rightIntrinsic,rightTranslation,rightRotation)<<endl;  
 
   //已知左右相机成像坐标求空间坐标  
//   Point2f l = xyz2uv(point,leftIntrinsic,leftTranslation,leftRotation);  
//   Point2f r = xyz2uv(point,rightIntrinsic,rightTranslation,rightRotation);  
//   Point3f worldPoint;  
//   worldPoint = uv2xyz(l,r);  
//   cout<<"空间坐标为:"<<endl<<uv2xyz(l,r)<<endl;  
 
   system("pause");  
 
   return 0;  
}