#include <iostream>
#include <raspicam/raspicam_cv.h>
#include <opencv2/imgproc.hpp>
 
using namespace std;
using namespace cv;
 
int main ( int argc,char **argv ) {
 
    raspicam::RaspiCam_Cv Camera;
    Mat image;
 
    Camera.set( CV_CAP_PROP_FORMAT, CV_8UC3);
    Camera.set( CV_CAP_PROP_FRAME_WIDTH, 640 );
    Camera.set( CV_CAP_PROP_FRAME_HEIGHT, 480 );
 
 
    if (!Camera.open()) {cerr<<"Error opening the camera"<<endl;return -1;}
 
    while(1){


        Camera.grab();
        Camera.retrieve ( image);
 
        //cvtColor(image, image, cv::COLOR_BGR2RGB);
 
        imshow( "picamera test", image );
        if ( waitKey(20) == 27 ) break; //ESC키 누르면 종료
 
    }
 
    Camera.release();
}
