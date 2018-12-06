#pragma once

#include "imgio.h"

using namespace cv;
using namespace std;

class imgprocess:public imgio
{
public:
	imgprocess(void);
	int ImgSpaceFusion(Mat R, Mat G, Mat B, Mat img2, Mat& result, int flag = 1);
	int ImgHISFusion(Mat RGB,Mat gray,Mat& result);
	int ImgCloudExtract(Mat R, Mat G, Mat B, Mat& result);
	int ImgWaterExtract(Mat G, Mat NIR, Mat& result);
	void ImgGray2RGB(Mat B, Mat G, Mat R, Mat& RGB);

	virtual ~imgprocess(void);
public:
	Mat m_imgproc, R, G, B, NIR, RGB, m_HISresult, m_Cloudresult, m_Watresult;
};

