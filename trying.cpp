#include <opencv2/core/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>


#include <iostream>
#include <string>
using namespace std;
using namespace cv;

string video = "slow2.m4v";
string window = "Origin Image";
string window2 = "Substracted Image";
string control = "Control Panel";

int ratio = 50;
int** curve;

Mat image;
Point vertexUP(-1,-1);
Point vertexDown(-1,-1);
// color (blue,green,red)
Scalar color(0,255,0);
Scalar dart1(255,153,51);
Scalar dart2(153,255,51);
Scalar dart3(255,0,255);
Rect subImgArea(0,0,1,1);


int objectlowerbound = 5;
int objectupperbound = 60;

int curveClear = 0;
int pause = 0;
int colorchange = 0;
int dart = 1;


void initial();
void onMouse(int event,int x,int y,int flags,void* param);
void showContours(Mat frame, vector< vector<Point> > contours); // Show contours in main window
void clearCurveFunc(int,void*);
void changeColor(int,void*);


int main(int argc, char* argv[])
{
    initial();
    VideoCapture cap(video);
    // VideoCapture cap("input3.mp4");


    Mat fgMOG2MaskImg, fgMOG2Img, bgMOG2Img, contoursImg;
    Ptr<BackgroundSubtractor> pMOG2;
    int history = 1000;
    double varThreshold = 200;
    bool detectShadows = true;
    pMOG2 = createBackgroundSubtractorMOG2(history, varThreshold, detectShadows);
    bool learningRate = true;
    //The value between 0 and 1 that indicates how fast the background model is learnt.
    //Negative parameter value makes the algorithm to use some automatically chosen learning rate.
    //0 means that the background model is not updated at all,
    //1 means that the background model is completely reinitialized from the last frame.

    while(1)
    {
        vector< vector<Point> > contours;
        vector<Vec4i> hierarchy;

        if(pause == 0)
        {
            cap >> image;
            if(image.empty())
            {
                break;
            }

            resize(image, image, Size(16*ratio,9*ratio));

            if(vertexUP.x < vertexDown.x && vertexUP.y < vertexDown.y)
            {       
                subImgArea.x = vertexUP.x;
                subImgArea.y = vertexUP.y;
                subImgArea.width = vertexDown.x-vertexUP.x;
                subImgArea.height = vertexDown.y-vertexUP.y;


                // Mat cloneImage;
                // image.copyTo(cloneImage);
                // Mat subImg(cloneImage,subImgArea);
                Mat subImg(image,subImgArea);


                //update the model
                pMOG2->apply(subImg, fgMOG2MaskImg, learningRate ? -1 : 0);
                
                fgMOG2Img = Scalar::all(0);
                subImg.copyTo(fgMOG2Img, fgMOG2MaskImg);
                
                pMOG2->getBackgroundImage(bgMOG2Img);

                // Find the contours in the image
                contoursImg = fgMOG2MaskImg.clone();
                

                if(curveClear == 0)
                {
                    findContours(contoursImg, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
                    showContours(subImg,contours); 
                    for(int i=0;i<ratio*16;i++)
                    {
                        for(int j=0;j<ratio*9;j++)
                        {
                            if(curve[i][j] == 1)
                            {
                                line(subImg,Point(i,j),Point(i,j),dart1,4);
                            }
                            if(curve[i][j] == 2)
                            {
                                line(subImg,Point(i,j),Point(i,j),dart2,4);
                            }
                            if(curve[i][j] == 3)
                            {
                                line(subImg,Point(i,j),Point(i,j),dart3,4);
                            }
                            
                        }
                    }
                }
                
                // imshow(window2,subImg);
            }

            rectangle(image, vertexUP, vertexDown, color);
            imshow(window,image);
        }
        int key = waitKey(1);
        if(key == 32)
        {
            cout << "Key \"space\" pressed." << endl;
        } 
        else if(key == 27)
        {
            break;
        }
    }
}
void initial()
{
    curve = new int*[ratio*16];
    for(int i=0;i<ratio*16;i++)
    {
        curve[i] = new int[ratio*9];
    }
    for(int i=0;i<ratio*16;i++)
    {
        for(int j=0;j<ratio*9;j++)
        {
            curve[i][j] = 0;
        }
    }
    namedWindow(window,1);
    createTrackbar("Object Lower Bound", window, &objectlowerbound, 50);
    createTrackbar("Object Upper Bound", window, &objectupperbound, 500);
    setMouseCallback(window,onMouse,NULL);
    moveWindow(window,0,0);

    // namedWindow(window2,1);
    // moveWindow(window2,700,200);

    namedWindow(control,1);
    createTrackbar("Pause", control, &pause, 1);
    createTrackbar("Stop tracking", control, &curveClear, 1, clearCurveFunc);
    createTrackbar("Change color", control, &colorchange, 1, changeColor);
    moveWindow(control,ratio*16+80,100);
}
void onMouse(int event,int x,int y,int flag,void* param)
{
    // cout << "x: " << x << " y: " << y << " event: " << event << " flag: " << flag << endl;
    if(event==1)
    {
        vertexUP.x = x;
        vertexUP.y = y;
    }
    else if(flag==1)
    {
        vertexDown.x = x;
        vertexDown.y = y;
    }   
}
void showContours(Mat frame, vector< vector<Point> > contours)
{
    bool found = false; // For finding the biggest one in the video only
    int i;
    for(i=0;i<contours.size();i++)
    {
        double objectArea = contourArea(contours[i], false); // Find the area of contour
        if(objectArea > objectlowerbound && objectArea < objectupperbound)
        {
            Rect bounding_rect = boundingRect(contours[i]);
            int x = bounding_rect.x + bounding_rect.width/2;
            int y = bounding_rect.y + bounding_rect.height/2;
            curve[x][y] = dart;
            rectangle(frame, bounding_rect, Scalar(255,0,0), 1, 8, 0);   
        }   
    }

}
void clearCurveFunc(int,void*)
{
    for(int i=0;i<ratio*16;i++)
    {
        for(int j=0;j<ratio*9;j++)
        {
            curve[i][j] = 0;
        }
    }
}

void changeColor(int,void*)
{
    dart++;
}