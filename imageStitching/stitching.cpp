
#include <iostream>
#include <fstream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/stitching/stitcher.hpp"
#include <opencv2/imgproc/imgproc.hpp>  
#include "Histogram1D.h" 
#include "vector"
#include <cxcore.h>


using namespace std;
using namespace cv;


int main(int argc, char* argv[])
{
	int BufferWriteIndex = 0;
	int ddepth = CV_16S;
	int scale = 1;
	int delta = 0;
	//double MinNum, MaxNum;

	Mat scr_gray, MaskImg_gray;
	Mat src = imread("E9 A_x3.jpg");
	Mat MaskImg = imread("白_x3.jpg");
	int ThresNum,j;
	double t = static_cast<double>(getTickCount());
	cvtColor(src, scr_gray, CV_RGB2GRAY);
	cvtColor(MaskImg, MaskImg_gray, CV_RGB2GRAY);
	Mat DImg;
	subtract(MaskImg_gray, scr_gray, DImg);
	Mat EqualizeImg;
	equalizeHist(DImg, EqualizeImg);
	Histogram1D h;
	cv::MatND histo = h.getHistogram(EqualizeImg);
	vector<Point> num;
	Point histopoint;
	for (int i = 1; i < 256; i++)
	{
		if (histo.at<float>(i) != 0)
		{
			histopoint.x = histo.at<float>(i);
			histopoint.y = i;
			num.push_back(histopoint);
		}
		cout << "Value" << i << "=" << histo.at<float>(i) << endl;
	}
		
	for (int i = 1; i < num.size(); i++)
	{
		if (num[i].x < num[i-1].x)
			if (num[i].x < num[i+1].x)
			{
				ThresNum = num[i].y;
				j = i;
				break;
			}
	}
	//for (int i = 0; i < 256; i++)
	//{
	//	if (num[j] == histo.at<float>(i))
	//	{
	//		ThresNum = i;
	//		break;
	//	}
	//}
	
	Mat ThresholdImg;
	//adaptiveThreshold(EqualizeImg, ThresholdImg, 255.0, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY_INV,35,55);
	threshold(EqualizeImg, ThresholdImg, ThresNum, 255, CV_THRESH_BINARY);
	//subtract(Scalar(255), ThresholdImg, ThresholdImg);
	Mat findrect;
	findrect = ~ThresholdImg;
	vector<vector<cv::Point>> contours;
	cv::findContours(findrect, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	// 寻找最大连通域  
	double maxArea = 0;
	vector<cv::Point> maxContour;
	for (size_t i = 0; i < contours.size(); i++)
	{
		double area = cv::contourArea(contours[i]);
		if (area > maxArea)
		{
			maxArea = area;
			maxContour = contours[i];
		}
	}

	// 将轮廓转为矩形框  
	cv::Rect maxRect = cv::boundingRect(maxContour);
	//CvPoint pt0 = cvPoint(0, 0);
	//CvPoint pt3 = cvPoint(EqualizeImg.cols, EqualizeImg.rows);
	CvPoint pt1 = cvPoint(maxRect.x, maxRect.y);
	CvPoint pt2 = cvPoint(maxRect.x + maxRect.width, maxRect.y + maxRect.height);
	Mat ContourImg = Mat(EqualizeImg.rows, EqualizeImg.cols,CV_8U,Scalar(255));
	rectangle(ContourImg, pt1, pt2, Scalar(0), -1, 8);
	bitwise_or(scr_gray, ContourImg, scr_gray);//清除轨道干扰



	Mat CalImg = scr_gray(Range::all(), Range(1,1));//垂直投影
	CvPoint pt3 = cvPoint(maxRect.x-1,0);
	CvPoint pt4 = cvPoint(maxRect.x + maxRect.width-1, 0);
	CvPoint pt5 = cvPoint(0, 0);
	CvPoint pt6 = cvPoint(EqualizeImg.rows, 0);
    reduce(scr_gray, CalImg, 0, CV_REDUCE_AVG);
	rectangle(CalImg, pt5, pt3, Scalar(0), -1, 8);
	rectangle(CalImg, pt4, pt6, Scalar(0), -1, 8);
	double min, max;
	minMaxLoc(CalImg, &min, &max);
	int LightMax = max;
	Mat LightPlus(CalImg.rows, CalImg.cols,CV_8U,Scalar(LightMax));
	absdiff(LightPlus, CalImg, LightPlus);
	rectangle(LightPlus, pt5, pt3, Scalar(0), -1, 8);
	rectangle(LightPlus, pt4, pt6, Scalar(0), -1, 8);
	Mat Light;
	for (int i = 0; i < 11000; i++)
	{
		Light.push_back(LightPlus);
	}
	add(scr_gray, Light, scr_gray);




	imwrite("DImg.jpg", DImg);
	imwrite("EqualizeImg.jpg", EqualizeImg);
	imwrite("ThresholdImg.jpg", ThresholdImg);
	vector<Point> PointOne;
	vector<Point> PointTwo;
	vector<Point> PointLast;
	int High = ThresholdImg.rows-10;
	
	do{

		Mat DetectImg = ThresholdImg(cv::Rect(0, BufferWriteIndex, ThresholdImg.cols, 3));
		erode(DetectImg, DetectImg, cv::Mat(), cv::Point(-1, -1), 3);
		dilate(DetectImg, DetectImg, cv::Mat());
		Rect recttemp, recttemp1;
		recttemp.x = 0;
		recttemp.y = 0;
		recttemp.width = 100;
		recttemp.height = 3;
		recttemp1.x = 4000;
		recttemp1.y = 0;
		recttemp1.width = 4096 - 4000;
		recttemp1.height = 3;
		rectangle(DetectImg, recttemp, Scalar(255), -1);
		rectangle(DetectImg, recttemp1, Scalar(255), -1);
		
		Mat LineOne = DetectImg(cv::Rect(0, 0, ThresholdImg.cols, 1));
		Mat LineTwo = DetectImg(cv::Rect(0, 1, ThresholdImg.cols, 1));
		Mat LineLast = DetectImg(cv::Rect(0, 2, ThresholdImg.cols, 1));
		
		Mat One, Two, Last;
		bitwise_not(LineOne, One);
		bitwise_not(LineTwo, Two);
		bitwise_not(LineLast, Last);
		int zeronumone, zeronumtwo, zeronumlast;
		if (countNonZero(One) == 0)
			zeronumone = 0;
		else{
			
			findNonZero(One, PointOne);
		}
		
		if (countNonZero(Two) == 0)
			zeronumtwo = 0;
		else{
			
			findNonZero(Two, PointTwo);
		}
		
		if (countNonZero(Last) == 0)
			zeronumlast = 0;
		else{
			
			findNonZero(Last, PointLast);
		}
		
		
		if (PointOne.size() != 0)
		{
			int numone = PointOne.size() - 1;
			if (PointOne[numone].x == PointOne[0].x)
				PointOne[numone].x++;
			Mat lineRectOne = LineOne(cv::Rect(PointOne[0].x, 0, PointOne[numone].x - PointOne[0].x, 1));
			zeronumone = countNonZero(lineRectOne);
		}
		else
			zeronumone = 0;

		if (PointTwo.size() != 0)
		{
			int numtwo = PointTwo.size() - 1;
			if (PointTwo[numtwo].x == PointTwo[0].x)
				PointTwo[numtwo].x++;
			Mat lineRectTwo = LineTwo(cv::Rect(PointTwo[0].x, 0, PointTwo[numtwo].x - PointTwo[0].x, 1));
			zeronumtwo = countNonZero(lineRectTwo);
		}
		else
			zeronumtwo = 0;
		if (PointLast.size() != 0)
		{
			int numlast = PointLast.size() - 1;
			if (PointLast[numlast].x == PointLast[0].x)
				PointLast[numlast].x++;
			Mat lineRectLast = LineLast(cv::Rect(PointLast[0].x, 0, PointLast[numlast].x - PointLast[0].x, 1));
			zeronumlast = countNonZero(lineRectLast);
		}
		else
			zeronumlast = 0;
		//if (Pointone[0].x < Pointtwo[0].x)
		//{
		//	Pointone[0].x = Pointtwo[0].x;
		//	Pointone[0].y = Pointtwo[0].y;
		//}
		//if (Pointone[numone].x > Pointtwo[numtwo].-x)
		//{
		//	Pointone[numone].x = Pointtwo[numtwo].x;
		//	Pointone[numone].y = Pointtwo[numtwo].y;
		//}
		/*Rect rectone,recttwo;*/
		
		
		/*rectone.x = 0;
		rectone.y = 0;
		rectone.width = PointOne[0].x;
		rectone.height = 3;
		recttwo.x = PointOne[numone].x;
		recttwo.y = 0;
		recttwo.width = 4096 - PointOne[numone].x;
		rectangle(LineOne, rectone, Scalar(0), -1);
		rectangle(LineOne, recttwo, Scalar(0), -1);

		rectone.x = 0;
		rectone.y = 0;
		rectone.width = PointTwo[0].x;
		rectone.height = 3;
		recttwo.x = PointTwo[numtwo].x;
		recttwo.y = 0;
		recttwo.width = 4096 - PointTwo[numtwo].x;
		rectangle(LineTwo, rectone, Scalar(0), -1);
		rectangle(LineTwo, recttwo, Scalar(0), -1);

		rectone.x = 0;
		rectone.y = 0;
		rectone.width = PointLast[0].x;
		rectone.height = 3;
		recttwo.x = PointLast[numlast].x;
		recttwo.y = 0;
		recttwo.width = 4096 - PointLast[numlast].x;
		rectangle(LineLast, rectone, Scalar(0), -1);
		rectangle(LineLast, recttwo, Scalar(0), -1);*/
		//Mat element(4, 4, CV_8U, cv::Scalar(1));
		//dilate(DetectImg, DetectImg, cv::Mat(),cv::Point(-1,-1),3);
		//erode(DetectImg, DetectImg, cv::Mat());
		//cv::minMaxIdx(grad, &MinNum, &MaxNum);
		cv::Scalar average,StdDev;
		cv::meanStdDev(DetectImg, average, StdDev);
		int zeronum = zeronumone + zeronumtwo + zeronumlast;
		ofstream fout;
		fout.open("output.txt", ios::app);
		cout << zeronum << endl;
		fout << ("%f", BufferWriteIndex)<<" = "<<("%d", zeronum) << endl;
		//if (StdDev.val[0] > 100)
		//{
		//	/*std::cout << "发现缺陷，行数" << BufferWriteIndex<<"\n";*/
		//	//imwrite("result.jpg", grad);
		//	fout << ("%f", BufferWriteIndex) << endl;
		//}
		PointOne.clear();
		PointTwo.clear();
		PointLast.clear();

		fout.close();
		BufferWriteIndex = BufferWriteIndex+3;
	} while (BufferWriteIndex<(High-2));
	t = static_cast<double>(getTickCount() - t);
	t /= cv::getTickFrequency();
	t = t / 1000;
	cout << "处理时间: " << t << "s" << endl;
	waitKey(0);
	PointOne.clear();
	PointTwo.clear();
	PointLast.clear();
	return 0;
}