#include <iostream>
#include <vector>
#include <cmath>

#include "Display3DMesh.h"
#include "PhotometricStereo.h"


using namespace cv;
using namespace std;

void rotate(cv::Mat& src, double angle, cv::Mat& dst)
{
	int len = std::max(src.cols, src.rows);
	cv::Point2f pt(len / 2., len / 2.);
	cv::Mat r = cv::getRotationMatrix2D(pt, angle, 1.0);

	cv::warpAffine(src, dst, r, cv::Size(len, len));
}

int main(int argc, char *argv[]) {

	const int NUM_IMGS = 4;
	const string CALIBRATION = "../images/lights/chrome.";
	const string MODEL = "../images/buddha/buddha.";
    

	PhotometricStereo photometric = PhotometricStereo(NUM_IMGS , CALIBRATION , MODEL);

	/*display normals in colour coded form*/
	cv::Mat Normalmap;
	const int height = photometric.normal.rows;
	const int width = photometric.normal.cols;
	cv::cvtColor(photometric.normal, Normalmap, CV_BGR2RGB);
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			Vec3f tmp = Normalmap.at<Vec3f>(Point(x, y));
			float xaa = tmp[1];
			float yaa = tmp[2];
			Normalmap.at<Vec3f>(Point(x, y))[1] = yaa;
			Normalmap.at<Vec3f>(Point(x, y))[2] = -xaa;
		}
	}
	cv::imshow("Normalmap", (Normalmap +1.)/2.);

	
	cv::Mat Z = photometric.globalHeights(photometric.pGradient, photometric.qGradient);

	/* display reconstruction */
	Display3DMesh mesh3D = Display3DMesh(photometric.pGradient.cols,photometric.pGradient.rows, Z);
	mesh3D.display(); 

	cv::waitKey();
	return 0;
}

