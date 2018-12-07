#include "stdafx.h"//VS2017格式
#include "imgprocess.h"

#define PI 3.141592653
#define sqrt3 1.7320508075689

void TransRGB2HIS(Mat srcImg, float * RGB, float* Intensity, float* Hue, float*Saturation);
void TransHIS2RGB(Mat M0, float * RGB, float* Intensity, float* Hue, float*Saturation);
void Histogrammatch(float *pPrincipalImageI, Mat HighData, Mat srcImg);
float corrcoef(Mat a, Mat b);
float fastinvSqrt(float x);

using namespace cv;
using namespace std;

imgprocess::imgprocess(void)
{
}


imgprocess::~imgprocess(void)
{
}


int imgprocess::ImgSpaceFusion(Mat R, Mat G, Mat B, Mat P, Mat& result,int flag) {
	Mat Ref, resR, resG, resB, temp3, temp4, temp1, fusR, fusG, fusB, fusRGB;
	Ref.create(1800, 1800, CV_8UC1);
	temp1.create(1800, 1800, CV_8UC1);
	temp3.create(1800, 1800, CV_8UC1);
	temp4.create(1800, 1800, CV_8UC1);
	fusR.create(1800, 1800, CV_8UC1);
	fusG.create(1800, 1800, CV_8UC1);
	fusB.create(1800, 1800, CV_8UC1);

	resize(R, resR, Size(R.cols * 3, R.rows * 3));
	resize(G, resG, Size(G.cols * 3, R.rows * 3));
	resize(B, resB, Size(B.cols * 3, R.rows * 3));

	for (int r = 0; r < Ref.rows; r++) {
		for (int c = 0; c < Ref.cols; c++)
		{
			if (r >= 200 & r < 200 + P.rows & c >= 100 & c < 100 + P.cols) {
				Ref.at<uchar>(r, c) = P.at<uchar>(r - 200, c - 100);
			}
			else {
				Ref.at<uchar>(r, c) = 0;
			}
			if (r >= 120 & r < 120 + resR.rows & c >= 182 & c < 182 + resR.cols) {
				temp3.at<uchar>(r, c) = resR.at<uchar>(r - 120, c - 182);
				temp4.at<uchar>(r, c) = resG.at<uchar>(r - 120, c - 182);
				temp1.at<uchar>(r, c) = resB.at<uchar>(r - 120, c - 182);
			}
			else
			{
				temp3.at<uchar>(r, c) = 0;
				temp4.at<uchar>(r, c) = 0;
				temp1.at<uchar>(r, c) = 0;
			}
		}
	}

	float corr[3] = { 0 };
	if (flag == 1) {
		corr[0] = fabs(corrcoef(temp3, Ref));
		corr[1] = fabs(corrcoef(temp4, Ref));
		corr[2] = fabs(corrcoef(temp1, Ref));
	}
	else
	{
		corr[2] = corr[1] = corr[0] = 0;
	}

	for (int r = 0; r < Ref.rows; r++) {
		for (int c = 0; c < Ref.cols; c++)
		{
			fusR.at<uchar>(r, c) = Ref.at<uchar>(r, c)*(1 + corr[0])*0.5 + temp3.at<uchar>(r, c)*(1 - corr[0])*0.5;
			fusG.at<uchar>(r, c) = Ref.at<uchar>(r, c)*(1 + corr[1])*0.5 + temp4.at<uchar>(r, c)*(1 - corr[1])*0.5;
			fusB.at<uchar>(r, c) = Ref.at<uchar>(r, c)*(1 + corr[2])*0.5 + temp1.at<uchar>(r, c)*(1 - corr[2])*0.5;
		}
	}

	//vector<Mat> channels;
	//channels.push_back(fusR);
	//channels.push_back(fusG);
	//channels.push_back(fusB);
	//merge(channels, fusRGB);

	ImgGray2RGB(fusB, fusG, fusR, fusRGB);

	//imwrite(".\\image\\sp_yc_Ref.bmp", Ref);
	//imwrite(".\\image\\tm1_resize.bmp", temp1);
	//imwrite(".\\image\\tm3_resize.bmp", temp3);
	//imwrite(".\\image\\tm4_resize.bmp", temp4);

	result = fusRGB;
	return 0;
}


int imgprocess::ImgHISFusion(Mat imgRGB,Mat imgGray,Mat& result){
	Mat RGB = imgRGB;
	//初始转换后图片的存储空间
	Mat HISafter(RGB.rows, RGB.cols, CV_8UC3, Scalar(0, 0, 0));
	//读取高分辨率图像
	Mat HighGraph = imgGray;
	//新建数组存储原图图像数据
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
	//新建Intensity，Hue，Saturation三个数组存储高分辨率图像的强度，色调，饱和度。
	float * Intensity = new float[RGB.rows*RGB.cols];
	float * Hue = new float[RGB.rows*RGB.cols];
	float * Saturation = new float[RGB.rows*RGB.cols];
	
	//对多光谱图像进行HIS变换，变换代码封装在一个函数中
	TransRGB2HIS(RGB, MSData, Intensity, Hue, Saturation);
	//高分辨率图像与I进行直方图匹配，代码封装在一个函数中
	Histogrammatch(Intensity, HighGraph, RGB);
	//对多光谱图像进行RGB变换，变换代码封装在一个函数中
	TransHIS2RGB(RGB, MSData, Intensity, Hue, Saturation);

	//将变换完成后的结果赋予图像
	for (int k = 0; k<3; k++) {
		for (int i = 0; i<RGB.rows; i++)
		{
			for (int j = 0; j<RGB.cols; j++)
			{
				int fTemp = MSData[RGB.cols*i + j + RGB.cols*RGB.rows*k];
				//保证灰阶取值在0-255范围内
				HISafter.at<Vec3b>(i, j)[k] = max(0, min(fTemp, 255));
			}
		}
	}

	//imshow("HIS-Transformation.bmp", HISafter);
	//waitKey(0);
	//imshow("original", imgRGB);
	//waitKey(0);

	result = HISafter;
	return 1;
}

int imgprocess::ImgCloudExtract(Mat R, Mat G, Mat B, Mat & result)
{
	result.create(R.rows, R.cols, CV_8UC1);
	for (int r = 0; r < result.rows; r++)
	{
		for (int c = 0; c < result.cols; c++)
		{
			if (R.at<uchar>(r, c) + G.at<uchar>(r, c) + B.at<uchar>(r, c) > 750)
			{
				result.at<uchar>(r, c) = 255;
			}
			else
			{
				result.at<uchar>(r, c) = 0;
			}
		}
	}
	Mat temp;
	temp.create(result.rows, result.cols, CV_8UC1);
	Mat SE = getStructuringElement(MORPH_ELLIPSE, Size(10, 10));
	erode(result, temp, SE);
	dilate(temp, result, SE);
	return 0;
}

int imgprocess::ImgWaterExtract(Mat G, Mat NIR, Mat & result)
{
	Mat NDWI;
	NDWI.create(G.rows, G.cols, CV_32FC1);
	result.create(G.rows, G.cols, CV_8UC1);
	for (int r = 0; r < G.rows; r++)
	{
		for (int c = 0; c < G.cols; c++)
		{
			NDWI.at<float>(r, c) = (float)(G.at<uchar>(r, c) - NIR.at<uchar>(r, c)) / (G.at<uchar>(r, c) + NIR.at<uchar>(r, c));
			if (NDWI.at<float>(r, c)>0.35)
			{
				result.at<uchar>(r, c) = 255;
			}
			else
			{
				result.at<uchar>(r, c) = 0;
			}
		}
	}

	return 0;
}

void imgprocess::ImgGray2RGB(Mat B, Mat G, Mat R, Mat & RGB)
{
	vector<Mat> channels;
	channels.push_back(R);
	channels.push_back(G);
	channels.push_back(B);
	merge(channels, RGB);
}


void TransRGB2HIS(Mat srcImg, float * RGB, float* Intensity, float* Hue, float* Saturation)
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

void TransHIS2RGB(Mat M0, float * RGB, float* Intensity, float* Hue, float*Saturation)
{
	float R, G, B;
	float I, H, S;
	int height = M0.rows;
	int width = M0.cols;

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

float corrcoef(Mat a, Mat b) {
	if (a.rows != b.rows)
	{
		printf_s("a!=b\n");
		return 0;
	}
	float total = a.rows*a.cols;
	float asum = 0, bsum = 0, a2sum = 0, b2sum = 0, absum = 0, result = 0;
	for (int r = 0; r < a.rows; r++)
	{
		for (int c = 0; c < a.cols; c++)
		{
			asum += a.at<uchar>(r, c);
			bsum += b.at<uchar>(r, c);
			a2sum += a.at<uchar>(r, c) * a.at<uchar>(r, c);
			b2sum += b.at<uchar>(r, c) * b.at<uchar>(r, c);
			absum += a.at<uchar>(r, c) * b.at<uchar>(r, c);
		}
	}

	result = (absum - asum * bsum / total) * fastinvSqrt((a2sum - asum * asum / total)*(b2sum - bsum * bsum / total));
	return result;
}


float fastinvSqrt(float x)
{
	float xhalf = 0.5 * x;
	int i = *(int*)&x; // get bits for floating value
	i = 0x5f3759df - (i >> 1); // gives initial guess
	x = *(float*)&i; // convert bits back to float
	x = x * (1.5 - xhalf * x * x); // Newton step
	return x;
}

