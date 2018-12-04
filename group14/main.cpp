#include "stdafx.h"//VS2017∏Ò Ω

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "imgprocess.h"

using namespace std;
using namespace cv;


int main (){
	imgprocess test;
	
	test.m_img = test.ImgRead("D:\\IMG_20161230_232925.jpg");
	test.m_RGB = test.ImgRead("D:\\Users\\lenovo\\source\\repos\\group14\\group14\\image\\HIS\\ik_beijing_c.bmp");
	test.m_gray = test.ImgRead("D:\\Users\\lenovo\\source\\repos\\group14\\group14\\image\\HIS\\ik_beijing_p.bmp", 0);
	test.ImgHISFusion(test.m_RGB, test.m_gray, test.m_HISresult);

	test.ImgWrite(".\\image\\HISTransformation.bmp", test.m_HISresult);
	test.ImgShow("test",test.m_HISresult);
	
	return 0;
}
