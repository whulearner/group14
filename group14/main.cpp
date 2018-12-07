#include "stdafx.h"//VS2017格式

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <Windows.h>

#include "imgprocess.h"

using namespace std;
using namespace cv;


int main (){
	bool idir = CreateDirectory(_T(".\\image"), NULL);
	bool rdir = CreateDirectory(_T(".\\result"), NULL);
	if (idir) printf_s("请在程序目录下找到image文件夹，将原始图片放进去。\n");
	int flag = 0;
	printf("输入0：退出程序。\n输入1：相关系数融合。\n输入2：HIS融合。\n输入3：云和水体识别。\n现在输入：");
	scanf_s("%d", &flag);

	if (flag==0)
	{
		exit(0);
	}

	if (flag==1)
	{
		imgprocess testSpaceFusion;
		testSpaceFusion.B = testSpaceFusion.ImgRead(".\\image\\tm1.bmp", 0);
		testSpaceFusion.G = testSpaceFusion.ImgRead(".\\image\\tm3.bmp", 0);
		testSpaceFusion.R = testSpaceFusion.ImgRead(".\\image\\tm4.bmp", 0);
		testSpaceFusion.m_gray = testSpaceFusion.ImgRead(".\\image\\sp_yc.bmp", 0);
		testSpaceFusion.ImgGray2RGB(testSpaceFusion.B, testSpaceFusion.G, testSpaceFusion.R, testSpaceFusion.m_RGB);
		testSpaceFusion.ImgSpaceFusion(testSpaceFusion.R, testSpaceFusion.G, testSpaceFusion.B, testSpaceFusion.m_gray, testSpaceFusion.m_imgproc, 1);
		testSpaceFusion.ImgWrite(".\\result\\SpaceFusion.bmp", testSpaceFusion.m_imgproc);
		testSpaceFusion.ImgShow("result", testSpaceFusion.m_imgproc);
	}

	if (flag==2)
	{
		imgprocess testHIS;
		testHIS.m_RGB = testHIS.ImgRead(".\\image\\ik_beijing_c.bmp");
		testHIS.m_gray = testHIS.ImgRead(".\\image\\ik_beijing_p.bmp", 0);
		testHIS.ImgHISFusion(testHIS.m_RGB, testHIS.m_gray, testHIS.m_HISresult);
		testHIS.ImgWrite(".\\result\\HISTransformation.bmp", testHIS.m_HISresult);
		testHIS.ImgShow("testHIS",testHIS.m_HISresult);
	}

	if (flag==3)
	{
		imgprocess testCloudAndWater;
		testCloudAndWater.B = testCloudAndWater.ImgRead(".\\image\\cloudimage-1.tif", 0);
		testCloudAndWater.G = testCloudAndWater.ImgRead(".\\image\\cloudimage-2.tif", 0);
		testCloudAndWater.R = testCloudAndWater.ImgRead(".\\image\\cloudimage-3.tif", 0);
		testCloudAndWater.NIR = testCloudAndWater.ImgRead(".\\image\\cloudimage-4.tif", 0);
		testCloudAndWater.ImgGray2RGB(testCloudAndWater.R, testCloudAndWater.G, testCloudAndWater.B, testCloudAndWater.RGB);
		testCloudAndWater.ImgCloudExtract(testCloudAndWater.R, testCloudAndWater.G, testCloudAndWater.B, testCloudAndWater.m_Cloudresult);
		//testCloudAndWater.ImgShow("testCloudAndWater", testCloudAndWater.m_Cloudresult);
		//testCloudAndWater.ImgWrite(".\\result\\CloudExtractElli.bmp", testCloudAndWater.m_Cloudresult);
		testCloudAndWater.ImgWaterExtract(testCloudAndWater.G, testCloudAndWater.NIR, testCloudAndWater.m_Watresult);
		//testCloudAndWater.ImgWrite(".\\result\\WaterExtract.bmp", testCloudAndWater.m_Watresult);

		for (int r = 0; r < testCloudAndWater.RGB.rows; r++)
		{
			for (int c = 0; c < testCloudAndWater.RGB.cols; c++)
			{
				testCloudAndWater.RGB.at<Vec3b>(r, c)[0] = max(testCloudAndWater.RGB.at<Vec3b>(r, c)[0], testCloudAndWater.m_Watresult.at<uchar>(r, c));
				testCloudAndWater.RGB.at<Vec3b>(r, c)[0] = max(0, testCloudAndWater.RGB.at<Vec3b>(r, c)[0] - testCloudAndWater.m_Cloudresult.at<uchar>(r, c));
				testCloudAndWater.RGB.at<Vec3b>(r, c)[1] = max(0, testCloudAndWater.RGB.at<Vec3b>(r, c)[1] - testCloudAndWater.m_Cloudresult.at<uchar>(r, c));
				testCloudAndWater.RGB.at<Vec3b>(r, c)[2] = max(testCloudAndWater.RGB.at<Vec3b>(r, c)[2], testCloudAndWater.m_Cloudresult.at<uchar>(r, c));
			}
		}
		testCloudAndWater.ImgWrite(".\\result\\WaterAndCloudExtract.bmp", testCloudAndWater.RGB);
		testCloudAndWater.ImgShow("testCloudAndWater", testCloudAndWater.RGB);
	}

	ShellExecute(NULL, _T("open"), _T(".\\result"), NULL, NULL, SW_SHOW);
	return 0;
}
