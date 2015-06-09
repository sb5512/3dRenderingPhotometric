#include "PhotometricStereo.h"


PhotometricStereo::PhotometricStereo(const int numberOfImages, const string sourceForLightImages, const string modelImagesSource)
: NUM_IMGS(numberOfImages)
, CALIBRATION(sourceForLightImages), MODEL(modelImagesSource)
{
	initialiseImagesANDLightdirection();
	calculateNormalAndPQGradient();
}


PhotometricStereo::~PhotometricStereo()
{
}


void PhotometricStereo::initialiseImagesANDLightdirection(){
	Mat Mask = imread(CALIBRATION + "mask.png", CV_LOAD_IMAGE_GRAYSCALE);

	Mat ModelMask = imread(MODEL + "mask.png", CV_LOAD_IMAGE_GRAYSCALE);
	Mat Lights(NUM_IMGS, 3, CV_32F);
	// Here the bounding box is already calculated inside this class
	LightDirection lightDirectionObj = LightDirection(Mask);

	for (int i = 0; i < NUM_IMGS; i++) {
		Mat Calib = imread(CALIBRATION + to_string(i) + ".png",
			CV_LOAD_IMAGE_GRAYSCALE);
		//Trying to load grayscale image
		Mat tmp2 = imread(MODEL + to_string(i) + ".png",
			CV_LOAD_IMAGE_GRAYSCALE);

		//Trying to load colour image
	    Mat tmp = imread(MODEL + to_string(i) + ".png");

		cv::Mat Model;
		cv::Mat ModelGrey;
		tmp.copyTo(Model, ModelMask);
		tmp2.copyTo(ModelGrey, ModelMask);
		Vec3f light = lightDirectionObj.lightDirectionFromMirrorBall(Calib);
		Lights.at<float>(i, 0) = light[0];
		Lights.at<float>(i, 1) = light[1];
		Lights.at<float>(i, 2) = light[2];
		cout << " light x - " << light[0] << endl;
		cout << " light y - " << light[1] << endl;
		cout << " light z - " << light[2] << endl;

		calibImages.push_back(Calib);
		modelImages.push_back(Model);
		modelImagesGrey.push_back(ModelGrey);
		//namedWindow("Display", WINDOW_AUTOSIZE);// Create a window for display.
		//imshow("Display", ModelGrey);
	}


	/* Testing peoples
	Lights.at<float>(0, 0) = -0.034;
	Lights.at<float>(0, 1) = 0.498;
	Lights.at<float>(0, 2) = 0.866;

	Lights.at<float>(1, 0) = -0.092;
	Lights.at<float>(1, 1) = 0.287;
	Lights.at<float>(1, 2) = 0.953;

	Lights.at<float>(2, 0) = 0.021;
	Lights.at<float>(2, 1) = -0.357;
	Lights.at<float>(2, 2) = 0.933;

	Lights.at<float>(3, 0) = -0.663;
	Lights.at<float>(3, 1) = 0.128;
	Lights.at<float>(3, 2) = 0.736;

	// TEsting ENDS */


	cv::invert(Lights, this->lightDirection, cv::DECOMP_SVD);
}

void PhotometricStereo::calculateNormalAndPQGradient(){
	const int height = modelImages[0].rows;
	const int width = modelImages[0].cols;
	float sumNormals;
	/* light directions, surface normals, p,q gradients */

	cv::Mat Normals(height, width, CV_32FC3, cv::Scalar::all(0));

	cv::Mat Pgrads(height, width, CV_32F, cv::Scalar::all(0));
	cv::Mat Qgrads(height, width, CV_32F, cv::Scalar::all(0));



	cv::Mat albedo1(height, width, CV_32FC3, cv::Scalar::all(0));
	cv::Mat albedoToSave(height, width, CV_32FC3, cv::Scalar::all(0));
	//modelImages[0].convertTo(albedo1, CV_32FC3 , 1/255.0);

	const int NUM_IMGSS = 4;
	/* estimate surface normals and p,q gradients */
	cout << "image width " << width << endl;
	cout << "image Height "<< height << endl;
	cout << "image width grey" << modelImagesGrey[0].cols << endl;
	cout << "image Height grey" << modelImagesGrey[0].rows << endl;
	for (int albedoCalculation = 0; albedoCalculation < 3; albedoCalculation++){
		for (int x = 0; x < width; x++) {
			for (int y = 0; y < height; y++) {
				Vec<float, NUM_IMGSS> I;  // TODO:.........................
				Vec<float, NUM_IMGSS> I2;
				for (int i = 0; i < NUM_IMGS; i++) {
					if (albedoCalculation == 1){
						// Loading the grey scale image for calculating the normal as we loose data for non white object
						I2[i] = modelImagesGrey[i].at<uchar>(y, x);
					}
					// Trying to load only one channel at a time. initially with Blue and Green and then Red
					I[i] = modelImages[i].at<Vec3b>(Point(x, y))[albedoCalculation];
				}

				cv::Mat n = lightDirection * cv::Mat(I);
				// getting the albedo here 
				float p = sqrt(cv::Mat(n).dot(n));

				if (p > 0) {
					n = n / p;
					albedoToSave.at<Vec3f>(Point(x, y))[albedoCalculation] = p;
					albedo1.at<Vec3f>(Point(x, y))[albedoCalculation] = p/255;
					//cout << endl <<"Albedo p for Blue" << p << endl;
				}
				
				if (albedoCalculation == 1){
					cv::Mat normalAllChannel = lightDirection * cv::Mat(I2);

					float p2 = sqrt(cv::Mat(normalAllChannel).dot(normalAllChannel));

					if (p2 > 0) {
						normalAllChannel = normalAllChannel / p2;
					}
					if (normalAllChannel.at<float>(2, 0) == 0) { normalAllChannel.at<float>(2, 0) = 1.0; }
					if (n.at<float>(2, 0) == 0) { n.at<float>(2, 0) = 1.0; }

					int legit = 1;

					//Note two things to change for only green channel
					/* avoid spikes at edges */
					for (int i = 0; i < NUM_IMGS; i++) {
						//For all Channels
						legit *= modelImagesGrey[i].at<uchar>(Point(x, y)) >= 0;

						//For Green only
						//legit *= modelImages[i].at<uchar>(Point(x, y)) >= 0;
					}
					if (legit) {
						//For all Channels
						Normals.at<cv::Vec3f>(cv::Point(x, y)) = normalAllChannel;
						Pgrads.at<float>(cv::Point(x, y)) = normalAllChannel.at<float>(0, 0) / normalAllChannel.at<float>(2, 0);
						Qgrads.at<float>(cv::Point(x, y)) = normalAllChannel.at<float>(1, 0) / normalAllChannel.at<float>(2, 0);
						/*

						//For Green only
						Normals.at<cv::Vec3f>(cv::Point(x, y)) = n;
						Pgrads.at<float>(cv::Point(x, y)) = n.at<float>(0, 0) / n.at<float>(2, 0);
						Qgrads.at<float>(cv::Point(x, y)) = n.at<float>(1, 0) / n.at<float>(2, 0);*/
						
					}
					else {
						cv::Vec3f nullvec(0.0f, 0.0f, 1.0f);
						Normals.at<cv::Vec3f>(cv::Point(x, y)) = nullvec;
						Pgrads.at<float>(cv::Point(x, y)) = 0.0f;
						Qgrads.at<float>(cv::Point(x, y)) = 0.0f;
					}
				}
			}
		}

		this->normal = Normals;
		this->pGradient = Pgrads;
		this->qGradient = Qgrads;
	}
	//cv::Mat albedo2(height, width, CV_32FC3, cv::Scalar::all(100));
	namedWindow("Albedo1", WINDOW_AUTOSIZE);// Create a window for display.
	imwrite("../images/Albedo.jpg", albedoToSave);
	imshow("Albedo1", albedo1);
}

cv::Mat PhotometricStereo :: globalHeights(cv::Mat Pgrads, cv::Mat Qgrads) {

	cv::Mat P(Pgrads.rows, Pgrads.cols, CV_32FC2, cv::Scalar::all(0));
	cv::Mat Q(Pgrads.rows, Pgrads.cols, CV_32FC2, cv::Scalar::all(0));
	cv::Mat Z(Pgrads.rows, Pgrads.cols, CV_32FC2, cv::Scalar::all(0));

	float lambda = 1.0f;
	float mu = 1.0f;

	//Discrete Fourier Transform that decomposes an image into its sine and cosine components.
	cv::dft(Pgrads, P, cv::DFT_COMPLEX_OUTPUT);
	cv::dft(Qgrads, Q, cv::DFT_COMPLEX_OUTPUT);
	for (int i = 0; i<Pgrads.rows; i++) {
		for (int j = 0; j<Pgrads.cols; j++) {
			if (i != 0 || j != 0) {
				float u = sin((float)(i * 2 * CV_PI / Pgrads.rows));
				float v = sin((float)(j * 2 * CV_PI / Pgrads.cols));

				float uv = pow(u, 2) + pow(v, 2);
				float d = (1.0f + lambda)*uv + mu*pow(uv, 2);
				Z.at<cv::Vec2f>(i, j)[0] = (u*P.at<cv::Vec2f>(i, j)[1] + v*Q.at<cv::Vec2f>(i, j)[1]) / d;
				Z.at<cv::Vec2f>(i, j)[1] = (-u*P.at<cv::Vec2f>(i, j)[0] - v*Q.at<cv::Vec2f>(i, j)[0]) / d;
			}
		}
	}

	/* setting unknown average height to zero */
	Z.at<cv::Vec2f>(0, 0)[0] = 0.0f;
	Z.at<cv::Vec2f>(0, 0)[1] = 0.0f;

	cv::dft(Z, Z, cv::DFT_INVERSE | cv::DFT_SCALE | cv::DFT_REAL_OUTPUT);

	cv::Mat heightMap(Pgrads.rows, Pgrads.cols, CV_32FC2, cv::Scalar::all(0));
	cv::Mat heightMapShow(Pgrads.rows, Pgrads.cols, CV_32FC2, cv::Scalar::all(0));

	cv::normalize(Z, heightMap, 0, 255, cv::NORM_MINMAX);
	cv::normalize(Z, heightMapShow, 0, 1, cv::NORM_MINMAX);
	namedWindow("HeightMap", WINDOW_AUTOSIZE);// Create a window for display.
	imshow("HeightMap", heightMapShow);
	imwrite("../images/HeightMap.jpg", heightMap);
	return Z;
}