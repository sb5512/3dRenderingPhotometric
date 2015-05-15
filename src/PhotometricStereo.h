#pragma once
#include <iostream>
#include <vector>
#include <cmath>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "LightDirection.h"

using namespace cv;
using namespace std;

class PhotometricStereo
{
private:
	const int NUM_IMGS;
	const string CALIBRATION;
	const string MODEL;
    vector<Mat> calibImages;
	vector<Mat> modelImages;
	Mat lightDirection;
	void calculateNormalAndPQGradient();
	void initialiseImagesANDLightdirection();
public:
	PhotometricStereo(const int numberOfImages, const std::string sourceForLightImages, const string modelImagesSource);
	~PhotometricStereo();

	cv::Mat globalHeights(cv::Mat Pgrads, cv::Mat Qgrads); /* global integration of surface normals */

	Mat normal, pGradient, qGradient;
};