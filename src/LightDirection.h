#pragma once
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
class LightDirection
{
private:
	cv::Mat Mask;
	Rect boundingRectForMask;
public:
	LightDirection(cv::Mat Mask);
	~LightDirection();
	cv::Rect getBoundingRectForMask();
	cv::Vec3f LightDirection::lightDirectionFromMirrorBall(Mat Image);
};