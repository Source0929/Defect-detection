#include "Histogram1D.h" 

using namespace cv;



//Histogram1D::Histogram1D()
//{
//}

Histogram1D::~Histogram1D()
{
}
MatND Histogram1D::getHistogram(const cv::Mat &image) {

	cv::MatND hist;

	// Compute histogram
	cv::calcHist(&image,
		1,			// histogram of 1 image only
		channels,	// the channel used
		cv::Mat(),	// no mask is used
		hist,		// the resulting histogram
		1,			// it is a 1D histogram
		histSize,	// number of bins
		ranges		// pixel value range
		);

	return hist;
}