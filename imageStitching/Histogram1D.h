#pragma once

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/stitching/stitcher.hpp"
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;

class Histogram1D
{
private:
	int histSize[1];
	float hranges[2];
	const float*ranges[1];
	int channels[1];
public:
	Histogram1D()
	{
		histSize[0] = 256;
		hranges[0] = 0.0;
		hranges[1] = 255.0;
		ranges[0] = hranges;
		channels[0] = 0;
	};
	MatND getHistogram(const cv::Mat &image);
	~Histogram1D();
};

