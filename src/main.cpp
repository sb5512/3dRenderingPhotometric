#include <iostream>
#include <vector>
#include <cmath>

#include "Display3DMesh.h"
#include "PhotometricStereo.h"


using namespace cv;
using namespace std;

int main(int argc, char *argv[]) {

	const int NUM_IMGS = 4;
	const string CALIBRATION = "../images/lights/lights.";
	const string MODEL = "../images/rock/rock.";
    

	PhotometricStereo photometric = PhotometricStereo(NUM_IMGS , CALIBRATION , MODEL);

	/*display normals in colour coded form*/
	cv::Mat Normalmap;
	cv::cvtColor(photometric.normal, Normalmap, CV_BGR2RGB);
	cv::imshow("Normalmap", Normalmap);

	
	cv::Mat Z = photometric.globalHeights(photometric.pGradient, photometric.qGradient);

	/* display reconstruction */
	Display3DMesh mesh3D = Display3DMesh(photometric.pGradient.cols,photometric.pGradient.rows, Z);
	mesh3D.display(); 

	cv::waitKey();
	return 0;
}