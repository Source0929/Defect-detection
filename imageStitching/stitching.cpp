
#include <iostream>
#include <fstream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/stitching/stitcher.hpp"
#include <opencv2/imgproc/imgproc.hpp>  
#include "Histogram1D.h" 
#include "vector"
#include <cxcore.h>
#include <math.h>

using namespace std;
using namespace cv;


bool SortByM1(const Point &v1, const Point &v2)//ע�⣺�������Ĳ���������һ��Ҫ��vector��Ԫ�ص�����һ��  
{
	return v1.x < v2.x;//��������  
}


int main(int argc, char* argv[])
{
	int BufferWriteIndex = 0;
	int ddepth = CV_16S;
	int scale = 1;
	int delta = 0;
	//double MinNum, MaxNum;

	Mat scr_gray, MaskRect_gray;
	Mat src = imread("A9���۰���_x3.jpg");
	Mat MaskImg = imread("��_x3.jpg");
	Mat MaskRect = MaskImg(Rect(0,0,4096, 10000));
	int ThresNum,j = 0;
	double t = static_cast<double>(getTickCount());
	cvtColor(src, scr_gray, CV_RGB2GRAY);
	cvtColor(MaskRect, MaskRect_gray, CV_RGB2GRAY);
	Mat DImg;
	subtract(MaskRect_gray, scr_gray, DImg);
	//bitwise_not(scr_gray, scr_gray);
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
			histopoint.x = int(histo.at<float>(i));
			histopoint.y = i;
			num.push_back(histopoint);
		}
		cout << "Value" << i << "=" << histo.at<float>(i) << endl;
	}
		
	for (int i = 2; i < num.size(); i++)
	{
		if (num[i].x <= num[i - 1].x && num[i - 1].x <= num[i - 2].x)
			if (num[i].x <= num[i + 1].x && num[i + 1].x <= num[i +2].x)
			{
				ThresNum = num[i].y;
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
	Mat DilateImg = getStructuringElement(MORPH_RECT,Size(15,15));
	dilate(ThresholdImg, ThresholdImg, DilateImg);
	erode(ThresholdImg, ThresholdImg, DilateImg);
	erode(ThresholdImg, ThresholdImg, DilateImg);
	dilate(ThresholdImg, ThresholdImg, DilateImg);


	//Mat DilateImgtwo = getStructuringElement(MORPH_RECT, Size(5, 5));
	//dilate(ThresholdImg, ThresholdImg, DilateImgtwo);
	//erode(ThresholdImg, ThresholdImg, DilateImgtwo);

	Mat findrect;
	findrect = ~ThresholdImg;
	vector<vector<cv::Point>> contours;
	cv::findContours(findrect, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	// Ѱ�������ͨ��  
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

	double S = contourArea(maxContour);
	double C = arcLength(maxContour,true);
	double disc = pow(0.5*C, 2) - 4 * S;
	double a1 = (0.5*C + sqrt(disc)) / 2;
	double a2 = (0.5*C - sqrt(disc)) / 2;



	// ������תΪ���ο�  
	cv::Rect maxRect = cv::boundingRect(maxContour);
	//CvPoint pt0 = cvPoint(0, 0);
	//CvPoint pt3 = cvPoint(EqualizeImg.cols, EqualizeImg.rows);
	CvPoint pt1 = cvPoint(maxRect.x, maxRect.y);
	CvPoint pt2 = cvPoint(maxRect.x + maxRect.width, maxRect.y + maxRect.height);
	Mat ContourImg = Mat(EqualizeImg.rows, EqualizeImg.cols,CV_8U,Scalar(255));
	rectangle(ContourImg, pt1, pt2, Scalar(0), -1, 8);
	bitwise_or(scr_gray, ContourImg, scr_gray);//����������
	//CvPoint pt3 = cvPoint(maxRect.x, maxRect.y+400);
	//CvPoint pt4 = cvPoint(maxRect.x + maxRect.width, maxRect.y + maxRect.height-400);
	Mat ImgROI = scr_gray(Rect(pt1, pt2));//Ѱ����ת����
	cv::Point2f center;

	/*Mat findImg(ImgROI.size(), CV_8U, Scalar(255));*/
	
	vector<Point> leftbound;
	Point boundpoint;
	int leftx = 1;
	int rightx = 1000;
	for (int i = 1; i < scr_gray.rows; i=i + 50)
	{
		Mat findImg = ThresholdImg(Rect(leftx,i, rightx,1));
		if (countNonZero(findImg) < rightx - leftx)
		{
			boundpoint.x = leftx+countNonZero(findImg) - 1;
			boundpoint.y = i;
			leftx = boundpoint.x - 1 - 50;
			rightx = boundpoint.x - 1 + 50;
			leftbound.push_back(boundpoint);
		}
	}
	Vec4f lines;
	fitLine(leftbound, lines, CV_DIST_L2 ,0, 0.01,0.01);
	double d = sqrt((double)lines[0] * lines[0] + (double)lines[1] * lines[1]);
	lines[0] /= d;
	lines[1] /= d;
	Point pt5, pt6;
	pt5.x = cvRound(lines[2] + 5000 * lines[0]);
	pt5.y = cvRound(lines[3] + 5000 * lines[1]);
	pt6.x = cvRound(lines[2] - 5000 * lines[0]);
	pt6.y = cvRound(lines[3] - 5000 * lines[1]);
	cvtColor(ThresholdImg, ThresholdImg, CV_GRAY2RGB);
	line(ThresholdImg,pt5,pt6,Scalar(55,100,195),8,CV_AA);




	//CvPoint pt3 = cvPoint(100, ImgROI.rows);
	//CvPoint pt4 = cvPoint(0, ImgROI.rows-100);
	//CvPoint pt5 = cvPoint(ImgROI.cols, ImgROI.rows-150);
	//CvPoint pt6 = cvPoint(ImgROI.cols, ImgROI.rows);

	//Mat SpliROI = ImgROI(Rect(pt4,pt3));
	//Mat MaxImg = SpliROI(Range::all(), Range(1, 1));
	//reduce(ImgROI, MaxImg, 0, CV_REDUCE_MIN);
	//int Colnum = countNonZero(MaxImg);
	//Mat ColImg = SpliROI.colRange(Colnum, Colnum).clone();
	center.x = leftbound.back().x-pt1.x;
	center.y = leftbound.back().y;
	//int AngleH = countNonZero(ZeroROI);
	double angle = (180*atan(lines[1]/lines[0])/CV_PI)-90;  // ��ת�Ƕ�  
	double k = 1; // ���ų߶� 
	
	Mat rotateMat;
	rotateMat = cv::getRotationMatrix2D(center, angle, k);
	Mat rotateImg;
	//bitwise_not(ImgROI, ImgROI);
	warpAffine(ImgROI, rotateImg, rotateMat, ImgROI.size());
	//Mat ManotskImgone = Mat(EqualizeImg.rows, EqualizeImg.cols, CV_8U, Scalar(255));
	//bitwise_or(scr_gray, MaskImgone, scr_gray);
	rotateImg.copyTo(ImgROI);
	/*bitwise_not(ImgROI, ImgROI);*/


	Mat CalImg = ImgROI(Range::all(), Range(1, 1));//��ֱͶӰ�����ս���
	//CvPoint pt3 = cvPoint(maxRect.x-1,0);
	//CvPoint pt4 = cvPoint(maxRect.x + maxRect.width-1, 0);
	//CvPoint pt5 = cvPoint(0, 0);
	//CvPoint pt6 = cvPoint(EqualizeImg.rows, 0);
	reduce(ImgROI, CalImg, 0, CV_REDUCE_AVG);
	//rectangle(CalImg, pt5, pt3, Scalar(0), -1, 8);
	//rectangle(CalImg, pt4, pt6, Scalar(0), -1, 8);
	double min, max;
	minMaxLoc(CalImg, &min, &max);
	int Maxrows = (int)max;
	Mat LightPlusrows(CalImg.rows, CalImg.cols,CV_8U,Scalar(Maxrows));
	absdiff(LightPlusrows, CalImg, LightPlusrows);//�ҳ�����ֵ
	//rectangle(LightPlus, pt5, pt3, Scalar(0), -1, 8);
	//rectangle(LightPlus, pt4, pt6, Scalar(0), -1, 8);
	Mat Light;
	for (int i = 0; i < ImgROI.rows; i++)//������������
	{
		Light.push_back(LightPlusrows);
	}
	//Mat PoccessImg = ImgROI;
	add(ImgROI, Light, ImgROI);//���ղ���




	//Mat CalImgone = ImgROI(Range(1, 1), Range::all());//ˮƽͶӰ�����ս���
	//reduce(ImgROI, CalImgone, 1, CV_REDUCE_AVG);
	//minMaxLoc(CalImgone, &min, &max);
	//int Maxcols = max;
	//Mat LightPluscols(CalImgone.rows, CalImgone.cols, CV_8U, Scalar(Maxcols));
	//absdiff(LightPluscols, CalImgone, LightPluscols);//�ҳ�����ֵ
	//LightPluscols = LightPluscols.t();
	//Mat Lightcols;
	//for (int i = 0; i < ImgROI.cols; i++)//������������
	//{
	//	Lightcols.push_back(LightPluscols);
	//}
	//Lightcols = Lightcols.t();
	////Mat PoccessImg = ImgROI;
	//add(ImgROI, Lightcols, ImgROI);//���ղ���

	Mat UpImg = ImgROI(Rect(0, 0, ImgROI.cols, 400));
	Mat MidImg = ImgROI(Rect(0, 400, ImgROI.cols, ImgROI.rows - 800));
	Mat DownImg = ImgROI(Rect(0, ImgROI.rows - 400, ImgROI.cols, 400));

	Mat CannyImg,MidImgone, MidImgtwo;//��Ե���
	DilateImg = getStructuringElement(MORPH_RECT, Size(5, 5));
	dilate(MidImg, MidImgone, DilateImg);
	erode(MidImg, MidImgtwo, DilateImg);
	subtract(MidImgone, MidImgtwo, CannyImg);
	/*DilateImg = getStructuringElement(MORPH_RECT, Size(10, 10));
	dilate(CannyImg, CannyImg, DilateImg);
	erode(CannyImg, CannyImg, DilateImg);*/
	Canny(CannyImg, CannyImg, 50, 100);
	DilateImg = getStructuringElement(MORPH_RECT, Size(10, 10));
	dilate(CannyImg, CannyImg, DilateImg);
	erode(CannyImg, CannyImg, DilateImg);
	CvPoint pt3 = Point(0, 0);
	CvPoint pt4 = Point(80, CannyImg.rows-1);

	rectangle(CannyImg, pt3, pt4, Scalar(0), -1, 8);
	//CannyImg = ~CannyImg;
	vector<vector<cv::Point>> decontours;
	cv::findContours(CannyImg, decontours, RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	
	vector<vector<cv::Point>> needContour;
	for (size_t i = 0; i < decontours.size(); i++)
	{
		if (decontours[i].size() > 90)
		{
			needContour.push_back(decontours[i]);
		}
	}
	Mat deresult(CannyImg.size(),CV_8U,Scalar(0));
	drawContours(deresult, needContour, -1, Scalar(255), 5);
	Mat Watchresult = deresult.clone();
	cvtColor(Watchresult, Watchresult, CV_GRAY2RGB);

	//for (size_t i = 0; i < needContour.size(); i++)//��ȱ�ݲ��ֱ�ǳ���
	//{
	//	RotatedRect box = minAreaRect(needContour[i]);
	//	Point2f vertex[4];
	//	box.points(vertex);
	//	for (int j = 0; j < 4; j++)
	//	{
	//		line(Watchresult, vertex[j], vertex[(j + 1) % 4], Scalar(0, 0, 255), 8);
	//	}
	//}
	Mat AvgImg;
	for (size_t i = 0; i < needContour.size(); i++)
	{
		Rect box = boundingRect(needContour[i]);
		CvPoint pt7 = cvPoint(box.x, box.y);
		CvPoint pt8 = cvPoint(box.x + box.width, box.y + box.height);
		Mat boxImg = Watchresult(Rect(pt7,pt8));
		Mat boxImgone = boxImg(Range::all(),Range(1,1));
		reduce(boxImg, boxImgone, 0, CV_REDUCE_AVG);
		boxImg.push_back(boxImgone);
		stringstream strstr;
		strstr << i << ".png";
		imwrite(strstr.str(), boxImg);

	}


	//RotatedRect box = minAreaRect(needContour);
	//Point2f vertex[4];
	//box.points(vertex);
	cv::MatND calcHist = h.getHistogram(MidImg);
	Mat calcHistsub(256,1,CV_8U,Scalar(0));
	vector<Point> Histnum;
	for (int i = 0; i < 255; i++)
	{
		histopoint.x = int(calcHist.at<float>(i + 1) - calcHist.at<float>(i));
		histopoint.y = i;
		Histnum.push_back(histopoint);		
	}

	sort(Histnum.begin(), Histnum.end(), SortByM1);
	int nonenum = 0;
	ThresNum = 0;
	Mat ThresROI;
	do 
	{
		j++;
		if (Histnum[Histnum.size() - j].y <= Histnum[Histnum.size() - 1].y)
		{
			ThresNum = Histnum[Histnum.size() - j].y;
			threshold(MidImg, ThresROI, ThresNum, 255, CV_THRESH_BINARY);
		}
		nonenum = countNonZero(ThresROI);
	} while (nonenum< 0.95*MidImg.rows*MidImg.cols);


	/*double MaxVaule = 0;
	double MinVaule = 0;
	Point MaxPoint = Point(0, 0);
	Point MinPoint = Point(0, 0);
	minMaxLoc(calcHistsub, &MinVaule, &MaxVaule, &MinPoint, &MaxPoint,NULL);*/


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
		//	/*std::cout << "����ȱ�ݣ�����" << BufferWriteIndex<<"\n";*/
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
	cout << "����ʱ��: " << t << "s" << endl;
	waitKey(0);
	PointOne.clear();
	PointTwo.clear();
	PointLast.clear();
	return 0;
}                                                 