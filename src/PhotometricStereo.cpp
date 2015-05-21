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
		Mat tmp = imread(MODEL + to_string(i) + ".png",
			CV_LOAD_IMAGE_GRAYSCALE);
		cv::Mat Model;
		tmp.copyTo(Model, ModelMask);
		Vec3f light = lightDirectionObj.lightDirectionFromMirrorBall(Calib);
		Lights.at<float>(i, 0) = light[0];
		Lights.at<float>(i, 1) = light[1];
		Lights.at<float>(i, 2) = light[2];
		cout << " light x - " << light[0] << endl;
		cout << " light y - " << light[1] << endl;
		cout << " light z - " << light[2] << endl;

		calibImages.push_back(Calib);
		modelImages.push_back(Model);
		//namedWindow("Display windowsss", WINDOW_AUTOSIZE);// Create a window for display.
		//imshow("Display windowss", Model);
	}
	cv::invert(Lights, this->lightDirection, cv::DECOMP_SVD);
}

void PhotometricStereo::calculateNormalAndPQGradient(){
	const int height = calibImages[0].rows;
	const int width = calibImages[0].cols;

	/* light directions, surface normals, p,q gradients */

	cv::Mat Normals(height, width, CV_32FC3, cv::Scalar::all(0));

	cv::Mat Pgrads(height, width, CV_32F, cv::Scalar::all(0));
	cv::Mat Qgrads(height, width, CV_32F, cv::Scalar::all(0));



	cv::Mat albedo1;//(height, width, CV_32FC3, cv::Scalar::all(0));
	modelImages[0].convertTo(albedo1, CV_32FC3 , 1/255.0);
	cv::Mat albedo2;//(height, width, CV_32FC3, cv::Scalar::all(0));
	//modelImages[1].convertTo(albedo2, CV_32FC3, 1 / 255.0);
	cv::Mat albedo3;//(height, width, CV_32FC3, cv::Scalar::all(0));
	//modelImages[2].convertTo(albedo3, CV_32FC3, 1 / 255.0);

	const int NUM_IMGSS = 4;
	/* estimate surface normals and p,q gradients */
	cout << "image width " << width;
	cout << "image Height "<< height;
	for (int x = 0; x<width; x++) {
		for (int y = 0; y<height; y++) {
			Vec<float, NUM_IMGSS> I;  // TODO:.........................
			for (int i = 0; i < NUM_IMGS; i++) {
				I[i] = modelImages[i].at<uchar>(y, x);
			}

			cv::Mat n = lightDirection * cv::Mat(I);
			// getting the albedo here 
			float p = sqrt(cv::Mat(n).dot(n));
			
			if (p > 0) { n = n / p; 
			   //albedo1.at<float>(Point(x, y)) /= p;
			//cout << p << " <<<ppppp" << endl;
			//cout << endl << " picture Red value" << albedo1.at<float>(Point(x, y));
			albedo1.at<float>(Point(x, y)) /= (p/255);
			}

			if (n.at<float>(2, 0) == 0) { n.at<float>(2, 0) = 1.0; }
			int legit = 1;
			/* avoid spikes ad edges */
			for (int i = 0; i < NUM_IMGS; i++) {
				legit *= modelImages[i].at<uchar>(Point(x, y)) >= 0;
			}
			if (legit) {
				Normals.at<cv::Vec3f>(cv::Point(x, y)) = n;
				
				if ((Normals.at<cv::Vec3f>(cv::Point(x, y))[0]<0) || (Normals.at<cv::Vec3f>(cv::Point(x, y))[1] < 0) + (Normals.at<cv::Vec3f>(cv::Point(x, y))[2] <0)){//albedo1.at<float>(Point(x, y)) != 0 ){//&& Normals.at<cv::Vec3f>(cv::Point(x, y))[0] != 1 && Normals.at<cv::Vec3f>(cv::Point(x, y))[0] != 0){// modelImages[0].at<Vec3b>(Point(x, y))[0] != 0 && modelImages[0].at<Vec3b>(Point(x, y))[1] != 0 && modelImages[0].at<Vec3b>(Point(x, y))[2] != 0){
				    //cout << modelImages[0].at<Vec3b>(Point(x, y)) ;
					//cout << "    normal is  " << n;
					//cout << static_cast<unsigned>(Normals.at<uchar>(cv::Point(x, y)));
					//cout <</*Normals.at<cv::Vec3f>(cv::Point(x, y))*/ static_cast<unsigned>(modelImages[0].at<uchar>(Point(x, y))) << endl;
				//cv:Vec3f k = { 0.0, 0.0, 0.0 };
					float average = abs(Normals.at<cv::Vec3f>(cv::Point(x, y))[0]) + abs(Normals.at<cv::Vec3f>(cv::Point(x, y))[1]) + abs(Normals.at<cv::Vec3f>(cv::Point(x, y))[2]);
				    //cout << "albedo " << albedo.at<float>(Point(x, y)) << endl; //= 255;

				   //cout << "normal" << Normals.at<Vec3f>(cv::Point(x, y)) << endl ;
				   //cout << "average normal is " << average / 3.0 <<endl;
				   //if (average >= 0){
					 
					   //albedo2.at<float>(Point(x, y)) /= average;
					   //albedo3.at<float>(Point(x, y)) /= average;


				   //}
				}
				//albedo.at<uchar>(Point(x, y)) = static_cast<unsigned>(modelImages[0].at<uchar>(Point(x, y)));

				if (!((n.at<float>(2, 0) == 1) && (n.at<float>(0, 0) == 0) && (n.at<float>(1, 0) == 0))){
					//cout << "Normals at " << x << " , " << y << "is -> " << n;// .at<float>(0, 0);
				}

				Pgrads.at<float>(cv::Point(x, y)) = n.at<float>(0, 0) / n.at<float>(2, 0);
				Qgrads.at<float>(cv::Point(x, y)) = n.at<float>(1, 0) / n.at<float>(2, 0);
				//cout << "nat0,0 " << n.at<float>(0, 0);
			}
			else {
				cv::Vec3f nullvec(0.0f, 0.0f, 1.0f);
				Normals.at<cv::Vec3f>(cv::Point(x, y)) = nullvec;
				Pgrads.at<float>(cv::Point(x, y)) = 0.0f;
				Qgrads.at<float>(cv::Point(x, y)) = 0.0f;
			}
			
			
		}
		namedWindow("Albedo1", WINDOW_AUTOSIZE);// Create a window for display.
		imshow("Albedo1", albedo1);
		/*
		namedWindow("Albedo2", WINDOW_AUTOSIZE);// Create a window for display.
		imshow("Albedo2", albedo2);
		namedWindow("Albedo3", WINDOW_AUTOSIZE);// Create a window for display.
		imshow("Albedo3", albedo2);*/
		
		this->normal = Normals;
		this->pGradient = Pgrads;
		this->qGradient = Qgrads;
	}
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

	cv::normalize(Z, heightMap, 0, 1, cv::NORM_MINMAX);
	namedWindow("HeightMap", WINDOW_AUTOSIZE);// Create a window for display.
	imshow("HeightMap", heightMap);

	return Z;
}