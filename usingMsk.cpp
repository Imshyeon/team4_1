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
Mat msk, roi;
Mat dst,cdst;   //hough
vector<Vec4i> lines;    //hough
int kernel_size=5;

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
    /*****************블러, 에지******************/
        cvtColor(frame,gray,COLOR_BGR2GRAY);    //영상 흑백처리
        GaussianBlur(gray,Gblur,Size(5,5),0,0);
        Canny(Gblur,edges,200,600,kernel_size);
    /****************관심영역 설정****************/
        msk = Mat::zeros(frame.size(), CV_8UC1);
        Point points[1][4];
        points[0][0] = Point(edges.cols/3,edges.rows/4*2.5);
        points[0][1] = Point(100,edges.rows-50);
        points[0][2] = Point(edges.cols-100,edges.rows-50);
        points[0][3] = Point(edges.cols/3*2,edges.rows/4*2.5);
        const Point* ppt[1] = {points[0]};
        int npt[] = {4};
        fillPoly(msk, ppt, npt, 1, Scalar(255,255,255), 8);   //mask 제작
        roi=edges.clone();
        bitwise_and(edges,msk,roi);
    /*******************Lines********************/
        //line(roi,)
        cvtColor(roi,cdst,COLOR_GRAY2BGR);
        HoughLinesP(roi, lines, 1, CV_PI / 180, 50, 100, 20);//hough
                for (size_t i = 0; i < lines.size(); i++) {//hough
                    Vec4i l = lines[i];//hough
                    line(cdst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255), 3, 4);//hough
                }
    /*****************영상보기********************/
        imshow(window_name,cdst);
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