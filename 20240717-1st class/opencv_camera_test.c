#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

#define IMG_Width 640
#define IMG_Height 480

int main(void)
{
	int img_width, img_height;
	img_width = 640;
	img_height = 480;
	
	Mat mat_image_org_color;
	Mat mat_image_org_gray;
	Mat mat_image_gray_result;
	Mat image;
	
	Scalar GREEN(0,255,0);
	Scalar RED(0,0,255);
	Scalar BLUE(255,0,0);
	Scalar YELLOW(0,255,255);
	
	VideoCapture cap(0);
	
	cap.set(CV_CAP_PROP_FRAME_WIDTH, img_width);
	cap.set(CV_CAP_PROP_FRAME_WIDTH, img_Height);
	
	if (!cap.isOpened())
	{
		cerr <<  "camer X\n";
		return -1;
	}
	cap.read(mat_image_org_color);
	
	printf("Image size[%3d,%3d]\n", img_width, img_height);
	
	nameWindow("Display window", CV_WINDOW,img_height);
	resizeWindow("Display window", img_width, img_height);
	moveWindow("Display window", 10, 10);
	
	while(1)
	mit e nun an ssut uh
