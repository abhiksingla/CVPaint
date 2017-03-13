/*
CVpaint(virtual paint)

Abhik Singla
2nd year, ECE, NIT KURUKSHETRA
Shreya Singh
2nd year, ECE, NIT KURUKSHETRA

Discription-->
CVpaint is an application which allows simple drawing using camera input. 
It provides user an experience of virtual paint in real time with the use of handheld colored object and camera input. 
User can draw or write virtually by movement of object in void without actually touching the screen and can save image in any preferred image format. 
The project includes the following control features with the camera:  
Changing the brush color  
Changing the brush thickness  
Clearing the screen  
Saving the image in preferred format  
Closing the application.
*/

#include "stdafx.h"
#include <iostream>
#include "opencv2\highgui\highgui.hpp"
#include "opencv2\imgproc\imgproc.hpp"
#include "opencv2\core\core.hpp"
#include "opencv2\flann\miniflann.hpp" 
#include "opencv2\imgproc\imgproc.hpp" 
#include "opencv2\photo\photo.hpp" 
#include "opencv2\video\video.hpp" 
#include "opencv2\features2d\features2d.hpp" 
#include "opencv2\objdetect\objdetect.hpp"
#include "opencv2\calib3d\calib3d.hpp" 
#include "opencv2\ml\ml.hpp"  
#include "opencv2\contrib\contrib.hpp" 
#include "opencv2\core\core_c.h" 
#include "opencv2\highgui\highgui_c.h" 
#include <cmath>

using namespace cv;//all opencv classes are in cv namespace
using namespace std;//for c++ functions

//defining colors
#define red  Scalar(0,0,255)
#define green Scalar(0,255,0)
#define blue Scalar(255,0,0)
#define white Scalar(255,255,255)
#define black Scalar(0,0,0)

int main()
{
	//defaults setting
	int lineThickness = 2;
	Scalar lineColor = blue;
	double area_limit = 700;
	Point cvpoint =Point(120,43);//location of the text

////////////////////////////////////////////////////////////////////////////////////////////////////////
	//creating camera object and capturing video
	VideoCapture cap(0);//captures video from device identity 0, webcam in this case

	if(!cap.isOpened())//functions true if videocapture intialization is success
	{
		cout<<"Cannot open the Webcam"<<endl;
		system("pause");
		return -1;//exit program if video not loaded
	}
////////////////////////////////////////////////////////////////////////////////////////////////////////
	//creating control box to control threshold settings
	namedWindow("Control Box",CV_WINDOW_AUTOSIZE);//create window named control box
	namedWindow("Threshold", CV_WINDOW_AUTOSIZE);
	namedWindow("Drawing", CV_WINDOW_AUTOSIZE);
	namedWindow("Video", CV_WINDOW_AUTOSIZE);

	int lowH=0;
	int highH=179;

	int lowS=0;
	int highS=255;

	int lowV=0;
	int highV=255;

	//creating trackbars in control box window to control HSV
	createTrackbar("LowH","Control Box", &lowH,179);//hue level(0-179)
	createTrackbar("HighH","Control Box", &highH,179);

	createTrackbar("LowS","Control Box", &lowS,255);//saturation level(0-255)
	createTrackbar("HighS","Control Box", &highS,255);

	createTrackbar("LowV","Control Box", &lowV,255);//value level(0-255)
	createTrackbar("HighV","Control Box", &highV,255);
	
//////////////////////////////////////////////////////////////////////////////////////////////////////////
	Mat imgColorPanel = imread("C:/Users/DELL/Desktop/cvPaint.panel",CV_LOAD_IMAGE_COLOR);//loading panel image in png format
	if(imgColorPanel.empty())
	{
		cout<<"cvpaint.panel is not loaded!!"<<endl;
		system("pause");
		return -1;
	}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
	int Width = cap.get(CV_CAP_PROP_FRAME_WIDTH); //get the width of frames of the video
    int Height = cap.get(CV_CAP_PROP_FRAME_HEIGHT); //get the height of frames of the video
	Mat imgScribble(Height,Width,CV_8UC3);
	Mat imgDrawing(Height,Width,CV_8UC3,white);
//////////////////////////////////////////////////////////////////////////////////////////////////////////
	//timers & buffers initialization
	int confirm_close =15;//counters for clear and exit confirmation
	int confirm_clear =20;
	char buffer[100];//buffer for text display(putText)
	int image_num = 0;//to keep track of image numbers for saving
	
/////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	int lastX;//coordinates defined
	int lastY;
	int posX=0;
	int posY=0;

	Mat frame;
	Mat imgHSV(Height,Width,CV_8UC3);
	Mat imgThreshold(Height,Width,CV_8UC3);


	while(true)//infinite loop
	{
		bool bsuccess = cap.read(frame); //read new frame from video and pass to mat datatype image file, true on success
		if(!bsuccess)//if not success break loop
		{
			cout<<"Cannot read a frame from video!!"<<endl;
			system("pause");
			break;
		}

		flip(frame,frame,1);//flip the frame to overcome mirroring problem
		
		//median filter to decrease background noise
	    //medianBlur(frame,frame,3);

		cvtColor(frame,imgHSV,COLOR_BGR2HSV);//Convert original frame from BGR to HSV

		inRange(imgHSV,Scalar(lowH,lowS,lowV),Scalar(highH,highS,highV),imgThreshold);//threshold the HSV image in range passed in second and third argument

		//Morphological opening(remove small objects from background)
		erode(imgThreshold,imgThreshold,getStructuringElement(MORPH_ELLIPSE,Size(5,5)));
		dilate(imgThreshold,imgThreshold,getStructuringElement(MORPH_ELLIPSE,Size(5,5)));

		//Morphological closing(fill small holes in foreground)
		//dilate(imgThreshold,imgThreshold,getStructuringElement(MORPH_ELLIPSE,Size(5,5)));
		//erode(imgThreshold,imgThreshold,getStructuringElement(MORPH_ELLIPSE,Size(5,5)));

		//calculate moments of the threshold image
		Moments oMoments = moments(imgThreshold);//all spatial moments of threshold image is saved in object moment

		double dM01 = oMoments.m01;//1st order spatial moment
		double dM10 = oMoments.m10;//1st order spatial moment
		double dArea = oMoments.m00;//0 order moment, equal to white area in pixels 
		
		lastX = posX;
		lastY = posY;

		//if area<area_limit, considered as noise
		if(dArea>area_limit)
		{
			//calculating position of object
			 posX = dM10/dArea;
			 posY = dM01/dArea;
		}

		if(posX<110 && posY>380)//clear tab
		{
			lineColor = white; // white color works as eraser
			putText(frame,"Eraser selected..",cvpoint, CV_FONT_HERSHEY_SIMPLEX,1.5,white,2);//diplays text on video
			sprintf_s(buffer,"Clearing the screen in %d",confirm_clear);//countdown for clearing the screen and coz it do formatting
			putText(frame,buffer,Point(85,85), CV_FONT_HERSHEY_SIMPLEX,1,white,2);
			confirm_clear--;
			if(confirm_clear<0)
			{
				//clearing the full screen
				confirm_clear=20;
				imgScribble.setTo(black);
				imgDrawing.setTo(white);
				putText(frame,"Cleared the screen",Point(110,125), CV_FONT_HERSHEY_SIMPLEX,1.5,white,2);
			}
		}
		else if(posX>540 && posY>350)//blue tab
		{
			lineColor = blue;
			putText(frame,"Blue color selected..",cvpoint, CV_FONT_HERSHEY_SIMPLEX,1.5,blue,2);
		}
		else if(posX>540 && posY>165 && posY<300)//green tab
		{
			lineColor = green;
			putText(frame,"Green color selected..",cvpoint, CV_FONT_HERSHEY_SIMPLEX,1.5,green,2);
		}
		else if(posX  > 540 && posY < 130)//red tab
		{
			lineColor = red;
			putText(frame,"Red color selected..",cvpoint, CV_FONT_HERSHEY_SIMPLEX,1.5,red,2);
		}
		else if(posX > 0 && posX  < 110 && posY > 0 && posY < 110)//exit tab
		{
			sprintf_s(buffer,"Exiting in.. %d",confirm_close);
			putText(frame,buffer,cvpoint, CV_FONT_HERSHEY_SIMPLEX,1.5,red,2);
			confirm_close--;
			if(confirm_close < 0) 
			{  
				//save the frame into an image
				sprintf_s(buffer, "d0%d.jpg",image_num++);
				vector<int> compression_params; //vector that stores the compression parameters of the image
				compression_params.push_back(CV_IMWRITE_JPEG_QUALITY); //specify the compression technique
				compression_params.push_back(98); //specify the compression quality
				bool bSuccess = imwrite(buffer, imgDrawing, compression_params); //write the image to file
				if ( !bSuccess )
				{
					cout << "ERROR : Failed to save the image" << endl;
					system("pause");
					return -1;//wait for a key press
				}
				return 0;
			}
		}
	    else if(posX < 110 && posY > 130 && posY < 350)//brush thickness tab
		{
			lineThickness = 6 - (posY/60 - 1);//changing brush thickness from 0-5 based on posY
		}

		sprintf_s(buffer,"%d",lineThickness);
		putText(frame,buffer,Point(32,260), CV_FONT_HERSHEY_TRIPLEX,2.5, lineColor,4);

		double diffX = lastX-posY;
		double diffY = lastY-posY;
		double magnitude = sqrt(pow(diffX,2)+pow(diffY,2));

		//we will draw a line only if its in a valid position
		if(magnitude>0 && magnitude<300 && posX>120 && posX<530)
		{
			line(imgDrawing,Point(posX,posY),Point(lastX,lastY),lineColor,lineThickness);//draws a line from prev point to present point
		}
		
		//combining everything in frame
		bitwise_and(frame,imgDrawing,frame);
		bitwise_and(imgColorPanel,frame,frame);//combining the image panel and the frame

		//displaying results
		imshow("Video",frame);
		imshow("Drawing",imgDrawing);
		imshow("Threshold",imgThreshold);

		if(waitKey(10)==27)//wait for esc key press for 10ms, if key pressed loop breaks
		{
			//cout<<"Esc key is pressed by user.."<<endl;
			break;
		}
	}

	return 0;
}