/*
 * test_slic.cpp.
 *
 * Written by: Pascal Mettes.
 *
 * This file creates an over-segmentation of a provided image based on the SLIC
 * superpixel algorithm, as implemented in slic.h and slic.cpp.
 */

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <stdio.h>
#include <math.h>
#include <vector>
#include <float.h>
using namespace std;

#include "slic.h"

inline bool cmp2(string name1, string name2) {
	stringstream ss;
	int n1,n2;

	ss << name1.substr(name1.rfind('_')+1, name1.rfind('.')-name1.rfind('_')-1);
	ss >> n1;
	ss.clear();
	ss << name2.substr(name2.rfind('_')+1, name2.rfind('.')-name1.rfind('_')-1);
	ss >> n2;
	return n1 > n2;
}

inline bool cmp(string name1, string name2) {
	stringstream ss;
	int n1,n2;

	ss << name1.substr(name1.rfind('_')+1, name1.rfind('.')-name1.rfind('_')-1);
	ss >> n1;
	ss.clear();
	ss << name2.substr(name2.rfind('_')+1, name2.rfind('.')-name1.rfind('_')-1);
	ss >> n2;
	return n1 < n2;
}

int main(int argc, char *argv[]) {
	/* Load the image and convert to Lab colour space. */

	vector<string> files;
	cv::glob(argv[1], files);
	cout<<"args 4 = "<<argv[4]<<endl;
	if(sizeof(argv[4]) != 0)
		sort(files.begin(), files.end(), cmp2);
	else sort(files.begin(), files.end(), cmp);
	for (int i = 0; i < files.size(); i++)
	{
		cout<<"file name = "<<files[i]<<endl;
		IplImage *image = cvLoadImage(files[i].c_str(), 1);
		cv::Mat src(image, true);
		IplImage *lab_image = cvCloneImage(image);
		cvCvtColor(image, lab_image, CV_BGR2Lab);

		/* Yield the number of superpixels and weight-factors from the user. */
		int w = image->width, h = image->height;
//		int nr_superpixels = atoi(argv[2]);
		int step = atoi(argv[2]);
		int nc = atoi(argv[3]);

//		double step = sqrt((w * h) / (double) nr_superpixels);
		int nr_superpixels = (w * h) * 1.0 / (step * step);

		/* Perform the SLIC superpixel algorithm. */
		Slic slic;
		slic.generate_superpixels(lab_image, step, nc);
		slic.create_connectivity(lab_image);
		slic.colour_with_cluster_means(image);
		std::cout << "centers size = " << slic.GetCenters().size() << std::endl;

		/* Display the contours and show the result. */
		slic.display_contours(image, CV_RGB(255, 0, 0));
		slic.display_center_grid(image, CV_RGB(0, 255, 0));
		cvNamedWindow("result", CV_GUI_EXPANDED);
		cvShowImage("result", image);
		cvWaitKey(0);
	}
	return 0;
	//cvSaveImage(argv[4], image);
}
