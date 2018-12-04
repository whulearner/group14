#include "stdafx.h"//VS2017��ʽ

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "imgprocess.h"

using namespace std;
using namespace cv;


int main (){

	imgprocess testHIS;
	testHIS.m_RGB = testHIS.ImgRead("D:\\Users\\lenovo\\source\\repos\\group14\\group14\\image\\HIS\\ik_beijing_c.bmp");
	testHIS.m_gray = testHIS.ImgRead("D:\\Users\\lenovo\\source\\repos\\group14\\group14\\image\\HIS\\ik_beijing_p.bmp", 0);
	testHIS.ImgHISFusion(testHIS.m_RGB, testHIS.m_gray, testHIS.m_HISresult);
	testHIS.ImgWrite(".\\image\\HISTransformation.bmp", testHIS.m_HISresult);
	//testHIS.ImgShow("testHIS",testHIS.m_HISresult);
	
	imgprocess testCloudAndWater;
	testCloudAndWater.B = testCloudAndWater.ImgRead("D:\\Users\\lenovo\\source\\repos\\group14\\group14\\image\\cloud\\cloudimage-1.tif", 0);
	testCloudAndWater.G = testCloudAndWater.ImgRead("D:\\Users\\lenovo\\source\\repos\\group14\\group14\\image\\cloud\\cloudimage-2.tif", 0);
	testCloudAndWater.R = testCloudAndWater.ImgRead("D:\\Users\\lenovo\\source\\repos\\group14\\group14\\image\\cloud\\cloudimage-3.tif", 0);
	testCloudAndWater.NIR = testCloudAndWater.ImgRead("D:\\Users\\lenovo\\source\\repos\\group14\\group14\\image\\cloud\\cloudimage-4.tif", 0);
	testCloudAndWater.ImgGray2RGB(testCloudAndWater.R, testCloudAndWater.G, testCloudAndWater.B, testCloudAndWater.RGB);
	testCloudAndWater.ImgCloudExtract(testCloudAndWater.R, testCloudAndWater.G, testCloudAndWater.B, testCloudAndWater.m_Cloudresult);
	//testCloudAndWater.ImgShow("testCloudAndWater", testCloudAndWater.m_Cloudresult);
	testCloudAndWater.ImgWrite(".\\image\\CloudExtractElli.bmp", testCloudAndWater.m_Cloudresult);

	testCloudAndWater.ImgWaterExtract(testCloudAndWater.G, testCloudAndWater.NIR, testCloudAndWater.m_Watresult);
	testCloudAndWater.ImgWrite(".\\image\\WaterExtract.bmp", testCloudAndWater.m_Watresult);


	return 0;
}
