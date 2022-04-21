#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <stdio.h>
#include <vector> //hough
#include <algorithm> //hough

using namespace cv;
using namespace std;

int R = 1;
int snr = 5200;

void calcPSF(Mat& frameOutImg, Size filterSize, int R);
void fftshift(const Mat& frameInImg, Mat& frameOutImg);
void filter2DFreq(const Mat& frameInImg, Mat& frameOutImg, const Mat& H);
void calcWnrFilter(const Mat& input_h_PSF, Mat& output_G, double nsr);

//hide the local functions in an anon namespace
namespace {
    void help(char** av) {
        cout << "q,Q,esc -- quit" << endl
             << "\tTo capture from a camera pass the device number. To find the device number, try ls /dev/video*" << endl
             << "\texample: " << av[0] << " 0" << endl
             << "\tYou may also pass a video file instead of a device number" << endl
             << "\texample: " << av[0] << " video.avi" << endl;
    }

    int process(VideoCapture& capture) {
        Mat dst,cdst;   //hough
        vector<Vec4i> lines;    //hough

        int n = 0;
        char filename[200];
        string window_name = "video test || q or esc -> video over";
        cout << "q or esc to quit" << endl;
        namedWindow(window_name, WINDOW_KEEPRATIO); //resizable window;
        Mat frame, frameOut;
        Mat gray;

        for (;;) {  //until frame empty
            capture >> frame;
            if (frame.empty())
                break;

            cvtColor(frame,gray,COLOR_BGR2GRAY);
            Canny(gray,dst,100,200);    //hough
            cvtColor(dst,cdst,COLOR_GRAY2BGR);  //hough
            HoughLinesP(dst, lines, 1, CV_PI / 180, 50, 100, 20);//hough
                for (size_t i = 0; i < lines.size(); i++) {//hough
                    Vec4i l = lines[i];//hough
                    line(cdst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255), 3, 4);//hough
            }
            imshow(window_name, cdst);//hough

            Rect roi = Rect(0,0,gray.cols&-2, gray.rows&-2);

            Mat Hw, h;
            calcPSF(h,roi.size(),R);
            calcWnrFilter(h,Hw,1.0/double(snr));

            filter2DFreq(gray(roi),frameOut,Hw);

            frameOut.convertTo(frameOut,CV_8U);
             normalize(frameOut,frameOut,0,255,NORM_MINMAX);


            char key = (char)waitKey(30); //delay N millis, usually long enough to display and capture input

            switch (key) {
            case 'q':
            case 'Q':
            case 27: //escape key
                return 0;
            default:
                break;
            }
        }
        return 0;
    }
}


/************************main함수***************************/

int main(int ac, char** av) {

    
    cv::CommandLineParser parser(ac, av, "{help h||}{@input||}");
    if (parser.has("help"))
    {
        help(av);
        return 0;
    }
    std::string arg = parser.get<std::string>("@input");
    if (arg.empty()) {
        help(av);
        return 1;
    }
    VideoCapture capture(arg); //try to open string, this will attempt to open it as a video file or image sequence
    if (!capture.isOpened()) //if this fails, try to open as a video camera, through the use of an integer param
        capture.open(atoi(arg.c_str()));
    if (!capture.isOpened()) {
        cerr << "Failed to open the video device, video file or image sequence!\n" << endl;
        help(av);
        return 1;
    }
    return process(capture);
}
/******************************************************************/


/***************************deblur 함수*****************************/

void calcPSF(Mat& frameOutImg, Size filterSize, int R)
{
    Mat h(filterSize,CV_32F,Scalar(0));
    Point point(filterSize.width/2, filterSize.height/2);
    circle(h,point,1,255,-1,8);
    Scalar summa = sum(h);
    frameOutImg = h/summa[0];
}

void fftshift(const Mat& frameInImg, Mat& frameOutImg)
{
    frameOutImg = frameInImg.clone();
    int cx = frameOutImg.cols/2;
    int cy = frameOutImg.rows/2;
    Mat q0(frameOutImg,Rect(0,0,cx,cy));
    Mat q1(frameOutImg,Rect(cx,0,cx,cy));
    Mat q2(frameOutImg,Rect(0,cy,cx,cy));
    Mat q3(frameOutImg,Rect(cx,cy,cx,cy));
    Mat tmp;
    q0.copyTo(tmp);
    q3.copyTo(q0);
    tmp.copyTo(q3);
    q1.copyTo(tmp);
    q2.copyTo(q1);
    tmp.copyTo(q2);
}

void filter2DFreq(const Mat& frameInImg, Mat& frameOutImg, const Mat& H)
{
    Mat planes[2] = { Mat_<float>(frameInImg.clone()), Mat::zeros(frameInImg.size(),CV_32F) };
    Mat complexI;
    merge(planes, 2, complexI);
    dft(complexI, complexI, DFT_SCALE);

    Mat planesH[2] = { Mat_<float>(H.clone()), Mat::zeros(H.size(), CV_32F) };
    Mat complexH;
    merge(planesH, 2, complexH);
    Mat complexIH;
    mulSpectrums(complexI, complexH, complexIH, 0);

    idft(complexIH, complexIH);
    split(complexIH, planes);
    frameOutImg = planes[0];
}

void calcWnrFilter(const Mat& input_h_PSF, Mat& output_G, double nsr)
{
    Mat h_PSF_shifted;
    fftshift(input_h_PSF, h_PSF_shifted);
    Mat planes[2] = { Mat_<float>(h_PSF_shifted.clone()), Mat::zeros(h_PSF_shifted.size(), CV_32F) };
    Mat complexI;
    merge(planes, 2, complexI);
    dft(complexI, complexI);
    split(complexI, planes);
    Mat denom;
    pow(abs(planes[0]),2,denom);
    denom += nsr;
    divide(planes[0],denom,output_G);
}
/******************************************************************/