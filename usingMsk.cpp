//drive.mp4
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <stdio.h>
#include <vector>
#include <algorithm>

using namespace cv;
using namespace std;

Mat frame, frameOut;
Mat gray, Gblur, edges;
Mat msk, roi_msk, roi_msk1, roi_msk2, roi1, roi2, roi;
Mat hsv;
Mat dst, cdst;   //hough
vector<Vec4i> lines;    //hough
int kernel_size=5;
Mat yellow_msk, white_msk;

int main(int ac, char** av)
{
    /****************영상 불러오기*************/
    cv::CommandLineParser parser(ac, av, "{help h||}{@input||}");
    std::string arg = parser.get<std::string>("@input");
    VideoCapture capture(arg);  //영상 불러오기
    int n = 0;
    char filename[200];
    string window_name = "video test || q or esc -> video over";
    namedWindow(window_name, WINDOW_KEEPRATIO);
        for (;;) {  //until frame empty
            capture >> frame;
            if (frame.empty())
                break;
    /****************관심영역 설정****************/
        roi_msk1 = Mat::zeros(frame.size(), CV_8UC3);
        Point points1[1][4];
        points1[0][0] = Point(frame.cols/3+80,frame.rows/4+10);
        points1[0][1] = Point(80,frame.rows-300);
        points1[0][2] = Point(300,frame.rows-300);
        points1[0][3] = Point(frame.cols/3+160,frame.rows/4+10);
        const Point* ppt1[1] = {points1[0]};
        int npt1[] = {4};
        fillPoly(roi_msk1, ppt1, npt1, 1, Scalar(255,255,255), 8);   //mask1 제작

        roi_msk2 = Mat::zeros(frame.size(), CV_8UC3);
        Point points2[1][4];
        points2[0][0] = Point(frame.cols/3*2-250,frame.rows/4+10);
        points2[0][1] = Point(frame.cols-360,frame.rows-300);
        points2[0][2] = Point(frame.cols-200,frame.rows-300);
        points2[0][3] = Point(frame.cols/3*2-200,frame.rows/4+10);
        const Point* ppt2[1] = {points2[0]};
        int npt2[] = {4};
        fillPoly(roi_msk2, ppt2, npt2, 1, Scalar(255,255,255), 8);   //mask2 제작
     
        bitwise_or(roi_msk1,roi_msk2,roi_msk);
        bitwise_and(frame,roi_msk,roi);
    /********************HSV*********************/ 
        cvtColor(roi,hsv,COLOR_BGR2HSV);
        inRange(hsv,Scalar(22,50,200),Scalar(180,255,255),yellow_msk);
        inRange(hsv,Scalar(0,0,200),Scalar(180,10,255),white_msk);
        addWeighted(yellow_msk,1.0,white_msk,1.0,0.0,msk);
        bitwise_and(roi,roi,hsv,msk);
        Mat kernel = (Mat_<uchar>(3,3) << 1,1,1,
                                          1,1,1,
                                          1,1,1);
        morphologyEx(msk,msk,MORPH_OPEN,kernel);
    /*****************블러, 에지******************/
        GaussianBlur(msk,Gblur,Size(5,5),0,0);
        Canny(Gblur,edges,300,800,kernel_size);//Canny(Gblur,edges,100,600,kernel_size);
    /*******************Lines********************/
        cvtColor(edges,cdst,COLOR_GRAY2BGR);
        HoughLinesP(edges, lines, 1, CV_PI / 180, 50, 100, 20);//hough
                for (size_t i = 0; i < lines.size(); i++) {//hough
                    Vec4i l = lines[i];//hough
                    line(frame, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255), 3);//hough
                }
    /*****************영상보기********************/
        imshow(window_name, frame);
    /****************영상 나가기******************/
        char key = (char)waitKey(30);
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