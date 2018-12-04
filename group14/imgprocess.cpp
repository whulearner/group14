#include "stdafx.h"//VS2017格式
#include "imgprocess.h"

#define PI 3.141592653
#define sqrt3 1.7320508075689 

void TransformRGBtoIHS(Mat srcImg, float * RGB, float* Intensity, float* Hue, float*Saturation);
void TransformIHStoRGB(Mat M0, float * RGB, float* Intensity, float* Hue, float*Saturation);
void Histogrammatch(float *pPrincipalImageI, Mat HighData, Mat srcImg);


imgprocess::imgprocess(void)
{
}


imgprocess::~imgprocess(void)
{
}


int imgprocess::ImgSpaceFusion(Mat img1,Mat img2,Mat& result){

	return 1;
}


int imgprocess::ImgHISFusion(Mat imgRGB,Mat imgGray,Mat& result){
	Mat RGB = imgRGB;
	//初始转换后图片的存储空间
	Mat HISafter(RGB.rows, RGB.cols, CV_8UC3, Scalar(0, 0, 0));
	//读取高分辨率图像
	Mat HighGraph = imgGray;
	//新建MSData数组存储原图图像数据
	float * MSData = new float[RGB.rows*RGB.cols * 3];
	for (int k = 0; k<3; k++) {
		for (int i = 0; i<RGB.rows; i++)
		{
			for (int j = 0; j<RGB.cols; j++)
			{
				MSData[i*RGB.cols + j + k * RGB.cols*RGB.rows] = RGB.at<Vec3b>(i, j)[k];
			}
		}
	}
	//新建Intensity，Hue,Saturation三个数组存储高分辨率图像的强度，色调，饱和度。
	float * Intensity = new float[RGB.rows*RGB.cols];
	float * Hue = new float[RGB.rows*RGB.cols];
	float * Saturation = new float[RGB.rows*RGB.cols];


	//对多光谱图像进行ISH变换
	TransformRGBtoIHS(RGB, MSData, Intensity, Hue, Saturation);

	//高分辨率图像与I进行匹配
	Histogrammatch(Intensity, HighGraph, RGB);

	//对多光谱图像进行ISH逆变换
	TransformIHStoRGB(RGB, MSData, Intensity, Hue, Saturation);
	//将变换完成后的结果赋予图像
	for (int k = 0; k<3; k++) {
		for (int i = 0; i<RGB.rows; i++)
		{
			for (int j = 0; j<RGB.cols; j++)
			{
				int fTemp = MSData[RGB.cols*i + j + RGB.cols*RGB.rows*k];
				HISafter.at<Vec3b>(i, j)[k] = max(0, min(fTemp, 255));
			}
		}
	}

	imshow("HIS-Transformation.bmp", HISafter);
	waitKey(0);
	imshow("original", imgRGB);
	waitKey(0);
	result = HISafter;
	return 1;
}


void TransformRGBtoIHS(Mat srcImg, float * RGB, float* Intensity, float* Hue, float* Saturation)
{
	float r, g, b, I, H, S;
	int RowsNum = srcImg.rows;
	int ColsNum = srcImg.cols;
	for (int i = 0; i<srcImg.rows; i++)
	{
		for (int j = 0; j<srcImg.cols; j++)
		{
			b = srcImg.at<Vec3b>(i, j)[0];
			g = srcImg.at<Vec3b>(i, j)[1];
			r = srcImg.at<Vec3b>(i, j)[2];

			I = float((r + g + b) / sqrt3);
			//if(g>=b)
			//H=atan2f((2*r-g-b),(sqrt(3)*(g-b)));
			//else
			//H=atan2f((2*r-g-b),(sqrt(3)*(g-b)))+PI;

			//S=sqrt(6)*sqrt((r-g)*(r-g)+(r-b)*(g-b))/3;
			float m = min(min(r, g), b);
			S = float(1 - sqrt3 * m / I);
			double f = 0.5*(2 * r - g - b) / sqrt((r - g)*(r - g) + (r - b)*(g - b));
			if (g >= b) H = float(acos(f));
			else H = float(2 * PI - acos(f));


			Intensity[i*ColsNum + j] = I;
			Hue[i*ColsNum + j] = H;
			Saturation[i*ColsNum + j] = S;
		}
	}
}

void TransformIHStoRGB(Mat M0, float * RGB, float* Intensity, float* Hue, float*Saturation)
{
	float R, G, B;
	float I, H, S;
	int height = M0.rows;
	int width = M0.cols;

	float s1 = sqrt(3.0);
	for (int i = 0; i<height; i++) {
		for (int j = 0; j<width; j++) {
			I = Intensity[i*width + j];
			H = Hue[i*width + j];
			S = Saturation[i*width + j];

			if (H >= 0 && H<2 * PI / 3)
			{
				R = I * ((1 + (S*cos(H) / cos(PI / 3 - H))) / sqrt3);
				B = (I*(1 - S)) / sqrt3;
				G = sqrt3 * I - R - B;
			}
			else if (H >= 2 * PI / 3 && H<4 * PI / 3)
			{
				G = I * ((1 + (S*cos(H - PI * 2 / 3) / cos(PI - H))) / sqrt3);
				R = (I*(1 - S)) / sqrt3;
				B = sqrt3 * I - R - G;
			}
			else if (H >= 4 * PI / 3 && H<2 * PI)
			{
				G = I * ((1 + (S*cos(H - PI * 4 / 3) / cos(PI * 5 / 3 - H))) / sqrt3);
				B = (I*(1 - S)) / sqrt3;
				R = sqrt3 * I - B - G;
			}
			RGB[width*i + j] = B;
			RGB[width*i + j + width * height] = G;
			RGB[width*i + j + width * height * 2] = R;
		}
	}
}

void Histogrammatch(float *pPrincipalImageI, Mat HighData, Mat srcImg) {

	double MinHigh, MaxHigh, MinI, MaxI;
	MinHigh = MinI = 1000000;
	MaxHigh = MaxI = 0;
	float Panch, Principal;
	//计算两幅图像的最大最小值（I与高分辨率图）
	for (int i = 0; i<srcImg.rows; i++)
	{
		for (int j = 0; j<srcImg.cols; j++)
		{
			float pTemp = float(HighData.at<uchar>(i, j));
			Panch = (float)pTemp;
			if (Panch>MaxHigh) MaxHigh = Panch;
			if (Panch<MinHigh) MinHigh = Panch;

			Principal = pPrincipalImageI[i*srcImg.cols + j];

			if (Principal>MaxI) MaxI = Principal;
			if (Principal<MinI) MinI = Principal;

		}
	}

	//拉伸高分辨率图像，结果保存在第一主分量中
	for (int i = 0; i<srcImg.rows; i++)
	{
		for (int j = 0; j<srcImg.cols; j++)
		{
			float pTemp = (float)HighData.at<uchar>(i, j);
			Panch = (double)pTemp;
			Panch = double((Panch - MinHigh)*(MaxI - MinI) / (MaxHigh - MinHigh) + MinI);
			pPrincipalImageI[i*HighData.cols + j] = Panch;
		}
	}
}