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
	const float radiusOfSphereH = boundingRectForMask.height / 2.0f;
	const float radiusOfSphereW = boundingRectForMask.width / 2.0f;

	cout << "MY Actual center of radius " << boundingRectForMask.height / 2.0f << endl;
	cout << "MY Actual center of radius  width " << boundingRectForMask.width / 2.0f << endl;
	Mat Binary;
	threshold(Image, Binary, THRESH, 255, CV_THRESH_BINARY);
	Mat SubImage(Binary, boundingRectForMask);
	
	/* 
	calculate center of pixels by using moments from OpenCV. 
	Moments(m00 , m01...) returns the centre of mass of the white pixel against the black pixel
	Basically moments calculates m00 which is the number of white pixels in a black and white image.
	m10 is the sum of xcoordinates of white pixels and m01 is sum of ycoordinates of white pixels.
	*/
	Moments centerOfMassWhitePixel = moments(SubImage, false);
	Point center(centerOfMassWhitePixel.m10 / centerOfMassWhitePixel.m00, centerOfMassWhitePixel.m01 / centerOfMassWhitePixel.m00);
	cv::circle(SubImage, center, 10, 128);
	namedWindow("Display window", WINDOW_AUTOSIZE);// Create a window for display.
	imshow("Display window", SubImage);
	//center.x = 119.5;
	//center.y = 119.5;
	std::cout << "x " <<center.x << "y " << center.y << "radius" << radiusOfSphereH;
	/* x,y are swapped here because opencv later takes x ,y in row coloumn fashion. This one is normalised*/
	
	float euclidianRadius = sqrt(pow((center.x - radiusOfSphereW), 2) + pow((radiusOfSphereH - center.y), 2));
	/*std::cout << "Euclidian " << euclidianRadius << std::endl;
	float cosineTheta = (center.x - radiusOfSphereW) / euclidianRadius;
	float theta = acos(cosineTheta);
	float sineTheta = sin(theta);
	float xDirection = (cosineTheta * euclidianRadius) / radiusOfSphereW;
	float yDirection = (sineTheta * euclidianRadius) / radiusOfSphereH;
	float zDirection = sqrt(1.0 - pow(xDirection, 2.0) - pow(yDirection, 2.0));*/

	// x,y are swapped here 
	float xDirection = (-center.y + radiusOfSphereW) / (radiusOfSphereW);
	float yDirection = (-center.x + radiusOfSphereH) / (radiusOfSphereW);
	float zDirection = sqrt(1.0 - pow(xDirection, 2.0) - pow(yDirection, 2.0));

	//The above xDirection , yDirection ,zDirection gave us the Normal. Hence the following calculation (using reflection formula) to get the actual light direction.
	// Using L = 2(N dot R)*N- R where R is the direction of camera. i.e. (0 , 0 , -1) orthogonal
	float x = 2 * xDirection * zDirection;
	float y = 2 * yDirection * zDirection;
	float z = 2*pow(zDirection, 2) - 1;
	return Vec3f(x, y, z);
}