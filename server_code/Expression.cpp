// Expression.cpp : Defines the entry point for the console application.
//

// clientServerTest.cpp : Defines the entry point for the console application.
/*
The main function to handle all functionality, open connection to take frame, passes frame to FT.
*/

#include "server.h"
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <math.h>
#include "CLMTracker.h"
#include "PoseDetectorHaar.h"
#include "utils.h"
#include "base64.h"



int counter;

using namespace cv;
using namespace std;
#include <string>  
#include <sstream>
#include <strstream>

using namespace std;
//JPEG Decompression

bool processDataCallback(string data, cv::Mat &frame){
	vector<unsigned char> tempBuffer = base64_decode(data);
	bool error=false;
	try{
		frame = cv::imdecode(tempBuffer,0);//built in function openCV
	}catch(cv::Exception e){
		error = true;
	}
	return error;
}

cv::Mat rotate_frame(cv::Mat &image, double angle)//Rotate the image with provided sensor angale(first 4 byte of frame)
{
	cv:: Point2f src_center(image.cols/2.0F, image.rows/2.0F);
	   
	cv::Mat rot_matrix = getRotationMatrix2D(src_center, angle, 1.0);

	cv::Mat rotated_img(cv::Size(image.size().height, image.size().width), image.type());

	warpAffine(image, rotated_img, rot_matrix, image.size());
	//imwrite("original.jpg", image);
	//imwrite("rotated.jpg", rotated_img);
	return rotated_img;
}


DWORD WINAPI startClient(LPVOID  port){

	int ports=*((int*)port);
	ostringstream  portStringValue;
	portStringValue<<ports;

		// Some initial parameters that can be overriden from command line	
	string file, dDir, outpose, outvideo, outfeatures;
	
	// By default try webcam
	int device = 0;

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
	char *name="";

	FaceTrackerController *tracker = new FaceTrackerController(0,&name, true);//FT Controller has been initialize.
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
	std::string response="";
	while(true){// waiting for new client connection

		// ############## Establishing Connection #############
		cout<<"Waiting for new Connection on port: "<<ports<<"\n";
		

		Server  serverObj=*(new Server(ports));// Constructor of server Object
		serverObj.initialize();
		if(serverObj.waitForConnection()){// if conn established
			cout<<"Connected"<<endl;
			
				
		}
		else{
			cout<<"Could Not establish a Connection";
			return -1;
		}
		// Client Connected. Send something
		serverObj.sendAny("FaceTracker Connected!!!\n");//first ack to client
		tracker->initializeValues(5);
		
		// ####################################################

		// Start a receive-acknowledge loop
		string recvString;cv::Mat inpframe;
		while(!serverObj.isClosingRequested()){// closing request detectected from client
			if(!serverObj.receiveAnyLine(recvString))//Socket  Stream Reading
				break;
			if(!recvString.empty()){
				//cout<<recvString.size()<<endl;
				counter++;
				//cout<< counter <<"\n";
			
				processDataCallback(recvString,inpframe);// convert byte string to real frame
				

				//#############new CLM CODE#################
				response=tracker->GenerateFeedback(inpframe,clmParams,classifier,haarParams,portStringValue.str());
				if(tracker->getFrameProc() % 10 == 0)
				{      
					t1 = cv::getTickCount();
					fps = 10.0 / (double(t1-t0)/cv::getTickFrequency()); 
					t0 = t1;
				}

				char fpsC[255];
				_itoa((int)fps, fpsC, 10);
				string fpsSt("\nFPS:");
				fpsSt += fpsC;
				cv::putText(inpframe, fpsSt, cv::Point(10,20), CV_FONT_HERSHEY_SIMPLEX, 0.5, CV_RGB(255,0,0));
		
				tracker->setFrameProc(tracker->getFrameProc()+1);
		 			
				
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
					return(0);
				}
				//END


				//GaussianBlur(frame,frame,Size(3,3),0);
				//cout<<"\n"<<frame.rows;
				//cout<<": "<<frame.cols<<"\n";
				//imshow("MyVideo", frame); //show the frame in "MyVideo" window
				//string response = tracker->trackFace(frame, serverObj.getSensorValue());/// passes frame and sensor value to Tracker. Output: event /no face
				// Sending Acknowledgement
				string toSend = "Got Data:"+ response;
				serverObj.sendAny(toSend.c_str());// sends feedback to client
			}else
				break;
		}
		serverObj.closeServer();// closting connection when client is disconnected
		cv::destroyAllWindows();// deallocate the memory
	}
	
	
			
	
    return 0;

}

void main()
{

	int Data_Of_Thread_1 = 8000;            // Data of Thread 1
    int Data_Of_Thread_2 = 9000;            // Data of Thread 2
	
	HANDLE Handle_Of_Thread_1 = 0;       // variable to hold handle of Thread 1
	HANDLE Handle_Of_Thread_2 = 0;       // variable to hold handle of Thread 1 
	
 	HANDLE Array_Of_Thread_Handles[3];   // Aray to store thread handles 

 	// Create thread 1.
    Handle_Of_Thread_1 = CreateThread( NULL, 0, startClient, &Data_Of_Thread_1, 0, NULL);  
    if ( Handle_Of_Thread_1 == NULL)  ExitProcess(Data_Of_Thread_1);
    
	// Create thread 2.
	Handle_Of_Thread_2 = CreateThread( NULL, 0, startClient, &Data_Of_Thread_2, 0, NULL);  
    if ( Handle_Of_Thread_2 == NULL)  ExitProcess(Data_Of_Thread_2);
    
	// Create thread 3.
	


	// Store Thread handles in Array of Thread Handles as per the requirement of WaitForMultipleObjects() 
	Array_Of_Thread_Handles[0] = Handle_Of_Thread_1;
	Array_Of_Thread_Handles[1] = Handle_Of_Thread_2;
	
    
	// Wait until all threads have terminated.
    WaitForMultipleObjects( 2, Array_Of_Thread_Handles, TRUE, INFINITE);

	printf("Since All threads executed lets close their handles \n");

	// Close all thread handles upon completion.
    CloseHandle(Handle_Of_Thread_1);
    CloseHandle(Handle_Of_Thread_2);
	getchar();
	
}
