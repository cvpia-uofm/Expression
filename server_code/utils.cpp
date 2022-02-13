#include "utils.h"
#include <iostream>
#include <CLMTracker.h>
#include <PoseDetectorHaar.h>
/*
 Get current date/time, format is YYYY-MM-DD.HH:mm:ss
 
 Main Class of FaceTracker
 */



std::string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    // Visit http://www.cplusplus.com/reference/clibrary/ctime/strftime/
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%Y_%m_%d_%H_%M_%S", &tstruct);

    return buf;
}
//Face Tracker Constructor
FaceTrackerController::FaceTrackerController(int argc,
	char** argv, bool show):fcheck(true),_Size(1),Scale(1),
	rotate(0),camidx(0),filename(""),outFileName(""),fpd(-1),
	nIter(5),clamp(3),fTol(0.01),fps(0),pitch(0.0),yaw(0.0),
	roll(0.0),X(0.0),Y(0.0),fnum(0), failed(true),
	t0(cv::getTickCount()),t1(0),skipFrames(1),
	model(CLMTracker::TrackerCLM("../model/face2.tracker")),
	tri(CLMTracker::IO::LoadTri("../model/face.tri")),
	con(CLMTracker::IO::LoadCon("../model/face.con")){
		parse_cmd(argc,argv);

		wSize1.push_back(7); wSize2.push_back(11);
		wSize2.push_back(9); wSize2.push_back(7);
		
		for(int i=0;i<12;i++)FeaturesInArray[i]=0.0;
		
		colorSet[0] = CV_RGB(255,0,0);	colorSet[1] = CV_RGB(0,255,0);
		colorSet[2] = CV_RGB(0,0,255);	colorSet[3] = CV_RGB(255,250,0);
		colorSet[4] = CV_RGB(0,255,250);colorSet[5] = CV_RGB(250,0,255);
		colorSet[6] = CV_RGB(255,50,0);	colorSet[7] = CV_RGB(0,255,50);
		colorSet[8] = CV_RGB(50,0,255);	colorSet[9] = CV_RGB(255,50,155);
		colorSet[10] = CV_RGB(155,255,50);colorSet[11] = CV_RGB(50,155,255);
		FeatureSet[0] = "Pitch";	FeatureSet[1] = "Yaw  ";
		FeatureSet[2] = "Roll ";	FeatureSet[3] = "inBrL";
		FeatureSet[4] = "otBrL";	FeatureSet[5] = "inBrR";
		FeatureSet[6] = "otBrR";	FeatureSet[7] = "EyeOL";
		FeatureSet[8] = "EyeOR";	FeatureSet[9] = "oLipH";
		FeatureSet[10] = "iLipH";	FeatureSet[11] = "LipCDt";

		model._clm._pdm._M.copyTo(trackedShape3d);
		
		trackedShape3d.copyTo(refShape3d);
		for(int i=0;i<10;i++){FeatureLst[i] = 0.0;Arr[i]=0.0;}
		outFileName = outFileName+"_"+currentDateTime()+".csv";
		ofstm = new std::ofstream(outFileName);
		*ofstm<<"X01.x  ,X02.x  ,X03.x  ,X04.x  ,X05.x  ,"
			"X06.x  ,X07.x  ,X08.x  ,X09.x  ,X10.x  ,X11.x";
		*ofstm<<",X12.x  ,X13.x  ,X14.x  ,X15.x  ,X16.x  ,"
			"X17.x  ,X28.x  ,X29.x  ,X30.x  ,X31.x  ,X01.y";
		*ofstm<<",X02.y  ,X03.y  ,X04.y  ,X05.y  ,X06.y  ,"
			"X07.y  ,X08.y  ,X09.y  ,X10.y  ,X11.y  ,X12.y";
		*ofstm<<",X13.y  ,X14.y  ,X15.y  ,X16.y  ,X17.y  ,"
			"X28.y  ,X29.y  ,X30.y  ,X31.y  ,inBrL  ,otBrL";  
		*ofstm<<",inBrR  ,otBrR  ,EyeOL  ,EyeOR  ,oLipH  ,"
			"iLipH  ,MAngl  ,LpCDt  ,EucldNrm"<<std::endl;
		
		

}


void FaceTrackerController::initializeValues(){ 
	    for (int i=0; i< 12; i++)
			for (int j=0; j< 10; j++)
					eventBufferVel[i][j] = 0.0;
					
		initialImage = 0;
		for(int i=0;i<12;i++)FeaturesInArray[i]=0.0;
}

void FaceTrackerController::initializeValues(int value){ 
	    
					
		initialImage = initialImage - value;
		
}

FaceTrackerController::~FaceTrackerController(){

	delete ofstm;
}

// Public method called from Main taking Two Param
std::string FaceTrackerController::trackFace(cv::Mat frame, float sensorVal){
	//std::string strTemp="";
	//char StrBuffer[256]={'\0'};
	//if(frame.data==NULL)return "";
	//cv::resize(frame,frame,cv::Size(),_Size,_Size);
	//// 0, 90, 180, 270 degrees of rotation
	///*if(rotate==1){
	//	frame=frame.t();
	//	cv::flip(frame,frame,1);
	//}else if(rotate==2)
	//	cv::flip(frame,frame,1);
	//else if(rotate==3){
	//	frame=frame.t();
	//	cv::flip(frame,frame,-1);
	//}*/

	//cv::Mat im = frame;// Copy the frame to im as temporary fram
	//if( sensorVal>0.0)
	//	frame = rotate_frame(frame,sensorVal);// rotates
	//// Convert to grayscale
	//if(frame.channels()!=1)//Check number of color which is the number of Channel 
	//	cv::cvtColor(frame,gray,CV_BGR2GRAY);//converts RGB to Gray scale. Make it One channel
	//else
	//	gray = frame;// 
	//
	//
	////track this image
	//std::vector<int> wSize; 
	//if(failed)wSize = wSize2; else wSize = wSize1; 

	//for (int i=0; i < 12; i++)// feature array initilization
	//		FeaturesInArray[i]=0.0;
	//
	//if(model.Track(gray,wSize,fpd,nIter,clamp,fTol,fcheck) == 0){// tracking face in image gray
	//	
	//	initialImage++;
	//	int idx = model._clm.GetViewIdx(); failed = false;
	//	trackedShape3d = model._clm._pdm._M + model._clm._pdm._V*model._clm._plocal;
	//	FeatureList(trackedShape3d,refShape3d,model._clm._pglobl,FeatureLst);// calculates 10 distances 

	//	//Copy to a array
	//	for (int i=0; i < 8; i++)
	//				FeaturesInArray[i] = (float)FeatureLst[i];// copy features from 0 to 7
	//				FeaturesInArray[8] =(float) FeatureLst[9];// copy 9 to 8 (8 is useless)
	//				FeaturesInArray[9] = (float)model._clm._pglobl.at<double>(1);//pitch
	//				FeaturesInArray[10] = (float)model._clm._pglobl.at<double>(2);//yaw
	//				FeaturesInArray[11] = (float) model._clm._pglobl.at<double>(3);//roll

	//	/*FeaturesInArray[0] = model._clm._pglobl.at<double>(1); // pitch
	//	FeaturesInArray[1] = model._clm._pglobl.at<double>(2); // yaw
	//	FeaturesInArray[2] = model._clm._pglobl.at<double>(3); // roll
	//	FeaturesInArray[3] = FeatureLst[0];	//inBrL
	//	FeaturesInArray[4] = FeatureLst[1]; //oBrL
	//	FeaturesInArray[5] = FeatureLst[2]; //inBrR
	//	FeaturesInArray[6] = FeatureLst[3]; //oBrR
	//	FeaturesInArray[7] = FeatureLst[4]; //EyeOL
	//	FeaturesInArray[8] = FeatureLst[5]; //EyeOR
	//	FeaturesInArray[9] = FeatureLst[6]; //oLipH
	//	FeaturesInArray[10] = FeatureLst[7]; //inLipH
	//	FeaturesInArray[11] = FeatureLst[9]; //LipCDt
	//	*/
	//	bool velocityFlag = true;
	//	if (velocityFlag == true)
	//		for (int i=0; i < 12; i++)
	//				if(i==4 || i==5) FeaturesInArray[i] = (FeaturesInArray[i] /*- initialFeature[i]*/)/0.15;	// sacling to make it bigger /convineint      
	//							else FeaturesInArray[i] = (FeaturesInArray[i] /*- initialFeature[i]*/)/0.20;

	//   
	//	EventDetection2(FeaturesInArray,filteredFeature,eventFeature,12, "", velocityFlag, 6, failed);//calculates velocity arg: Inp features,filter features , velocity featur, length, staitic char , velocity flag, window size, failed ?is able to track
	//	strTemp = detectBEs2((double*)eventFeature.data, velocityFlag, 12, initialImage);// detects expression , yawn arg: velocity, velocity flag , length, initialimage


	//	//FilterFeatures(FeaturesInArray,filteredFeature,eventFeature,12);
	//	//plotFeatureswithSlidingWindow(FeaturesInArray,12,
	//	//	(double*)eventFeature.data,fnum);



	//	/*sprintf(StrBuffer,"%d,%.2f,%.2f,%.2f,%.3f,%.3f, \
	//						%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.2f, \
	//						%.2f,%.2f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f, \
	//						%.3f,%.3f,%.3f\n",
	//						fnum,
	//						filteredFeature.atd(0),
	//						eventFeature.atd(0),
	//						filteredFeature.atd(1),
	//						eventFeature.atd(1),
	//						filteredFeature.atd(2),
	//						eventFeature.atd(2),
	//						filteredFeature.atd(3),
	//						eventFeature.atd(3),
	//						filteredFeature.atd(4),
	//						eventFeature.atd(4),
	//						filteredFeature.atd(5),
	//						eventFeature.atd(5),
	//						filteredFeature.atd(6),
	//						eventFeature.atd(6),
	//						filteredFeature.atd(7),
	//						eventFeature.atd(7),
	//						filteredFeature.atd(8),
	//						eventFeature.atd(8),
	//						filteredFeature.atd(9),
	//						eventFeature.atd(9),
	//						filteredFeature.atd(10),
	//						eventFeature.atd(10),
	//						filteredFeature.atd(11),
	//						eventFeature.atd(11)
	//						);
	//	*ofstm<<StrBuffer;

	//	*/
	//	Draw(frame,model._shape,con,tri,model._clm._visi[0][idx]);
	//}else{
	//	if(show){ 
	//	cv::Mat R(frame,cvRect(0,0,150,50)); R = cv::Scalar(0,0,255);
	//	}
	//	failed = true;
	//	model.FrameReset();
	//	double eventFeatureFailed[12] = {0.0};
	//	strTemp = detectBEs2(eventFeatureFailed, true, 12, initialImage);
	//}

	//if (failed)	*ofstm<<fnum<<"\n";

	////count framerate
	//if(fnum%5 == 0){      
	//	t1 = cv::getTickCount();
	//	fps = 7.50/((double(t1-t0)/cv::getTickFrequency())); 
	//	t0 = t1;
	//}
	//fnum ++;
	////draw framerate on display image 
	//char frameNo[50] = {" "};
	//	if(show){
	//	sprintf(sss,"%d frames/sec",(int)ceil(fps)); text = sss;
	//	cv::putText(frame,text,cv::Point(10,20),
	//		CV_FONT_HERSHEY_SIMPLEX,0.5,CV_RGB(255,255,255),2);
	//	cv::putText(frame,text,cv::Point(10,20),
	//		CV_FONT_HERSHEY_SIMPLEX,0.5,CV_RGB(0,0,0),1);
	//	sprintf(frameNo,"Frame %d",fnum);
	//	cv::putText(frame,frameNo,cv::Point(frame.cols - 100,20),
	//		CV_FONT_HERSHEY_SIMPLEX,0.5,CV_RGB(255,255,255),3);
	//	cv::putText(frame,frameNo,cv::Point(frame.cols - 100,20),
	//		CV_FONT_HERSHEY_SIMPLEX,0.5,CV_RGB(0,0,0),2);
	//	
	//	//std::cout  << "\n:::::::::::::::: "<<fps <<" ::::::::::::::::::\n";

	//	}
	//	
	//	//show image and check for user input
	//	cv::imshow("Input Frames",im);
	//	cv::imshow("Rotated Frames",frame);
	//
	//int c = cvWaitKey(1);
	//if(char(c) == 'r')model.FrameReset();
	//return getResponseString(strTemp,failed);
return "";
}
std::string FaceTrackerController::getResponseString(
	double events[], double filteredFeature[], bool fail){
	char feature[256] = {"\0"};
	int length = 0;

	if(fail)
		length = sprintf(feature,"%s","no face");
	else
		length = sprintf(feature,"%0.2f, %0.2f, %0.2f, %0.0f, %0.0f,"
		" %0.0f, %0.0f, %0.0f",(events[0]==0.0)?filteredFeature[0]:events[0], // 0: pitch
		(events[1]==0.0)?filteredFeature[1]:events[1], // 1: yaw
		(events[2]==0.0)?filteredFeature[2]:events[2], //2: roll
		events[4], //3: Eye Brow
		((events[7]>0.5)||(events[8]>0.5))?1.0:(((events[7]<-0.5)||
		(events[8]<-0.5))?-1.0:0.0), //4: Left Eye
		events[8], //5: Right Eye
		events[10], //6: mouth
		events[11] //7: Lip Corner Distance
	);
	return feature;
}

std::string FaceTrackerController::getResponseString(std::string eventString, bool fail){
	char feature[256] = {"\0"};
	int length = 0;
	strcpy(feature, eventString.c_str());
	
	/*if(fail)
		length = sprintf(feature,"%s","no face");
	else
		strcpy(feature, eventString.c_str());
		*/
	return feature;
}
void FaceTrackerController::Writeblank(std::ofstream &ofstm){

	//printf("inBrL  otBrL  inBrR  otBrR  EyeOL  EyeOR  oLipH  iLipH  MAngl  LpCDt\n");
	double eucldn=0.0;double temp=0.0;
	for (int i=0;i<10;i++){

		ofstm<<",";
	}
	ofstm<<std::endl;
}

/* Find the distance of (x1,y1,z1) to a line formed by the
points (x2,y2,z2) and (x3,y3,z3)*/
double FaceTrackerController::DistFromLine(double x1,
	double y1, double z1,double x2, double y2, double z2,
	double x3, double y3, double z3){

		cv::Point3d X0(x1,y1,z1);
		cv::Point3d X1(x2,y2,z2);
		cv::Point3d X2(x3,y3,z3);

		cv::Point3d num = (X0-X1).cross(X0-X2);
		cv::Point3d den = (X2-X1);

		return cv::norm(num)/cv::norm(den);
}

/* Static Function for getting 10 facial features
% Inner Eye brow (Left)
% Outer Eye Brow (Left)
% Inner Eye Brow (Right)
% Outer Eye Brow (Right)
% Eye Opening (Left)
% Eye Opening (Right)
% Height of outer lip boundary
% Height of inner lip boundary
% Mouth corner angle
% Distance between lip corners */
void FaceTrackerController::FeatureList (cv::Mat &TrackedShape,
	cv::Mat &RefShape, cv::Mat &globalPar, double * FeatureLst){
	// Annotation metadata can be found here
	// https://lh4.googleusercontent.com/-H5d99m0kZmY/Tq3oWDN-WDI/AAAAAAAADGg/nb9__qoesnU/s512/Face-Annotations.png
	// Remember: The point number here = point number in picture - 1


	double temp1=0.0,temp2=0.0; cv::Point2d tempPoint1, tempPoint2;
	// Feature 0 = Inner Eye brow (Left) = Distance of point 22
	// from the line formed by point 42 and 45
	// = abs(y22 + x22*(y45 - y42)/(x42-x45) + x42*(y42-y45)/(x42-x45) - y42)/
	// sqrt(1 + ((y45-y42)/(x42-x45))^2)
	int x = 0,y=66,z=132;
	temp1 = DistFromLine(TrackedShape.atd(x+22),
		TrackedShape.atd(y+22),
		TrackedShape.atd(z+22),
		TrackedShape.atd(x+42),
		TrackedShape.atd(y+42),
		TrackedShape.atd(z+42),
		TrackedShape.atd(x+45),
		TrackedShape.atd(y+45),
		TrackedShape.atd(z+45));
	temp2 = DistFromLine(RefShape.atd(x+22),
		RefShape.atd(y+22),
		RefShape.atd(z+22),
		RefShape.atd(x+42),
		RefShape.atd(y+42),
		RefShape.atd(z+42),
		RefShape.atd(x+45),
		RefShape.atd(y+45),
		RefShape.atd(z+45));
	FeatureLst[0] = temp1/temp2;

	// Feature 1 = Outer Eye brow (Left) = Distance of point 26
	// from the line formed by point 42 and 45
	// = abs(y26 + x26*(y45 - y42)/(x42-x45) + x42*(y42-y45)/(x42-x45) - y42)/
	// sqrt(1 + ((y45-y42)/(x42-x45))^2)
	temp1 = DistFromLine(TrackedShape.atd(x+26),
		TrackedShape.atd(y+26),
		TrackedShape.atd(z+26),
		TrackedShape.atd(x+42),
		TrackedShape.atd(y+42),
		TrackedShape.atd(z+42),
		TrackedShape.atd(x+45),
		TrackedShape.atd(y+45),
		TrackedShape.atd(z+45));
	temp2 = DistFromLine(RefShape.atd(x+26),
		RefShape.atd(y+26),
		RefShape.atd(z+26),
		RefShape.atd(x+42),
		RefShape.atd(y+42),
		RefShape.atd(z+42),
		RefShape.atd(x+45),
		RefShape.atd(y+45),
		RefShape.atd(z+45));
	FeatureLst[1] = temp1/temp2;

	// Feature 2 = Inner Eye brow (Right) = Distance of point 21
	// from the line formed by point 36 and 39
	// = abs(y21 + x21*(y36 - y39)/(x39-x36) + x39*(y43-y36)/(x39-x36) - y39)/
	// sqrt(1 + ((y36-y39)/(x39-x36))^2)
	temp1 = DistFromLine(TrackedShape.atd(x+21),
		TrackedShape.atd(y+21),
		TrackedShape.atd(z+21),
		TrackedShape.atd(x+36),
		TrackedShape.atd(y+36),
		TrackedShape.atd(z+36),
		TrackedShape.atd(x+39),
		TrackedShape.atd(y+39),
		TrackedShape.atd(z+39));
	temp2 = DistFromLine(RefShape.atd(x+21),
		RefShape.atd(y+21),
		RefShape.atd(z+21),
		RefShape.atd(x+36),
		RefShape.atd(y+36),
		RefShape.atd(z+36),
		RefShape.atd(x+39),
		RefShape.atd(y+39),
		RefShape.atd(z+39));
	FeatureLst[2] = temp1/temp2;

	// Feature 3 = Outer Eye brow (Right) = Distance of point 17
	// from the line formed by point 36 and 39
	// = abs(y17 + x17*(y36 - y39)/(x39-x36) + x39*(y43-y36)/(x39-x36) - y39)/
	// sqrt(1 + ((y36-y39)/(x39-x36))^2)
	temp1 = DistFromLine(TrackedShape.atd(x+17),TrackedShape.atd(y+17),TrackedShape.atd(z+17),
		TrackedShape.atd(x+36),TrackedShape.atd(y+36),TrackedShape.atd(z+36),
		TrackedShape.atd(x+39),TrackedShape.atd(y+39),TrackedShape.atd(z+39));
	temp2 = DistFromLine(RefShape.atd(x+17),RefShape.atd(y+17),RefShape.atd(z+17),
		RefShape.atd(x+36),RefShape.atd(y+36),RefShape.atd(z+36),
		RefShape.atd(x+39),RefShape.atd(y+39),RefShape.atd(z+39));
	FeatureLst[3] = temp1/temp2;

	// Feature 4 = Eye Opening (Left) = avg(Distance of point 43 to 47
	// and point 44 to point 46)
	temp1 = 0.5*(cv::norm(cv::Point3d(TrackedShape.atd(x+43),TrackedShape.atd(y+43),TrackedShape.atd(z+43))-
		cv::Point3d(TrackedShape.atd(x+47),TrackedShape.atd(y+47),TrackedShape.atd(z+47)))
		+ cv::norm(cv::Point3d(TrackedShape.atd(x+44),TrackedShape.atd(y+44),TrackedShape.atd(z+44))-
		cv::Point3d(TrackedShape.atd(x+46),TrackedShape.atd(y+46),TrackedShape.atd(z+46))));

	temp2 = 0.5*(cv::norm(cv::Point3d(RefShape.atd(x+43),RefShape.atd(y+43),RefShape.atd(z+43))-
		cv::Point3d(RefShape.atd(x+47),RefShape.atd(y+47),RefShape.atd(z+47)))
		+ cv::norm(cv::Point3d(RefShape.atd(x+44),RefShape.atd(y+44),RefShape.atd(z+44))-
		cv::Point3d(RefShape.atd(x+46),RefShape.atd(y+46),RefShape.atd(z+46))));
	FeatureLst[4] = temp1/temp2;

	// Feature 5 = Eye Opening (Right) = avg(Distance of point 37 to 41
	// and point 38 to point 40)
	temp1 = 0.5*(cv::norm(cv::Point3d(TrackedShape.atd(x+37),TrackedShape.atd(y+37),TrackedShape.atd(z+37))-
		cv::Point3d(TrackedShape.atd(x+41),TrackedShape.atd(y+41),TrackedShape.atd(z+41)))
		+ cv::norm(cv::Point3d(TrackedShape.atd(x+38),TrackedShape.atd(y+38),TrackedShape.atd(z+38))-
		cv::Point3d(TrackedShape.atd(x+40),TrackedShape.atd(y+40),TrackedShape.atd(z+40))));

	temp2 = 0.5*(cv::norm(cv::Point3d(RefShape.atd(x+37),RefShape.atd(y+37),RefShape.atd(z+37))-
		cv::Point3d(RefShape.atd(x+41),RefShape.atd(y+41),RefShape.atd(z+41)))
		+ cv::norm(cv::Point3d(RefShape.atd(x+38),RefShape.atd(y+38),RefShape.atd(z+38))-
		cv::Point3d(RefShape.atd(x+40),RefShape.atd(y+40),RefShape.atd(z+40))));
	FeatureLst[5] = temp1/temp2;

	// Feature 6 = Height of outer lip boundary = avg(dist(50,58)
	//dist(51,57),dist(52,56))
	temp1 = 0.3333333*(cv::norm(cv::Point3d(TrackedShape.atd(x+50),TrackedShape.atd(y+50),TrackedShape.atd(z+50))-
		cv::Point3d(TrackedShape.atd(x+58),TrackedShape.atd(y+58),TrackedShape.atd(z+58)))
		+ cv::norm(cv::Point3d(TrackedShape.atd(x+51),TrackedShape.atd(y+51),TrackedShape.atd(z+51))-
		cv::Point3d(TrackedShape.atd(x+57),TrackedShape.atd(y+57),TrackedShape.atd(z+57)))
		+ cv::norm(cv::Point3d(TrackedShape.atd(x+52),TrackedShape.atd(y+52),TrackedShape.atd(z+52))-
		cv::Point3d(TrackedShape.atd(x+56),TrackedShape.atd(y+56),TrackedShape.atd(z+56))));
	temp2 = 0.3333333*(cv::norm(cv::Point3d(RefShape.atd(x+50),RefShape.atd(y+50),RefShape.atd(z+50))-
		cv::Point3d(RefShape.atd(x+58),RefShape.atd(y+58),RefShape.atd(z+58)))
		+ cv::norm(cv::Point3d(RefShape.atd(x+51),RefShape.atd(y+51),RefShape.atd(z+51))-
		cv::Point3d(RefShape.atd(x+57),RefShape.atd(y+57),RefShape.atd(z+57)))
		+ cv::norm(cv::Point3d(RefShape.atd(x+52),RefShape.atd(y+52),RefShape.atd(z+52))-
		cv::Point3d(RefShape.atd(x+56),RefShape.atd(y+56),RefShape.atd(z+56))));
	FeatureLst[6] = temp1/temp2;

	// Feature 7 = Height of inner lip boundary = avg(dist(60,65)
	//dist(61,64),dist(62,63))
	temp1 = 0.3333333*(cv::norm(cv::Point3d(TrackedShape.atd(x+60),TrackedShape.atd(y+60),TrackedShape.atd(z+60))-
		cv::Point3d(TrackedShape.atd(x+65),TrackedShape.atd(y+65),TrackedShape.atd(z+65)))
		+ cv::norm(cv::Point3d(TrackedShape.atd(x+61),TrackedShape.atd(y+61),TrackedShape.atd(z+61))-
		cv::Point3d(TrackedShape.atd(x+64),TrackedShape.atd(y+64),TrackedShape.atd(z+64)))
		+ cv::norm(cv::Point3d(TrackedShape.atd(x+62),TrackedShape.atd(y+62),TrackedShape.atd(z+62))-
		cv::Point3d(TrackedShape.atd(x+63),TrackedShape.atd(y+63),TrackedShape.atd(z+63))));
	temp2 = 0.3333333*(cv::norm(cv::Point3d(RefShape.atd(x+60),RefShape.atd(y+60),RefShape.atd(z+60))-
		cv::Point3d(RefShape.atd(x+65),RefShape.atd(y+65),RefShape.atd(z+65)))
		+ cv::norm(cv::Point3d(RefShape.atd(x+61),RefShape.atd(y+61),RefShape.atd(z+61))-
		cv::Point3d(RefShape.atd(x+64),RefShape.atd(y+64),RefShape.atd(z+64)))
		+ cv::norm(cv::Point3d(RefShape.atd(x+62),RefShape.atd(y+62),RefShape.atd(z+62))-
		cv::Point3d(RefShape.atd(x+63),RefShape.atd(y+63),RefShape.atd(z+63))));
	FeatureLst[7] = temp1/temp2;

	// Feature 8 = avg angle of the mouth corners

	FeatureLst[8] = 0.0;

	// Feature 9 = Lip Corner Puller =  dist(48,54)
	temp1 = cv::norm(cv::Point3d(TrackedShape.atd(x+48),TrackedShape.atd(y+48),TrackedShape.atd(z+48))-
		cv::Point3d(TrackedShape.atd(x+54),TrackedShape.atd(y+54),TrackedShape.atd(z+54)));
	temp2 = cv::norm(cv::Point3d(RefShape.atd(x+48),RefShape.atd(y+48),RefShape.atd(z+48))-
		cv::Point3d(RefShape.atd(x+54),RefShape.atd(y+54),RefShape.atd(z+54)));
	FeatureLst[9] = (std::pow(temp1,2)/std::pow(temp2,2));

	// Feature 10,11,12,13,14,15 = global parameters
}
//=============================================================================
void FaceTrackerController::Draw(cv::Mat &image,
	cv::Mat &shape,cv::Mat &con,cv::Mat &tri,cv::Mat &visi){
	int i,n = shape.rows/2; cv::Point p1,p2; cv::Scalar c;

	
	//draw triangulation
	c = CV_RGB(0,0,0);
	for(i = 0; i < tri.rows; i++){
		if(visi.at<int>(tri.at<int>(i,0),0) == 0 ||
			visi.at<int>(tri.at<int>(i,1),0) == 0 ||
			visi.at<int>(tri.at<int>(i,2),0) == 0)continue;
		p1 = cv::Point((int)shape.at<double>(tri.at<int>(i,0),0),
			(int)shape.at<double>(tri.at<int>(i,0)+n,0));
		p2 = cv::Point((int)shape.at<double>(tri.at<int>(i,1),0),
			(int)shape.at<double>(tri.at<int>(i,1)+n,0));
		cv::line(image,p1,p2,c);
		p1 = cv::Point((int)shape.at<double>(tri.at<int>(i,0),0),
			(int)shape.at<double>(tri.at<int>(i,0)+n,0));
		p2 = cv::Point((int)shape.at<double>(tri.at<int>(i,2),0),
			(int)shape.at<double>(tri.at<int>(i,2)+n,0));
		cv::line(image,p1,p2,c);
		p1 = cv::Point((int)shape.at<double>(tri.at<int>(i,2),0),
			(int)shape.at<double>(tri.at<int>(i,2)+n,0));
		p2 = cv::Point((int)shape.at<double>(tri.at<int>(i,1),0),
			(int)shape.at<double>(tri.at<int>(i,1)+n,0));
		cv::line(image,p1,p2,c);
	}
	//draw connections
	c = CV_RGB(0,0,255);
	for(i = 0; i < con.cols; i++){
		if(visi.at<int>(con.at<int>(0,i),0) == 0 ||
			visi.at<int>(con.at<int>(1,i),0) == 0)continue;
		p1 = cv::Point((int)shape.at<double>(con.at<int>(0,i),0),
			(int)shape.at<double>(con.at<int>(0,i)+n,0));
		p2 = cv::Point((int)shape.at<double>(con.at<int>(1,i),0),
			(int)shape.at<double>(con.at<int>(1,i)+n,0));
		cv::line(image,p1,p2,c,1);
	}
	//draw points
	for(i = 0; i < n; i++){    
		if(visi.at<int>(i,0) == 0)continue;
		p1 = cv::Point((int)shape.at<double>(i,0),
			(int)shape.at<double>(i+n,0));
		c = CV_RGB(255,0,0); cv::circle(image,p1,2,c);
	}return;
}
//=============================================================================


cv::Mat FaceTrackerController::getFilterKernel(int windowSize,int ArrLen,bool forfilter){
	cv::Mat FilterKernel ;
	if(forfilter){
		FilterKernel = cv::Mat::ones(windowSize,ArrLen,CV_64FC1);
	}else{
		cv::vconcat(-1.0*cv::Mat::ones(windowSize/2,ArrLen,CV_64FC1),
			cv::Mat::ones(windowSize/2,ArrLen,CV_64FC1),FilterKernel);
	}
	FilterKernel = FilterKernel/(double)1 /*FilterKernel.rows*/;
	return FilterKernel;
}

void FaceTrackerController::FilterFeatures(double FeatureLst[],cv::Mat &FilteredContent,cv::Mat &Differentiated,
	int ArrLen,int windowSize,float thresHold){

		cv::Mat tempBuff,dottedData;

		static cv::Mat DatBufferforFilter = cv::Mat::ones(windowSize,ArrLen,CV_64FC1);
		static cv::Mat Kernel = getFilterKernel(windowSize,ArrLen);
		static cv::Mat Kernel_Diff = getFilterKernel(windowSize,ArrLen,false);
		static int SkipEvent[12] = {0};

		// Slides the window horizontally for Filtering
		DatBufferforFilter.rowRange(1,windowSize).copyTo(tempBuff);
		tempBuff.copyTo(DatBufferforFilter.rowRange(0,windowSize-1));
		for(int i=0;i<ArrLen;i++)
			DatBufferforFilter.atd(windowSize-1,i)=FeatureLst[i];

		// Apply the Kernel for Filter
		FilteredContent.create(1,ArrLen,CV_64FC1);
		dottedData = DatBufferforFilter.mul(Kernel);
		for (int i=0;i<ArrLen;i++){
			FilteredContent.atd(0,i) = cv::sum(dottedData.col(i))[0];
		}

		// Apply the Kernel for Filter
		Differentiated.create(1,ArrLen,CV_64FC1);
		dottedData = DatBufferforFilter.mul(Kernel_Diff);
		for (int i=0;i<ArrLen;i++){
			if((cv::sum(dottedData.col(i))[0]>thresHold)&&(SkipEvent[i]==0)){
				Differentiated.atd(0,i) = 1.0;
				SkipEvent[i]=windowSize;
			}else if((cv::sum(dottedData.col(i))[0]<-1.0*thresHold)&&(SkipEvent[i]==0)){
				Differentiated.atd(0,i) = -1.0;
				SkipEvent[i]=windowSize;
			}else
				Differentiated.atd(0,i) = 0.0;
			SkipEvent[i] = (SkipEvent[i]>0?--SkipEvent[i]:0);
		}
}

void FaceTrackerController::plotFeatureswithSlidingWindow(double FeatureLst[],int ArrLen,double eventFeature[],
	unsigned long int fnum, int rows,int cols,int HorzScale,
	double vertScale){

		int BuffLen = cols/HorzScale;
		static cv::Mat DatBuffer = cv::Mat::ones(ArrLen,BuffLen,CV_64FC1);
		static cv::Mat updownEventBuff = cv::Mat::zeros(ArrLen,BuffLen,CV_64FC1);

		// Preparing the Axis
		cv::Mat PlotArea = cv::Mat::zeros(rows,cols,CV_8UC3);
		cv::line(PlotArea,cv::Point(0,rows/2),cv::Point(cols,rows/2),
			CV_RGB(255,255,255),1); // x axis
		cv::line(PlotArea,cv::Point(cols/2,0),cv::Point(cols/2,rows),
			CV_RGB(255,255,255),1); // y axis

		// Slides the window horizontally for displaying image
		cv::Mat tempBuff;
		DatBuffer.colRange(1,BuffLen).copyTo(tempBuff);
		tempBuff.copyTo(DatBuffer.colRange(0,BuffLen-1));
		for(int i=0;i<ArrLen;i++)
			DatBuffer.atd(i,BuffLen-1)=FeatureLst[i];

		// Slides the window horizontally for Marking Events
		updownEventBuff.colRange(1,BuffLen).copyTo(tempBuff);
		tempBuff.copyTo(updownEventBuff.colRange(0,BuffLen-1));
		for(int i=0;i<ArrLen;i++)
			updownEventBuff.atd(i,BuffLen-1)=eventFeature[i];

		//Draw the plots
		double *tempArray = new double[ArrLen];
		for(int i=0;i<ArrLen;i++)tempArray[i]=0.0;
		for(int i=0;i<(BuffLen-HorzScale);i++){
			for(int j=0;j<3;j++)
			{
				cv::line(PlotArea,cv::Point(HorzScale*i,-1*vertScale
					*(int)tempArray[j]+int(rows*1.5/ArrLen)),
					cv::Point(HorzScale*(i+1),-1*vertScale*
					(int)DatBuffer.atd(j,i)+int(rows*1.5/ArrLen)),
					colorSet[j],1,CV_AA);
				tempArray[j] = DatBuffer.atd(j,i);
			}
			for(int j=3;j<ArrLen;j++)
			{
				// Plot lines
				cv::line(PlotArea,cv::Point(HorzScale*i,-1*0.5*
					vertScale*tempArray[j]+int(rows*float(j+1)/ArrLen)),
					cv::Point(HorzScale*(i+1),-1*0.5*vertScale*
					DatBuffer.atd(j,i)+int(rows*float(j+1)/ArrLen)),
					colorSet[j],1,CV_AA);
				tempArray[j] = DatBuffer.atd(j,i);

				// Write Up/Down Events
				if(updownEventBuff.atd(j,i)==1.0){
					cv::putText(PlotArea,"Up",cv::Point(HorzScale*(i+1),
						-1*0.5*vertScale*DatBuffer.atd(j,i)+int(rows*
						float(j+1)/ArrLen)),CV_FONT_HERSHEY_SIMPLEX,0.25,
						colorSet[j]);
				}else if(updownEventBuff.atd(j,i)==-1.0){
					cv::putText(PlotArea,"Down",cv::Point(HorzScale*(i+1),
						-1*0.5*vertScale*DatBuffer.atd(j,i)+int(rows*
						float(j+1)/ArrLen)),CV_FONT_HERSHEY_SIMPLEX,0.25,
						colorSet[j]);
				}
			}
		}


		cv::putText(PlotArea,"Pitch,Yaw,Roll",cv::Point(cols/2,
			int(rows*1.5/ArrLen)),CV_FONT_HERSHEY_SIMPLEX,0.5,CV_RGB(255,255,255));
		for(int j=3;j<ArrLen;j++){
			cv::putText(PlotArea,FeatureSet[j],cv::Point(cols/2,
				int(rows*float(j)/ArrLen)),CV_FONT_HERSHEY_SIMPLEX,0.5,CV_RGB(255,255,255));
		}

		imshow("Plot",PlotArea);
}
int FaceTrackerController::parse_cmd(int argc,char** argv)
{
	int i; fcheck = false; fpd = -1;
	
	for(i = 1; i < argc; i++){
		if(std::strcmp(argv[i],"--check") == 0){fcheck = true; break;}
	}
	if(i >= argc)fcheck = false;
	for(i = 1; i < argc; i++){
		if(std::strcmp(argv[i],"-s") == 0){
			if(argc > i+1)Scale = std::atof(argv[i+1]); else Scale = 1;
			break;
		}
	}
	if(i >= argc)Scale = 1;
	for(i = 1; i < argc; i++){
		if(std::strcmp(argv[i],"-d") == 0){
			if(argc > i+1)fpd = std::atoi(argv[i+1]); else fpd = -1;
			break;
		}
	}
	if(i >= argc)fpd = -1;
	for(i = 1; i < argc; i++){
		if(std::strcmp(argv[i],"-r") == 0){
			if(argc > i+1)rotate = std::atoi(argv[i+1]);
			else rotate = 0;
			break;
		}
	}
	if(i >= argc)rotate = 0;
	for(i = 1; i < argc; i++){
		if(std::strcmp(argv[i],"-i") == 0){
			if(argc > i+1)camidx = std::atoi(argv[i+1]);
			else camidx = 0;
			break;
		}
	}
	if(i >= argc)camidx = 0;
	for(i = 1; i < argc; i++){
		if(std::strcmp(argv[i],"-f") == 0){
			if(argc > i+1)filename =argv[i+1];
			else filename = "test.mp4";
			break;
		}
	}
	if(i >= argc)filename = "test.mp4";
		for(i = 1; i < argc; i++){
		if(std::strcmp(argv[i],"-of") == 0){
			if(argc > i+1)outFileName =argv[i+1];
			else outFileName = "Output";
			break;
		}
	}
	if(i >= argc)outFileName = "Output";
	return 0;
}


void FaceTrackerController::EventDetection(double FeatureLst[],cv::Mat &FilteredContent,cv::Mat &Differentiated, 
	int ArrLen, char codes[], int windowSize ,float thresHold){

		double thresholdSum[12][2] = {{1.0,  1.0},
		{1.25,  1.25},
		{.75,  .75}, //
		{.75,  .75}, //
		{.75,  .750}, //eyeOL
		{.75, .75},  //Eye OR
		{.75, .6},  //OlipH
		{1.0,  .75},  //iLupH
		{.85, .65}, //LpCDT
		{1.75, 1.75}, //pitch
		{1.75, 1.75}, //yaw
		{1.75, 1.75}}; //roll

		cv::Mat tempBuff, dottedData;
		static cv::Mat DatBufferforFilter = cv::Mat::ones(windowSize,ArrLen,CV_64FC1);

		//std::cout << DatBufferforFilter << "\n";
		static cv::Mat Kernel = getFilterKernel(windowSize,ArrLen);

		static cv::Mat Kernel_Diff = getFilterKernel(windowSize,ArrLen,false);
		//std::cout << Kernel_Diff << "\n";



		// Slides the window horizontally for Filtering
		DatBufferforFilter.rowRange(1,windowSize).copyTo(tempBuff);
		//std::cout << tempBuff << "\n";
		tempBuff.copyTo(DatBufferforFilter.rowRange(0,windowSize-1));
		//std::cout << DatBufferforFilter << "\n";
		for(int i=0;i<ArrLen;i++)
			DatBufferforFilter.atd(windowSize-1,i)=FeatureLst[i];

		//std::cout << DatBufferforFilter << "\n";
		// Apply the Kernel for Filter
		FilteredContent.create(1,ArrLen,CV_64FC1);
		//std::cout << FilteredContent << "\n";
		dottedData = DatBufferforFilter.mul(Kernel);
		//std::cout << dottedData << "\n";
		for (int i=0;i<ArrLen;i++){
			FilteredContent.atd(0,i) = cv::sum(dottedData.col(i))[0];
		}

		Differentiated.create(1,ArrLen,CV_64FC1);

		for (int i=0;i < ArrLen;i++){
			if((cv::sum(dottedData.col(i))[0] > thresholdSum[i][0]) /*&& (!strcmp(codes,"1")==0)*/ ){
				Differentiated.atd(0,i) = 1.0;
			}else if((cv::sum(dottedData.col(i))[0] < -1.0*thresholdSum[i][1]) /*&& (!strcmp(codes,"1")==0)*/ ){
				Differentiated.atd(0,i) = -1.0;

			}else
				Differentiated.atd(0,i) = 0.0;

		}
		
}



void FaceTrackerController::EventDetection1(double FeatureLst[],cv::Mat &FilteredContent,cv::Mat &Differentiated, 
	int ArrLen, char codes[], bool velocityFlag,  int windowSize){

		double thresholdSum[12][2] = {{1.0,  1.0},
		{1.25,  1.25},
		{.75,  .75}, //
		{.75,  .75}, //
		{.75,  1.0}, //eyeOL
		{.75, 1.0},  //Eye OR
		{1.0, 1.0},  //OlipH
		{1.0,  1.0},  //iLupH
		{.75, .65}, //LpCDT
		{1.75, 1.75}, //pitch
		{1.75, 1.75}, //yaw
		{1.75, 1.75}}; //roll

		double threshold[12][2] = {{1.0,  1.0},
		{1.025,  1.025},
		{.075,  .075}, //
		{.075,  .075}, //
		{.55,  .55}, //eyeOL
		{.52, .55},  //Eye OR
		{.35, .25},  //OlipH
		{.50,  .50},  //iLupH
		{.35, .35}, //LpCDT
		{.5, .5}, //pitch
		{.75, .75}, //yaw
		{.75, .75}}; //roll

		cv::Mat tempBuff, dottedData;
		static cv::Mat DatBufferforFilter = cv::Mat::ones(windowSize,ArrLen,CV_64FC1);

		
		static cv::Mat Kernel = getFilterKernel(windowSize,ArrLen);
		static cv::Mat Kernel_Diff = getFilterKernel(windowSize,ArrLen,false);
	



		// Slides the window horizontally for Filtering
		DatBufferforFilter.rowRange(1,windowSize).copyTo(tempBuff);
		tempBuff.copyTo(DatBufferforFilter.rowRange(0,windowSize-1));
		for(int i=0;i<ArrLen;i++)
			DatBufferforFilter.atd(windowSize-1,i)=FeatureLst[i];

	
		// Apply the Kernel for Filter
		FilteredContent.create(1,ArrLen,CV_64FC1);
		dottedData = DatBufferforFilter.mul(Kernel);
		//std::cout << dottedData << "\n";
		for (int i=0;i<ArrLen;i++){
			FilteredContent.atd(0,i) = cv::sum(dottedData.col(i))[0];
		}

		Differentiated.create(1,ArrLen,CV_64FC1);

		if (velocityFlag == false)
		{
			for (int i=0;i < ArrLen;i++){
				if((cv::sum(dottedData.col(i))[0] > thresholdSum[i][0]) /*&& (!strcmp(codes,"1")==0)*/ ){
					Differentiated.atd(0,i) = 1.0;
				}else if((cv::sum(dottedData.col(i))[0] < -1.0*thresholdSum[i][1]) /*&& (!strcmp(codes,"1")==0)*/ ){
					Differentiated.atd(0,i) = -1.0;

				}else Differentiated.atd(0,i) = 0.0;
			}

		} else {

			dottedData = DatBufferforFilter.mul(Kernel_Diff);
			for (int i=0;i<ArrLen;i++){
				if(cv::sum(dottedData.col(i))[0]> threshold[i][0]){
					Differentiated.atd(0,i) = 1;
				}else if(cv::sum(dottedData.col(i))[0]<-1.0*threshold[i][1]){
					Differentiated.atd(0,i) = -1;
				}else
					Differentiated.atd(0,i) = 0;
			}
		}

}


void FaceTrackerController::EventDetection2(double FeatureLst2[],cv::Mat &FilteredContent,cv::Mat &Differentiated, 
	int ArrLen, char codes[], bool velocityFlag,  int windowSize, bool fail){

		double thresholdSum[12][2] = {{1.0,  1.0},
		{1.25,  1.25},
		{.75,  .75}, //
		{.75,  .75}, //
		{.75,  1.0}, //eyeOL
		{.75, 1.0},  //Eye OR
		{1.0, 1.0},  //OlipH
		{1.0,  1.0},  //iLupH
		{.75, .65}, //LpCDT
		{1.75, 1.75}, //pitch
		{1.75, 1.75}, //yaw
		{1.75, 1.75}}; //roll

		double threshold[12][2] = {{1.0,  1.0},
		{1.025,  1.025},
		{.075,  .075}, //
		{.075,  .075}, //
		{.55,  .55}, //eyeOL
		{.52, .55},  //Eye OR
		{.35, .25},  //OlipH
		{.50,  .50},  //iLupH
		{.35, .35}, //LpCDT
		{.5, .5}, //pitch
		{.75, .75}, //yaw
		{.75, .75}}; //roll

		cv::Mat tempBuff, dottedData;
		static cv::Mat DatBufferforFilter = cv::Mat::ones(windowSize,ArrLen,CV_64FC1);


		static cv::Mat Kernel = getFilterKernel(windowSize,ArrLen);
		static cv::Mat Kernel_Diff = getFilterKernel(windowSize,ArrLen,false);



		// Slides the window horizontally for Filtering
		DatBufferforFilter.rowRange(1,windowSize).copyTo(tempBuff);
		tempBuff.copyTo(DatBufferforFilter.rowRange(0,windowSize-1));
		for(int i=0;i<ArrLen;i++)
				DatBufferforFilter.atd(windowSize-1,i)=FeatureLst2[i];


		// Apply the Kernel for Filter
		FilteredContent.create(1,ArrLen,CV_64FC1);
		dottedData = DatBufferforFilter.mul(Kernel);
		//std::cout << dottedData << "\n";
		for (int i=0;i<ArrLen;i++){
			FilteredContent.atd(0,i) = cv::sum(dottedData.col(i))[0];
		}

		Differentiated.create(1,ArrLen,CV_64FC1);

		if (velocityFlag == false)
		{
			for (int i=0;i < ArrLen;i++){
				if((cv::sum(dottedData.col(i))[0] > thresholdSum[i][0]) /*&& (!strcmp(codes,"1")==0)*/ ){
					Differentiated.atd(0,i) = 1.0;
				}else if((cv::sum(dottedData.col(i))[0] < -1.0*thresholdSum[i][1]) /*&& (!strcmp(codes,"1")==0)*/ ){
					Differentiated.atd(0,i) = -1.0;

				}else Differentiated.atd(0,i) = 0.0;
			}

		} else {


			if  (!fail)
			{
				dottedData = DatBufferforFilter.mul(Kernel_Diff);
				for (int i=0;i<ArrLen;i++)
					Differentiated.atd(0,i) = cv::sum(dottedData.col(i))[0]; //DatBufferforFilter.atd(windowSize-1,i) - DatBufferforFilter.atd(0,i);				

			}
			else{
				for (int i=0;i<ArrLen;i++) Differentiated.atd(0,i) = 0;	
			}

		}

}


std::string FaceTrackerController:: detectBEs(double eventFeature[],  bool velocityFlag, int ArrLen){
	static int eventBuffer[6][20] = {{0},{0},{0},{0},{0},{0}};
	//static int eventBufferVel[12][20] = {{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0}};
	char eventString[15] = "";
	if (velocityFlag == false)
	{   

		int yawn=0, smile = 0, sleepy=0, pitch=0,  yaw=0, tilt=0; 
		if (eventFeature[6]==1 && eventFeature[8]==-1)		
			yawn = 1;
		if (eventFeature[8]==1)
			smile = 1;
		if (eventFeature[4]==-1 || eventFeature[5]==-1)
			sleepy=1;
		if (eventFeature[9] == 1 || eventFeature[9] == -1 )
			pitch = 1;
		if (eventFeature[10] == 1 || eventFeature[10] == -1 )
			yaw = 1;
		if (eventFeature[11] == 1 || eventFeature[11] == -1 )
			tilt = 1;


		//Sliding buffers
		for (int i=1; i < 20;  i++ )
		{
			eventBuffer[0][i-1] = eventBuffer[0][i];
			eventBuffer[1][i-1] = eventBuffer[1][i];
			eventBuffer[2][i-1] = eventBuffer[2][i];


			eventBuffer[3][i-1] = eventBuffer[3][i];
			eventBuffer[4][i-1] = eventBuffer[4][i];
			eventBuffer[5][i-1] = eventBuffer[5][i];
		}

		//inserting new frame
		eventBuffer[0][19] = yawn;
		eventBuffer[1][19] = smile;
		eventBuffer[2][19] = sleepy;


		eventBuffer[3][19] = pitch;
		eventBuffer[4][19] = yaw;
		eventBuffer[5][19] = tilt;

		//Calculate the sum of buffer 
		int yawnSum = 0, smileSum=0, sleepySum = 0, pitchSum=0,  yawSum = 0, tiltSum = 0;
		for (int i=0; i < 20; i++)
		{
			yawnSum =  yawnSum + eventBuffer[0][i];
			smileSum =  smileSum + eventBuffer[1][i];
			sleepySum =  sleepySum + eventBuffer[2][i];

			pitchSum =  smileSum + eventBuffer[3][i];
			yawSum =  yawSum + eventBuffer[4][i];
			tiltSum =  tiltSum + eventBuffer[5][i];
		}


		//Event Detection 

		if (yawSum > 14)
			strcat(eventString, "yaw");
		else if (pitchSum > 14)
			strcat(eventString, "pitch");
		else if (tiltSum > 14)
			strcat(eventString, "Tilt");
		else if (yawnSum > 6)
			strcat(eventString, "yawn");
		else if (smileSum >  6)
			strcat(eventString, "smile");
		else if (sleepySum > 10)
			strcat(eventString, "sleepy");
		else strcat(eventString, "");
		return eventString;
	
	
	
	}else {

		//Behavioral Expression Detection using velocity
		//Sliding buffers
		for (int i=0;i < 12; i++)
			{for (int j=1; j < 20;  j++ )
				eventBufferVel[i][j-1] = eventBufferVel[i][j];
		}



		//inserting new frame
		for (int i=0;i < 12; i++)
			eventBufferVel[i][19] = eventFeature[i];

		std :: cout << eventFeature[6] << " : " <<  eventFeature[8] << " : " << eventFeature[10] << ":" <<  eventFeature[4] << ":" << eventFeature[5];

		
		int region[12] = {0}, sumEvent[12]={0};
		for (int i=0; i< 12; i++)
		{   
			sumEvent[i] = 0;
			for (int j=0; j < 20; j++)
				sumEvent[i]   =  sumEvent[i] + eventBufferVel[i][j];
		}

		//Open Lip Height ---Index 6
		int i=6;
		if (sumEvent[i]  > 3)
				region[i] = 1;
			else if (sumEvent[i]  < -3)
				region[i] = -1;
			else region[i] = 0;

		// Lip Corner Distance ---Index 8
		i=8;
		if (sumEvent[i]  > 3)
				region[i] = 1;
			else if (sumEvent[i] < -3)
				region[i] = -1;
			else region[i] = 0;

		// Eye Opening Left---Index 4
		i=4;
		if (sumEvent[i]  > 4)
				region[i] = 1;
			else if (sumEvent[i]  < -3)
				region[i] = -1;
			else region[i] = 0;
	    // Eye Opening Left---Index 4
		i=5;
		if (sumEvent[i]  > 4)
				region[i] = 1;
			else if (sumEvent[i] < -4)
				region[i] = -1;
			else region[i] = 0;

		// Yaw Index 10
		i=10;
		if (sumEvent[i]  > 7)
				region[i] = 1;
			else if (sumEvent[i]  < -7)
				region[i] = -1;
			else region[i] = 0;


		int yawn=0, smile = 0, sleepy=0, pitch=0,  yaw=0, tilt=0; 
		if (region[6]==1 && region[8]==-1)		
			strcat(eventString, "yawn"); //yawn = 1;
		else if (region[8]==1 && region[6]==0)
			strcat(eventString, "smile"); //Closed Lip smile = 1;
		else if (region[8]==1 && region[6]==1)
			strcat(eventString, "open smile"); //Open Lip smile = 1;
		else if (region[4]==-1 && region[5]==-1)
			strcat(eventString, "sleepy");  // Sleepy
		else if (region[10]==-1 || region[10]==1)
			strcat(eventString, "Looking Aside");

		//if (strlen(eventString) > 0)
				 std::cout << ":"  << eventString << "\n";
		return eventString;
	}




}
std::string FaceTrackerController:: detectBEs2(double eventFeature[],  bool velocityFlag, int ArrLen, int initialImages){
	static int eventBuffer[6][20] = {{0},{0},{0},{0},{0},{0}};
	//static int eventBufferVel[12][20] = {{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0}};
	char eventString[20] = "";
	if (velocityFlag == false)
	{   

		int yawn=0, smile = 0, sleepy=0, pitch=0,  yaw=0, tilt=0; 
		if (eventFeature[6]==1 && eventFeature[8]==-1)		
			yawn = 1;
		if (eventFeature[8]==1)
			smile = 1;
		if (eventFeature[4]==-1 || eventFeature[5]==-1)
			sleepy=1;
		if (eventFeature[9] == 1 || eventFeature[9] == -1 )
			pitch = 1;
		if (eventFeature[10] == 1 || eventFeature[10] == -1 )
			yaw = 1;
		if (eventFeature[11] == 1 || eventFeature[11] == -1 )
			tilt = 1;


		//Sliding buffers
		for (int i=1; i < 20;  i++ )
		{
			eventBuffer[0][i-1] = eventBuffer[0][i];
			eventBuffer[1][i-1] = eventBuffer[1][i];
			eventBuffer[2][i-1] = eventBuffer[2][i];


			eventBuffer[3][i-1] = eventBuffer[3][i];
			eventBuffer[4][i-1] = eventBuffer[4][i];
			eventBuffer[5][i-1] = eventBuffer[5][i];
		}

		//inserting new frame
		eventBuffer[0][19] = yawn;
		eventBuffer[1][19] = smile;
		eventBuffer[2][19] = sleepy;


		eventBuffer[3][19] = pitch;
		eventBuffer[4][19] = yaw;
		eventBuffer[5][19] = tilt;

		//Calculate the sum of buffer 
		int yawnSum = 0, smileSum=0, sleepySum = 0, pitchSum=0,  yawSum = 0, tiltSum = 0;
		for (int i=0; i < 20; i++)
		{
			yawnSum =  yawnSum + eventBuffer[0][i];
			smileSum =  smileSum + eventBuffer[1][i];
			sleepySum =  sleepySum + eventBuffer[2][i];

			pitchSum =  smileSum + eventBuffer[3][i];
			yawSum =  yawSum + eventBuffer[4][i];
			tiltSum =  tiltSum + eventBuffer[5][i];
		}


		//Event Detection 

		if (yawSum > 14)
			strcat(eventString, "yaw");
		else if (pitchSum > 14)
			strcat(eventString, "pitch");
		else if (tiltSum > 14)
			strcat(eventString, "Tilt");
		else if (yawnSum > 6)
			strcat(eventString, "yawn");
		else if (smileSum >  6)
			strcat(eventString, "smile");
		else if (sleepySum > 10)
			strcat(eventString, "sleepy");
		else strcat(eventString, "");
		return eventString;
	
	
	
	}else {

		//Behavioral Expression Detection using velocity
		//Sliding buffers moves the right column to left
		for (int i=0;i < 12; i++)
		{
			for (int j=1; j < 10;  j++ )
			{
				eventBufferVel[i][j-1] = eventBufferVel[i][j];
			}
		}
			
		



		//inserting new frame
		for (int i=0;i < 12; i++)
			eventBufferVel[i][9] = eventFeature[i];
	
		int region[12] = {0};
		double sumEvent[12]={0};
		//std :: cout<<"printing buffer\n";
		for (int i=0; i< 12; i++)
		{   
			sumEvent[i] = 0.0;
			for (int j=0; j < 10; j++)
			{
				//std :: cout<<eventBufferVel[i][j]<<" |";
				sumEvent[i]   =  sumEvent[i] + eventBufferVel[i][j];
			}

			//std :: cout<<"\n";
		}

		//std :: cout << "EOL:" <<  sumEvent[4] << " |EOR:" << sumEvent[5] <<" |OLH:"<< sumEvent[6] << " |ILH:"<< sumEvent[7]<< " |LCD:" <<  sumEvent[8] << " |Yaw:" << sumEvent[10] <<" |Pit:" <<sumEvent[9] << " |Tilt" << sumEvent[11]; 
		 
		//Open Lip Height ---Index 6
		int i=6;
		if (sumEvent[i]  > 8)
				region[i] = 1;
			else if (sumEvent[i]  < -8)
				region[i] = -1;
			else region[i] = 0;

		// Lip Corner Distance ---Index 8
		i=8;
		if (sumEvent[i]  > 8.5)
				region[i] = 1;
			else if (sumEvent[i] < -7)
				region[i] = -1;
			else region[i] = 0;

		// Eye Opening Left---Index 4
		i=4;
		if (sumEvent[i]  > 14)
				region[i] = 1;
			else if (sumEvent[i]  < -18)
				region[i] = -1;
			else region[i] = 0;
	    // Eye Opening Left---Index 4
		i=5;
		if (sumEvent[i]  > 14)
				region[i] = 1;
			else if (sumEvent[i] < -18)
				region[i] = -1;
			else region[i] = 0;

		// Yaw Index 10
		i=10;
		if (sumEvent[i]  > 6)
				region[i] = 1;
			else if (sumEvent[i]  < -6)
				region[i] = -1;
			else region[i] = 0;

		// Pitch Index 9
		i=9;
		if (sumEvent[i]  > 6)
				region[i] = 1;
			else if (sumEvent[i]  < -6)
				region[i] = -1;
			else region[i] = 0;
		//Head Tilt
		i=11;
		if (sumEvent[i]  > 7)
				region[i] = 1;
			else if (sumEvent[i]  < -7)
				region[i] = -1;
			else region[i] = 0;



		int yawn=0, smile = 0, sleepy=0, pitch=0,  yaw=0, tilt=0; 
		if (region[6]==1 && region[8]==-1)		
			strcat(eventString, "yawn"); //yawn = 1;
		else if (region[8]==1 && region[6]==0)
			strcat(eventString, "smile"); //Closed Lip smile = 1;
		//else if (region[8]==1 && region[6]==1)
		//	strcat(eventString, "open smile"); //Open Lip smile = 1;
		//else if (region[4]==-1 && region[5]==-1)
		//	strcat(eventString, "sleepy");  // Sleepy
		else if (region[10]==-1 )
			 strcat(eventString, "Looking Left");
		else if (region[10]==1)
			strcat(eventString, "Looking Right");
		else if (region[9]==-1)
			strcat(eventString, "Looking Up");
		else if (region[9]==1)
			strcat(eventString, "Looking Down");
	/*	else if (region[11]==1)
			 strcat(eventString, "TiltRight");
		else if (region[11]==-1)
			 strcat(eventString, "TiltLeft");*/
		else strcat(eventString, "");

			bool flag = true;
			for(int i=0; i< 12; i++)
			  if (sumEvent[i] != 0){
			  	  flag = false;
				  break;
		       }
			    
			if (flag)
				strcpy(eventString, "No Face Found");

		if (strlen(eventString) > 0)
		std::cout << "|" << initialImage << ":" << eventString << "\n";
		//char noFace[30] = "No Face";
	
		         
				 if (initialImage > 12) {
					 initialImage = 13;
					 return eventString;
				 } 
				 else if (initialImage <= -4)
					 return eventString;
				 else  return "";
	}




}


cv::Mat FaceTrackerController::rotate_frame(cv::Mat &image, double angle)
{
	cv:: Point2f src_center(image.cols/2.0F, image.rows/2.0F);
	cv::Mat rot_matrix = getRotationMatrix2D(src_center, angle, 1.0);
	cv::Mat rotated_img(cv::Size(image.size().height, image.size().width), image.type());
	warpAffine(image, rotated_img, rot_matrix, image.size());
	//imwrite("original.jpg", image);
	//imwrite("rotated.jpg", rotated_img);
	return rotated_img;
}
std::string FaceTrackerController::GenerateFeedback(Mat img, CLMWrapper::CLMParameters clmParams,CascadeClassifier classifier,PoseDetectorHaar::PoseDetectorHaarParameters haarParams, string port)
{


			if(img.data==NULL)return "";
			cv::resize(img,img,cv::Size(),_Size,_Size);
			if(img.channels()!=1)//Check number of color which is the number of Channel 
				cv::cvtColor(img,img,CV_BGR2GRAY);//converts RGB to Gray scale. Make it One channel
	
		
	
		Mat disp;
		int  dimx = img.cols;
		int dimy = img.rows;
		float cx = dimx / 2.0f;
		float cy = dimy / 2.0f;
		float fx=500;
		float fy=500;
		std::string strTemp="";
		char StrBuffer[256]={'\0'};	
		Mat_<float> depth;
		Mat_<uchar> grayimg;
		//cvtColor(img, gray, CV_BGR2GRAY);
		grayimg=img;
		// Don't resize if it's unneeded
		Mat_<uchar> img_scaled;		
		if(dimx != grayimg.cols || dimy != grayimg.rows)
		{
			resize( grayimg, img_scaled, Size( dimx, dimy ) );
			resize(img, disp, Size( dimx, dimy));
		}
		else
		{
			img_scaled = grayimg;
			disp = img.clone();
		}
		

		// Get depth image
	/*	if(readDepth)
		{
			char* dst = new char[100];
			std::stringstream sstream;
			//sstream << dDir << "\\depth%06d.png";
			sstream << dDir << "\\depth%05d.png";
			sprintf(dst, sstream.str().c_str(), frameProc + 1);
			Mat_<short> dImg = imread(string(dst), -1);
			if(!dImg.empty())
			{
				if(dimx != dImg.cols || dimy != dImg.rows)
				{
					Mat_<short> dImgT;
					resize(dImg, dImgT, Size( dimx, dimy));
					dImgT.convertTo(depth, CV_32F);
				}
				else
				{
					dImg.convertTo(depth, CV_32F);
				}
			}
			else
			{
				cout<< "Can't find depth image" ;
			}
		}*/

		Vec6d poseEstimateHaar;
		Matx66d poseEstimateHaarUncertainty;

		Rect faceRegion;

		// The start place where CLM should start a search (or if it fails, can use the frame detection)
		if(!trackingInitialised || (!success && ( frameProc  % 2 == 0)))
		{
			// The tracker can return multiple head pose observation
			vector<Vec6d> poseEstimatesInitialiser;
			vector<Matx66d> covariancesInitialiser;			
			vector<Rect> regionsInitialiser;

			bool initSuccess = PoseDetectorHaar::InitialisePosesHaar(img_scaled, depth, poseEstimatesInitialiser, covariancesInitialiser, regionsInitialiser, classifier, fx, fy, cx, cy, haarParams);
					
			if(initSuccess)
			{
				if(poseEstimatesInitialiser.size() > 1)
				{
					cout << "ambiguous detection ";
					// keep the closest one (this is a hack for the experiment)
					double best = 10000;
					int bestIndex = -1;
					for( size_t i = 0; i < poseEstimatesInitialiser.size(); ++i)
					{
						cout << poseEstimatesInitialiser[i][2] << " ";
						if(poseEstimatesInitialiser[i][2] < best  && poseEstimatesInitialiser[i][2] > 200)
						{
							bestIndex = i;
							best = poseEstimatesInitialiser[i][2];
						}									
					}
					if(bestIndex != -1)
					{
						cout << endl << "Choosing " << poseEstimatesInitialiser[bestIndex][2] << regionsInitialiser[bestIndex].x << " " << regionsInitialiser[bestIndex].y <<  " " << regionsInitialiser[bestIndex].width << " " <<  regionsInitialiser[bestIndex].height << endl;
						faceRegion = regionsInitialiser[bestIndex];
						
					}
					else
					{
						initSuccess = false;
					}
					
				}
				else
				{	
					faceRegion = regionsInitialiser[0];
				}				
	
				facesInRow++;
				
			}
		}

        // If condition for tracking is met initialise the trackers
		if(!trackingInitialised && facesInRow >= 1)
		{			
			trackingInitialised = CLMWrapper::InitialiseCLM(img_scaled, depth, model, poseEstimateHaar, faceRegion, fx, fy, cx, cy, clmParams);		
			facesInRow = 0;
		}		

		// opencv detector is needed here, if tracking failed reinitialise using it
		//face traced here
		if(trackingInitialised)
		{
			
			success = CLMWrapper::TrackCLM(img_scaled, depth, model, vector<Vec6d>(), vector<Matx66d>(), faceRegion, fx, fy, cx, cy, clmParams);								
		}			
		if(success)
		{			
			clmParams.wSizeCurrent = clmParams.wSizeLarge;
		}
		else
		{
			clmParams.wSizeCurrent = clmParams.wSizeLarge;
		}

		//Vec6d poseEstimateCLM = CLMWrapper::GetPoseCLM(model, fx, fy, cx, cy, clmParams);

						
	
		if(success )			
		{
			initTime++;
		//prev
			if(initTime>1)
			{
				initialImage++;
				int idx = model._clm.GetViewIdx(); failed = false;
				trackedShape3d = model._clm._pdm._M + model._clm._pdm._V*model._clm._plocal;
				FeatureList(trackedShape3d,refShape3d,model._clm._pglobl,FeatureLst);// calculates 10 distances 

				//Copy to a array
				for (int i=0; i < 8; i++)
							FeaturesInArray[i] = (float)FeatureLst[i];// copy features from 0 to 7
							FeaturesInArray[8] =(float) FeatureLst[9];// copy 9 to 8 (8 is useless)
							FeaturesInArray[9] = (float)model._clm._pglobl.at<double>(1);//pitch
							FeaturesInArray[10] = (float)model._clm._pglobl.at<double>(2);//yaw
							FeaturesInArray[11] = (float) model._clm._pglobl.at<double>(3);//roll


				bool velocityFlag = true;
				if (velocityFlag == true)
					for (int i=0; i < 12; i++)
							if(i==4 || i==5) FeaturesInArray[i] = (FeaturesInArray[i] /*- initialFeature[i]*/)/0.15;	// sacling to make it bigger /convineint      
										else FeaturesInArray[i] = (FeaturesInArray[i] /*- initialFeature[i]*/)/0.20;

	   
				EventDetection2(FeaturesInArray,filteredFeature,eventFeature,12, "", velocityFlag, 6, failed);//calculates velocity arg: Inp features,filter features , velocity featur, length, staitic char , velocity flag, window size, failed ?is able to track
				strTemp = detectBEs2((double*)eventFeature.data, velocityFlag, 12, initialImage);// detects expression , yawn arg: velocity, velocity flag , length, initialimage

					//end
	

					// drawing the facial features on the face if tracking is successful
					model._clm._pdm.Draw(disp, model._shape, model._clm._triangulations[idx]);


					string title="Facial Expression:";
					title=title.append(port);
					imshow(title,disp);
			}
			else
			{
				refShape3d= model._clm._pdm._M;
			}
			
		

			//DrawBox(disp, poseEstimateCLM, Scalar(255,0,0), 3, fx, fy, cx, cy);			
		}
		else if(!model._clm._pglobl.empty())
		{			
			//int idx = model._clm.GetViewIdx(); 	
			
			// draw the facial features
			//model._clm._pdm.Draw(disp, model._shape, model._clm._triangulations[idx]);

			// if tracking fails draw a different shaped outline
			//DrawBox(disp, poseEstimateCLM, Scalar(0,0,255), 3, fx, fy, cx, cy);	
		double eventFeatureFailed[12] = {0.0};
		strTemp = detectBEs2(eventFeatureFailed, true, 12, initialImage);
		}
	return strTemp;

}