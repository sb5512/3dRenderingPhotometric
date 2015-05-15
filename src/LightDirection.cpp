#include "LightDirection.h"

LightDirection::LightDirection(cv::Mat Mask)
{
	this->Mask = Mask;
	this->boundingRectForMask = getBoundingRectForMask();
}


LightDirection::~LightDirection()
{
}

cv::Rect LightDirection::getBoundingRectForMask() {

	std::vector<std::vector<cv::Point> > outputContour;
	cv::findContours(Mask.clone(), outputContour, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
	assert(outputContour.size() > 0);
	return cv::boundingRect(outputContour[0]);
}

cv::Vec3f LightDirection::lightDirectionFromMirrorBall(Mat Image){
	const int THRESH = 254;
	const float radiusOfSphere = boundingRectForMask.width / 2.0f;
	namedWindow("Display windows", WINDOW_AUTOSIZE);// Create a window for display.
	imshow("Display windows", Image);
	Mat Binary;
	threshold(Image, Binary, THRESH, 255, CV_THRESH_BINARY);
	Mat SubImage(Binary, boundingRectForMask);
	//namedWindow("Display window", WINDOW_AUTOSIZE);// Create a window for display.
	//imshow("Display window", SubImage);
	/* 
	calculate center of pixels by using moments from OpenCV. 
	Moments(m00 , m01...) returns the centre of mass of the white pixel against the black pixel
	Basically moments calculates m00 which is the number of white pixels in a black and white image.
	m10 is the sum of xcoordinates of white pixels and m01 is sum of ycoordinates of white pixels.
	*/
	Moments centerOfMassWhitePixel = moments(SubImage, false);
	Point center(centerOfMassWhitePixel.m10 / centerOfMassWhitePixel.m00, centerOfMassWhitePixel.m01 / centerOfMassWhitePixel.m00);

	/* x,y are swapped here. This one is normalised*/
	float xDirection = (center.y - radiusOfSphere) / radiusOfSphere;
	float yDirection = (center.x - radiusOfSphere) / radiusOfSphere;
	float zDirection = sqrt(1.0 - pow(xDirection, 2.0) - pow(yDirection, 2.0));

	//The above xDirection , yDirection ,zDirection gave us the Normal. Hence the following calculation (using reflection formula) to get the actual light direction.
	// Using L = 2(N dot R)*N- R where R is the direction of camera. i.e. (0 , 0 , -1) orthogonal
	float x = 2* xDirection * zDirection;
	float y = 2 * yDirection * zDirection;
	float z = 2*pow(xDirection, 2) - 1;
	return Vec3f(x, y, z);
}