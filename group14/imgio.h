#pragma once

#include <opencv.hpp>
#include <highgui.h>

using namespace cv;
using namespace std;

class imgio
{
public:
	imgio(void);
	Mat ImgRead(const string path, int flag = 1);
	bool ImgWrite(const string path, Mat img);
	void ImgShow(const string name, Mat img);
	virtual ~imgio(void);

};

