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
	return img;
}

bool imgio::ImgWrite(const string path, Mat img){
	bool flag = imwrite(path, img);
	return flag;
}

void imgio::ImgShow(const string name, Mat img){
	imshow(name, img);
	waitKey(0);
}