///////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2012, Tadas Baltrusaitis, all rights reserved.
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are met:
//
//     * The software is provided under the terms of this licence stricly for
//       academic, non-commercial, not-for-profit purposes.
//     * Redistributions of source code must retain the above copyright notice, 
//       this list of conditions (licence) and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright 
//       notice, this list of conditions (licence) and the following disclaimer 
//       in the documentation and/or other materials provided with the 
//       distribution.
//     * The name of the author may not be used to endorse or promote products 
//       derived from this software without specific prior written permission.
//     * As this software depends on other libraries, the user must adhere to 
//       and keep in place any licencing terms of those libraries.
//     * Any publications arising from the use of this software, including but
//       not limited to academic journal and conference publications, technical
//       reports and manuals, must cite the following work:
//
//       Tadas Baltrusaitis, Peter Robinson, and Louis-Philippe Morency. 3D
//       Constrained Local Model for Rigid and Non-Rigid Facial Tracking.
//       IEEE Conference on Computer Vision and Pattern Recognition (CVPR), 2012.    
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED 
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO 
// EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF 
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
///////////////////////////////////////////////////////////////////////////////


// SimpleCLM.cpp : Defines the entry point for the console application.

#include <CLMTracker.h>
#include <PoseDetectorHaar.h>

#include <fstream>
#include <sstream>

#include <cv.h>
#include "utils.h"
#include <strstream>
#define INFO_STREAM( stream ) \
std::cout << stream << std::endl

#define WARN_STREAM( stream ) \
std::cout << "Warning: " << stream << std::endl

#define ERROR_STREAM( stream ) \
std::cout << "Error: " << stream << std::endl

static void printErrorAndAbort( const std::string & error )
{
    std::cout << error << std::endl;
    abort();
}

#define FATAL_STREAM( stream ) \
printErrorAndAbort( std::string( "Fatal error: " ) + stream )

using namespace std;
using namespace cv;

// takes in doubles for orientation for added precision, but ultimately returns a float matrix
Matx33f Euler2RotationMatrix(const Vec3d& eulerAngles)
{
	Matx33f rotationMatrix;

	double s1 = sin(eulerAngles[0]);
	double s2 = sin(eulerAngles[1]);
	double s3 = sin(eulerAngles[2]);

	double c1 = cos(eulerAngles[0]);
	double c2 = cos(eulerAngles[1]);
	double c3 = cos(eulerAngles[2]);
  
	rotationMatrix(0,0) = (float)(c2 * c3);
	rotationMatrix(0,1) = (float)(-c2 *s3);
	rotationMatrix(0,2) = (float)(s2);
	rotationMatrix(1,0) = (float)(c1 * s3 + c3 * s1 * s2);
	rotationMatrix(1,1) = (float)(c1 * c3 - s1 * s2 * s3);
	rotationMatrix(1,2) = (float)(-c2 * s1);
	rotationMatrix(2,0) = (float)(s1 * s3 - c1 * c3 * s2);
	rotationMatrix(2,1) = (float)(c3 * s1 + c1 * s2 * s3);
	rotationMatrix(2,2) = (float)(c1 * c2);
	    
	return rotationMatrix;
}

void Project(Mat_<float>& dest, const Mat_<float>& mesh, Size size, double fx, double fy, double cx, double cy)
{
	dest = Mat_<float>(mesh.rows,2, 0.0);

	int NbPoints = mesh.rows;

	register float X, Y, Z;


	Mat_<float>::const_iterator mData = mesh.begin();
	Mat_<float>::iterator projected = dest.begin();

	for(int i = 0;i < NbPoints; i++)
	{
		// Get the points
		X = *(mData++);
		Y = *(mData++);
		Z = *(mData++);
			
		float x;
		float y;

		// if depth is 0 the projection is different
		if(Z != 0)
		{
			x = (float)((X * fx / Z) + cx);
			y = (float)((Y * fy / Z) + cy);
		}
		else
		{
			x = X;
			y = Y;
		}

		// Clamping to image size
		if( x < 0 )	
		{
			x = 0.0;
		}
		else if (x > size.width - 1)
		{
			x = size.width - 1.0f;
		}
		if( y < 0 )
		{
			y = 0.0;
		}
		else if( y > size.height - 1) 
		{
			y = size.height - 1.0f;
		}

		// Project and store in dest matrix
		(*projected++) = x;
		(*projected++) = y;
	}

}

// Need to move this all to opengl
void DrawBox(Mat image, Vec6d pose, Scalar color, int thickness, float fx, float fy, float cx, float cy)
{
	float boxVerts[] = {-1, 1, -1,
						1, 1, -1,
						1, 1, 1,
						-1, 1, 1,
						1, -1, 1,
						1, -1, -1,
						-1, -1, -1,
						-1, -1, 1};
	Mat_<float> box = Mat(8, 3, CV_32F, boxVerts).clone() * 100;


	Matx33f rot = Euler2RotationMatrix(Vec3d(pose[3], pose[4], pose[5]));
	Mat_<float> rotBox;
	
	Mat((Mat(rot) * box.t())).copyTo(rotBox);
	rotBox = rotBox.t();

	rotBox.col(0) = rotBox.col(0) + pose[0];
	rotBox.col(1) = rotBox.col(1) + pose[1];
	rotBox.col(2) = rotBox.col(2) + pose[2];

	// draw the lines
	Mat_<float> rotBoxProj;
	Project(rotBoxProj, rotBox, image.size(), fx, fy, cx, cy);
	
	Mat begin;
	Mat end;

	rotBoxProj.row(0).copyTo(begin);
	rotBoxProj.row(1).copyTo(end);
	cv::line(image, Point((int)begin.at<float>(0), (int)begin.at<float>(1)), Point((int)end.at<float>(0), (int)end.at<float>(1)), color, thickness);
		
	rotBoxProj.row(1).copyTo(begin);
	rotBoxProj.row(2).copyTo(end);
	cv::line(image, Point((int)begin.at<float>(0), (int)begin.at<float>(1)), Point((int)end.at<float>(0), (int)end.at<float>(1)), color, thickness);
	
	rotBoxProj.row(2).copyTo(begin);
	rotBoxProj.row(3).copyTo(end);
	cv::line(image, Point((int)begin.at<float>(0), (int)begin.at<float>(1)), Point((int)end.at<float>(0), (int)end.at<float>(1)), color, thickness);
	
	rotBoxProj.row(0).copyTo(begin);
	rotBoxProj.row(3).copyTo(end);
	//std::cout << begin <<endl;
	//std::cout << end <<endl;
	cv::line(image, Point((int)begin.at<float>(0), (int)begin.at<float>(1)), Point((int)end.at<float>(0), (int)end.at<float>(1)), color, thickness);
	
	rotBoxProj.row(2).copyTo(begin);
	rotBoxProj.row(4).copyTo(end);
	cv::line(image, Point((int)begin.at<float>(0), (int)begin.at<float>(1)), Point((int)end.at<float>(0), (int)end.at<float>(1)), color, thickness);
	
	rotBoxProj.row(1).copyTo(begin);
	rotBoxProj.row(5).copyTo(end);
	cv::line(image, Point((int)begin.at<float>(0), (int)begin.at<float>(1)), Point((int)end.at<float>(0), (int)end.at<float>(1)), color, thickness);
	
	rotBoxProj.row(0).copyTo(begin);
	rotBoxProj.row(6).copyTo(end);
	cv::line(image, Point((int)begin.at<float>(0), (int)begin.at<float>(1)), Point((int)end.at<float>(0), (int)end.at<float>(1)), color, thickness);
	
	rotBoxProj.row(3).copyTo(begin);
	rotBoxProj.row(7).copyTo(end);
	cv::line(image, Point((int)begin.at<float>(0), (int)begin.at<float>(1)), Point((int)end.at<float>(0), (int)end.at<float>(1)), color, thickness);
	
	rotBoxProj.row(6).copyTo(begin);
	rotBoxProj.row(5).copyTo(end);
	cv::line(image, Point((int)begin.at<float>(0), (int)begin.at<float>(1)), Point((int)end.at<float>(0), (int)end.at<float>(1)), color, thickness);
	
	rotBoxProj.row(5).copyTo(begin);
	rotBoxProj.row(4).copyTo(end);
	cv::line(image, Point((int)begin.at<float>(0), (int)begin.at<float>(1)), Point((int)end.at<float>(0), (int)end.at<float>(1)), color, thickness);
		
	rotBoxProj.row(4).copyTo(begin);
	rotBoxProj.row(7).copyTo(end);
	cv::line(image, Point((int)begin.at<float>(0), (int)begin.at<float>(1)), Point((int)end.at<float>(0), (int)end.at<float>(1)), color, thickness);
	
	rotBoxProj.row(7).copyTo(begin);
	rotBoxProj.row(6).copyTo(end);
	cv::line(image, Point((int)begin.at<float>(0), (int)begin.at<float>(1)), Point((int)end.at<float>(0), (int)end.at<float>(1)), color, thickness);
	

}
int main_mkm(int argc, char **argv)
{

	for (int i=450;i<=700;i=i+25)
	{
		for (int j=2; j<=5;j++)
		{
			char buffer [300],bufferCounter[300] ;
			ofstream   fl,flCounter;
			int size=i;
			int frate=j;
			//sprintf(buffer,"%s%d_%d%s","D:/EECE2201/Spring 2014/Google Glass/result",size,frate,".txt");
			sprintf(bufferCounter,"%s%d_%d%s","C:/Users/cvpia/Dropbox/summer 2014/time/time.",size,frate,".txt");
			//fl.open(buffer,'w');
			flCounter.open(bufferCounter,'w');


					// Some initial parameters that can be overriden from command line	
			string file, dDir, outpose, outvideo, outfeatures;
	
			// By default try webcam
			int device = 0;
			file="D:/EECE2201/Spring 2014/Google Glass/shajon1.mp4";
			// cx and cy aren't always half dimx or half dimy, so need to be able to override it (start with unit vals and init them if none specified)

			bool useCLMTracker = true;

			CLMWrapper::CLMParameters clmParams;
			clmParams.wSizeCurrent = clmParams.wSizeVLarge;

			PoseDetectorHaar::PoseDetectorHaarParameters haarParams;

			haarParams.ClassifierLocation = "..\\lib\\3rdParty\\OpenCV\\classifiers\\haarcascade_frontalface_alt.xml";
    
			bool readDepth = false;
			if(!dDir.empty())
			{
				readDepth = true;
			}

			// Do some grabbing	
	
			// Do some grabbing
			VideoCapture vCap;
			if( file.size() > 0 )
			{
				INFO_STREAM( "Attempting to read from file: " << file );
				vCap = VideoCapture( file );
			}
			else
			{
				INFO_STREAM( "Attempting to capture from device: " << device );
				vCap = VideoCapture( device );
			}

			if( !vCap.isOpened() ) FATAL_STREAM( "Failed to open video source" );
	
			Mat img;
			vCap >> img;

	

			FaceTrackerController *tracker = new FaceTrackerController(0,argv, true);//FT Controller has been initialize.
			tracker->initializeValues();
	
			// faces in a row detected

			tracker->setTrackingInitialised(false);
					//trackingInitialised = false;
			tracker->setFacesInRow(0);
			tracker->setSuccess(false);
			tracker->setFrameProc(0);
			tracker->setInitTime(0);
			// For measuring the timings
			int64 t1,t0 = cv::getTickCount();
			double fps = 10;

			Mat disp;
			int initCounter=1;

			CascadeClassifier classifier(haarParams.ClassifierLocation);

			string out="";

			while(!img.empty())
			{		
		
				cvtColor(img,disp,CV_BGR2GRAY);
				t0 = cv::getTickCount();
				tracker->GenerateFeedback(disp,clmParams,classifier,haarParams,"8000");
				t1 = cv::getTickCount();

				ostringstream  timeval;
				timeval<<double(t1-t0)/cv::getTickFrequency();
				cout<<"\n"<<timeval.str();
				out=out.append(timeval.str()).append("\n");
			
				initCounter++;
				if(tracker->getFrameProc() % 10 == 0)
				{      

					t1 = cv::getTickCount();
					fps = 10.0 / (double(t1-t0)/cv::getTickFrequency()); 
					t0 = t1;

					flCounter<<out;
					out="";
				}

				char fpsC[255];
				_itoa((int)fps, fpsC, 10);
				string fpsSt("\nFPS:");
				fpsSt += fpsC;
				cv::putText(img, fpsSt, cv::Point(10,20), CV_FONT_HERSHEY_SIMPLEX, 0.5, CV_RGB(255,0,0));
		
				tracker->setFrameProc(tracker->getFrameProc()+1);
		 			
				//imshow("colour", img);
				vCap >> img;
				//resize(img,img ,Size(400,225));
		
				/*if(!outvideo.empty())
				{		
					writerFace << disp;
				}*/
		
				// detect key presses
				char c = cv::waitKey(1);

				// key detections

				// restart the tracker
				if(c == 'r')
				{
					tracker->setTrackingInitialised(false);
					//trackingInitialised = false;
					tracker->setFacesInRow(0);
					//facesInRow = 0;
				}
				// quit the application
				else if(c=='q')
				{
					flCounter<<out;
					out="";
					flCounter.close();
					return(0);
				}
				

			}
			flCounter.close();
		}
		
	}



	

	//posesFile.close();

	return 0;
}

