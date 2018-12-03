#include "stdafx.h"//VS2017∏Ò Ω

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "imgprocess.h"

using namespace std;
using namespace cv;


int main (){
	imgprocess test;
	
	Mat img = test.ImgRead("D:\\IMG_20161230_232925.jpg", 1);
	if(!img.rows){
		return 0;
	}

	test.ImgShow("test",img);

	return 0;
}
