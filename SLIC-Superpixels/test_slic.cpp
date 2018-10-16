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

int main(int argc, char *argv[]) {
    /* Load the image and convert to Lab colour space. */
    IplImage *image = cvLoadImage(argv[1], 1);
    IplImage *lab_image = cvCloneImage(image);
    cvCvtColor(image, lab_image, CV_BGR2Lab);
    
    /* Yield the number of superpixels and weight-factors from the user. */
    int w = image->width, h = image->height;
    int nr_superpixels = atoi(argv[2]);
    int nc = atoi(argv[3]);

    double step = sqrt((w * h) / (double) nr_superpixels);
    
    /* Perform the SLIC superpixel algorithm. */
    Slic slic;
    slic.generate_superpixels(lab_image, step, nc);
    slic.create_connectivity(lab_image);
//    slic.colour_with_cluster_means(image);
    slic.display_center_grid(image, CV_RGB(0,255,0));
    std::cout<<"centers size = "<<slic.GetCenters().size()<<std::endl;
    float min_dis = FLT_MAX;
    int min_x = min_y = -1;
    for(int i=0;i<slic.GetCenters().size();i++)
    {
    	std::cout<<"["<<slic.GetCenters()[i][3]<<", "<<slic.GetCenters()[i][4]<<"]"<<std::endl;
    	float dis = sqrt(pow(slic.GetCenter()[i][3]-19,2)+pow(slic.GetCenter()[i][4]-19,2));
    	if(dis < min_dis)
    	{
    		min_x = slic.GetCenter()[i][3];
    		min_y = slic.GetCenter()[i][4];
    		min_dis = dis;
		}
    }
    /* Display the contours and show the result. */
    slic.display_contours(image, CV_RGB(255,0,0));
    cvShowImage("result", image);
    cvWaitKey(0);
    //cvSaveImage(argv[4], image);
}
