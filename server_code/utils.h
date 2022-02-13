#ifndef __FACE_TRACKER_UTILS_H__
#define __FACE_TRACKER_UTILS_H__


#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>
#include <time.h>
#include <string>
#include <vector>
#include <cv.h>
#include <CLMTracker.h>
#include <PoseDetectorHaar.h>

#define atd at<double>
using namespace std;

static std::string currentDateTime();

class FaceTrackerController{
private:
	cv::Scalar colorSet[12];
	std::string FeatureSet[12];
	// ************* Global Private Variables **************
	int initialImage;
	double eventBufferVel[12][10];
	vector <double [12]> eventBufferVelTime;

	bool fcheck; int fpd; bool show;
	double _Size;int rotate;int skipFrames;int camidx;
	std::string filename;std::string outFileName;
	cv::Mat filteredFeature;cv::Mat eventFeature;
	std::vector<int> wSize1;std::vector<int> wSize2;
	int nIter; double clamp,fTol;
	CLMTracker::TrackerCLM model;cv::Mat trackedShape3d,refShape3d;
	double FeatureLst[10];cv::Mat tri;cv::Mat con;
	double Scale, pitch, yaw, roll, X, Y;
	cv::Mat frame,gray,im; double fps; char sss[256]; 
	std::string text;double Arr[10];int64 t1,t0;
	unsigned long int fnum;bool failed;double FeaturesInArray[12];
	std::ofstream *ofstm;
	// ************************************************
	void Writeblank(std::ofstream &ofstm);
	double DistFromLine(double x1, double y1, double z1,
		double x2, double y2, double z2,
		double x3, double y3, double z3);
	void FeatureList (cv::Mat &TrackedShape,
		cv::Mat &RefShape, cv::Mat &globalPar, double * FeatureLst);
	std::string getResponseString(double events[],
		double filteredFeature[], bool fail);
	void Draw(cv::Mat &image,cv::Mat &shape,cv::Mat &con,
		cv::Mat &tri,cv::Mat &visi);
	cv::Mat getFilterKernel(int windowSize,int ArrLen,
		bool forfilter = true);
	void FilterFeatures(double FeatureLst[],
		cv::Mat &FilteredContent,cv::Mat &Differentiated,
		int ArrLen,int windowSize = 6,float thresHold = 0.05);
	void plotFeatureswithSlidingWindow(double FeatureLst[],
		int ArrLen,double eventFeature[],unsigned long int fnum,
		int rows = 480,int cols = 640,int HorzScale = 4,
		double vertScale = 75.0);
	int parse_cmd(int argc,char** argv);

	std::string detectBEs(double eventFeature[],  bool velocityFlag = true, int ArrLen=12);
	std::string detectBEs2(double eventFeature[],  bool velocityFlag = true, int ArrLen=12, int initialImages=20);
	void EventDetection(double FeatureLst[],cv::Mat &FilteredContent,cv::Mat &Differentiated, 	int ArrLen, char codes[], int windowSize = 6,float thresHold = 0.05);
	void EventDetection1(double FeatureLst[],cv::Mat &FilteredContent,cv::Mat &Differentiated, int ArrLen, char codes[], bool velocityFlag = true,  int windowSize = 6);
	void EventDetection2(double FeatureLst[],cv::Mat &FilteredContent,cv::Mat &Differentiated, int ArrLen, char codes[], bool velocityFlag = true ,  int windowSize = 6, bool fail=false);
	std::string getResponseString(std::string eventString, bool fail);
	cv::Mat rotate_frame(cv::Mat &image, double angle);

	//for new CLM adaptation 
	bool trackingInitialised;
	bool success;
	int frameProc, facesInRow;
	int initTime;

public:
	void initializeValues();
	void initializeValues(int value);
	FaceTrackerController (int argc, char** argv, bool show = true);
	std::string trackFace(cv::Mat frame, float sensorVal=0.0);
	inline void resetFace(){failed = true;model.FrameReset();};
	~FaceTrackerController();

	bool getTrackingInitialised(){return trackingInitialised;};
	void setTrackingInitialised(bool value){trackingInitialised=value;};
	int getFacesInRow(){return facesInRow;};
	void setFacesInRow(int value){facesInRow=value;};

	bool getSuccess(){return success;};
	void setSuccess(bool value){success=value;};

	int getFrameProc(){return frameProc;};
	void setFrameProc(int value){frameProc=value;};
	int getInitTime(){return initTime;};
	void setInitTime(int value){initTime=value;};

	std::string FaceTrackerController::GenerateFeedback(Mat img, CLMWrapper::CLMParameters clmParams,CascadeClassifier classifier,PoseDetectorHaar::PoseDetectorHaarParameters haarParams, string port);
};

#endif