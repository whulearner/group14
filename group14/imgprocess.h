#pragma once

#include "imgio.h"

using namespace cv;
using namespace std;

class imgprocess:public imgio
{
public:
	imgprocess(void);
	int ImgSpaceFusion(Mat img1,Mat img2,Mat& result);
	int ImgHISFusion(Mat RGB,Mat gray,Mat& result);
	virtual ~imgprocess(void);
protected:
	Mat m_imgpro;
};

