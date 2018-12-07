#include "stdafx.h"//VS2017∏Ò Ω
#include "imgio.h"


imgio::imgio(void)
{
}


imgio::~imgio(void)
{
}

Mat imgio::ImgRead(const string path,int flag){
	Mat img = imread(path, flag);
	if (!img.data) {
		img.create(128, 128, CV_16UC1);
		printf_s("failed to open image file\n");
	}
	return img;
}

bool imgio::ImgWrite(const string path, Mat img){
	bool flag = imwrite(path, img);
	return flag;
}

void imgio::ImgShow(const string name, Mat img){
	Mat resImg;
	resize(img, resImg, Size(600, 600 * img.rows / img.cols));
	namedWindow(name);
	imshow(name, resImg);
	waitKey(30000);
}
