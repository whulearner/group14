#include "stdafx.h"//VS2017格式
#include "imgprocess.h"

#define PI 3.141592653
#define sqrt3 1.7320508075689

void TransRGB2HIS(Mat srcImg, float * RGB, float* Intensity, float* Hue, float*Saturation);
void TransHIS2RGB(Mat M0, float * RGB, float* Intensity, float* Hue, float*Saturation);
void Histogrammatch(float *pPrincipalImageI, Mat HighData, Mat srcImg);
double corrcoef(vector<double> a, vector<double> b);
float fastinvSqrt(float x);

bool transferimage(Mat image, int x, int y);
Mat zoomimage(Mat image, double kx, double ky);
Mat rotateimage(Mat img, double degree);

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

double corrcoef(vector<double> a, vector<double> b) {
	if (sizeof(a) != sizeof(b))
	{
		printf_s("a!=b\n");
		return 0;
	}
	int atotal = a.size();
	float asum = 0, bsum = 0, a2sum = 0, b2sum = 0, absum = 0, result = 0;
	for (int i = 0; i < atotal; i++)
	{
		asum += a[i];
		bsum += b[i];
		a2sum += a[i] * a[i];
		b2sum += b[i] * b[i];
		absum += a[i] * b[i];
	}
	result = (absum - asum * bsum / atotal) * fastinvSqrt((a2sum - asum * asum / atotal)*(b2sum - bsum * bsum / atotal));
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


//平移
bool transferimage(Mat image, int x, int y) {
	Mat image2;
	image2.create(image.rows, image.cols, CV_8UC3);//创建一个与原图像等长等宽的图像
	unsigned char* p1 = image.data;
	unsigned char* p2 = image2.data;

	imshow("显示图像", image);//显示原图像

	if (x*y > 0) {
		if (x < 0)//当x，y均小于0时，说明是向左下平移
		{
			x = -x;//将x, y统一为正数
			y = -y;
			for (int i = 0; i < image.rows - y; i++)
			{
				for (int j = 0; j < image.cols - x; j++)
				{
					p2[((i + y)*image2.cols + j) * 3] = p1[3 * (i*image.cols + j + x)];//分通道平移
					p2[((i + y)*image2.cols + j) * 3 + 1] = p1[3 * (i*image.cols + j + x) + 1];
					p2[((i + y)*image2.cols + j) * 3 + 2] = p1[3 * (i*image.cols + j + x) + 2];
				}
			}
		}
		else //x, y均大于0，说明是向右上平移
		{
			for (int i = 0; i < image.rows - y; i++)
			{
				for (int j = image.cols - 1; j >= x; j--)
				{
					p2[(i*image2.cols + j) * 3] = p1[3 * ((i + y)*image.cols + j - x)];
					p2[(i*image2.cols + j) * 3 + 1] = p1[3 * ((i + y)*image.cols + j - x) + 1];
					p2[(i*image2.cols + j) * 3 + 2] = p1[3 * ((i + y)*image.cols + j - x) + 2];
				}
			}
		}
	}
	else {
		if (x < 0)//x < 0,y > 0, 说明是向左上平移
		{
			x = -x;//将x, y统一为正数
			for (int i = 0; i < image.rows - y; i++)
			{
				for (int j = 0; j <= image.cols - x; j++)
				{
					p2[(i*image2.cols + j) * 3] = p1[3 * ((i + y)*image.cols + j + x)];
					p2[(i*image2.cols + j) * 3 + 1] = p1[3 * ((i + y)*image.cols + j + x) + 1];
					p2[(i*image2.cols + j) * 3 + 2] = p1[3 * ((i + y)*image.cols + j + x) + 2];
				}
			}
		}
		else//x 〉0,y < 0, 说明是向右下平移
		{
			y = -y;//将x, y统一为正数
			for (int i = image.rows - 1; i >= y; i--)
			{
				for (int j = image.cols - 1; j >= x; j--)
				{
					p2[(i*image2.cols + j) * 3] = p1[3 * ((i - y)*image.cols + j - x)];
					p2[(i*image2.cols + j) * 3 + 1] = p1[3 * ((i - y)*image.cols + j - x) + 1];
					p2[(i*image2.cols + j) * 3 + 2] = p1[3 * ((i - y)*image.cols + j - x) + 2];
				}
			}

		}
	}
	if ((x == 0) & (y == 0))//均为0 时不平移，直接将原图赋给输出图。注意到，x，y只有一个等于0 的情况已经包含在上面的else语句中
	{
		image2 = image;
	}
	imshow("transimgout", image2);
	waitKey(0);
	return 1;
}

//缩放
Mat zoomimage(Mat image, double kx, double ky) {

	int newwidth = image.cols*kx;//定义缩放比率
	int newheight = image.rows*ky;

	Mat image2;
	image2.create(newheight, newwidth, CV_8UC3);
	unsigned char* p1 = image.data;
	unsigned char* p2 = image2.data;
	imshow("显示图像", image);

	for (int i = 0; i < newheight; i++)
	{
		for (int j = 0; j <= newwidth; j++)//分通道最近邻像素采样
		{
			p2[(i*image2.cols + j) * 3] = p1[3 * (((int)(i / ky))*image.cols + (int)(j / kx))];
			p2[(i*image2.cols + j) * 3 + 1] = p1[3 * (((int)(i / ky))*image.cols + (int)(j / kx)) + 1];
			p2[(i*image2.cols + j) * 3 + 2] = p1[3 * (((int)(i / ky))*image.cols + (int)(j / kx)) + 2];
		}
	}

	imshow("zoomimgout", image2);
	waitKey(0);
	return image2;
}


//旋转
Mat rotateimage(Mat img, double degree) {
	int width = img.cols;
	int height = img.rows;
	Mat img2;
	unsigned char* p1 = img.data;

	int tempLength = sqrt((double)width * width + (double)height *height) + 10;//保证原图可以任意角度旋转 
	img2.create(tempLength, tempLength, CV_8UC3);
	unsigned char* p2 = img2.data;


	//逆时针旋转到pdst的中心，其它用0填充
	//pSrc,srcW,srcH原图及其尺寸
	//pDst,dstW,dstH旋转后图像及其尺寸

	unsigned char* pSrc = p1;
	unsigned char* pDst = p2;
	int nchannel = 3;//通道数
	int srcH = img.rows;
	int srcW = img.cols;
	int dstW = tempLength;
	int dstH = tempLength;
	int k;
	double angle = degree * 3.14159265358979 / 180.0;	//旋转角度
	double co = cos(angle);	//余弦
	double si = sin(angle);	//正弦
	int rotateW, rotateH;	//旋转后图像的高宽
	int srcWidthStep = srcW * nchannel;//宽度步长
	int dstWisthStep = dstW * nchannel;
	int x, y;
	int xMin, xMax, yMin, yMax;
	int xOff, yOff;	//偏移
	double xSrc = 0.;
	double ySrc = 0.;	//变换后图像的坐标在原图中的坐标

						//临时变量
	float valueTemp = 0.;
	float a1, a2, a3, a4;

	memset(pDst, 0, dstWisthStep*dstH * sizeof(unsigned char));
	//计算旋转后的坐标范围
	rotateH = srcW * fabs(si) + srcH * fabs(co);
	rotateW = srcW * fabs(co) + srcH * fabs(si);

	//计算偏移
	xOff = dstW / 2;
	yOff = dstH / 2;

	yMin = (dstH - rotateH) / 2.0;
	yMax = yMin + rotateH + 1;	//加1
	xMin = (dstW - rotateW) / 2.0;
	xMax = xMin + rotateW + 1;

	for (y = yMin; y <= yMax; y++)
	{
		for (x = xMin; x <= xMax; x++)
		{
			//求取在原图中的坐标
			ySrc = si * double(x - xOff) + co * double(y - yOff) + double(int(srcH / 2));
			xSrc = co * double(x - xOff) - si * double(y - yOff) + double(int(srcW / 2));

			//如果在原图范围内
			if (ySrc >= 0. && ySrc < srcH - 0.5 && xSrc >= 0. && xSrc < srcW - 0.5)
			{
				//插值
				int xSmall = floor(xSrc);
				int xBig = ceil(xSrc);
				int ySmall = floor(ySrc);
				int yBig = ceil(ySrc);

				for (k = 0; k < nchannel; k++)
				{
					a1 = (xSmall >= 0 && ySmall >= 0 ? pSrc[ySmall*srcWidthStep + xSmall * nchannel + k] : 0);
					a2 = (xBig < srcW && ySmall >= 0 ? pSrc[ySmall*srcWidthStep + xBig * nchannel + k] : 0);
					a3 = (xSmall >= 0 && yBig < srcH ? pSrc[yBig*srcWidthStep + xSmall * nchannel + k] : 0);
					a4 = (xBig < srcW && yBig < srcH ? pSrc[yBig*srcWidthStep + xBig * nchannel + k] : 0);
					double ux = xSrc - xSmall;
					double uy = ySrc - ySmall;
					//双线性插值
					valueTemp = (1 - ux)*(1 - uy)*a1 + (1 - ux)*uy*a3 + (1 - uy)*ux*a2 + ux * uy*a4;
					pDst[y*dstWisthStep + x * nchannel + k] = floor(valueTemp);
				}
			}
		}
	}

	imshow("rotateimgout", img2);
	waitKey(0);
	return img2;
}
