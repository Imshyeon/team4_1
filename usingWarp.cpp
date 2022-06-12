#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/opencv_modules.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include <math.h>
#include <iomanip>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>


using namespace cv;
using namespace std;

void draw_locations(Mat& img,vector<Rect>& locations, const Scalar& color, string text);
#define CASCADE_FILE_NAME "/home/suhyeon/capstone_design/suhyeon_code/cars.xml"
Mat frame, frameOut;
Mat msk, roi_msk, roi_msk1, roi_msk2, roi1, roi2, roi;
Mat hsv, yellow_msk, white_msk, msk_final;
Mat edges, cdst, Gblur, newWarp, result;
vector<Vec4i> lines;
vector<Rect> car_found;
//Mat car_tracking_img;
int CarRect, Detectlines, Gpio, DCount = 0;

void gpio()
{
    if(Detectlines == 1){ 
        if(DCount == 2){
            system("./buzzer.py &");
            system(" ./LED.py & ");
            DCount = 0;
            printf("ON\r\n");
        }
        else
        {
            DCount++;
            printf("%d\r\n", DCount);    
        }
    }
    else
    {
        printf("OFF\r\n");
        DCount = 0;
    }
}

int main(int ac, char** av)
{
    CascadeClassifier car;
    car.load(CASCADE_FILE_NAME);
    if(car.empty())
        cout << "xml file not loaded" << endl;

    /****************영상 불러오기*************/
        cv::CommandLineParser parser(ac, av, "{help h||}{@input||}");
        std::string arg = parser.get<std::string>("@input");
        VideoCapture capture(arg);  //영상 불러오기
        if(!capture.isOpened())
            capture.open(atoi(arg.c_str()));

        int n = 0;
        char filename[200];
        string window_name = "video test || q or esc -> video over";
        namedWindow(window_name, WINDOW_KEEPRATIO);
        for (;;) {  //until frame empty
            capture >> frame;
            if (frame.empty())
                break;

    /******************warp*****************/    
        Point2f inputp[4];  //워프변환 행렬에 필요한 값 설정
        inputp[0] = Point(frame.cols/2-65,frame.rows*0.6);
        inputp[1] = Point(frame.cols/2+97, frame.rows*0.6);
        inputp[2] = Point(frame.cols*0.42,frame.rows);
        inputp[3] = Point(frame.cols*0.9, frame.rows);
        Point2f* ppt0[1] = {inputp};
        Point2f outputp[4]; //워프변환 행렬에 필요한 값 설정
        outputp[0] = Point(0,0);
        outputp[1] = Point(frame.cols-350,0);
        outputp[2] = Point(400, frame.rows);
        outputp[3] = Point(frame.cols-150, frame.rows);
        Mat transform_matrix = getPerspectiveTransform(inputp, outputp);   
          //워프변환 행렬(원본->워프)
        Mat re_matrix = getPerspectiveTransform(outputp,inputp);   
         //워프변환 행렬(워프->원본)
        Mat out;
        warpPerspective(frame,out,transform_matrix,frame.size());   //워프변환
         //transform_matrix 행렬값에 의해서 frame를 frame사이즈에 맞춰 out창에 넣는다.

    /****************관심영역 설정****************/
        roi_msk1 = Mat::zeros(out.size(), CV_8UC3); //roi_msk1을 out사이즈로 검은 창을 만든다.
        Point points1[1][4];    //관심영역으로 설정할 구역 값 설정하기
        points1[0][0] = Point(out.cols*0.1,out.rows);
        points1[0][1] = Point(out.cols*0.1,out.rows*0.1);
        points1[0][2] = Point(out.cols*0.4,out.rows*0.1);
        points1[0][3] = Point(out.cols*0.4,out.rows);
        const Point* ppt1[1] = {points1[0]};
        int npt1[] = {4};
        fillPoly(roi_msk1, ppt1, npt1, 1, Scalar(255,255,255), 8);   //mask1 제작
                                                             //다각형의 색을 채워넣는 함수.
                // roi_msk1 창에 관심구역만큼 흰색으로 채워넣는다. -> 결과적으로 마스크제작

        roi_msk2 = Mat::zeros(out.size(), CV_8UC3); //roi_msk2을 out사이즈로 검은 창을 만든다.
        Point points2[1][4];    //관심영역으로 설정할 구역 값 설정하기
        points2[0][0] = Point(out.cols*0.7,out.rows);
        points2[0][1] = Point(out.cols*0.7,out.rows*0.1);
        points2[0][2] = Point(out.cols*0.9,out.rows*0.1);
        points2[0][3] = Point(out.cols*0.9,out.rows);
        const Point* ppt2[1] = {points2[0]};
        int npt2[] = {4};
        fillPoly(roi_msk2, ppt2, npt2, 1, Scalar(255,255,255), 8);   //mask2 제작
                                                             //다각형의 색을 채워넣는 함수.
                // roi_msk1 창에 관심구역만큼 흰색으로 채워넣는다. -> 결과적으로 마스크제작

        bitwise_or(roi_msk1,roi_msk2,roi_msk);//roi_msk1와 roi_msk2를 or연산을 하여 
                                          //그 결과값을 roi_msk에 넣는다.
        bitwise_and(out,roi_msk,roi);   //out과 roi_msk의 and 연산 결과를 roi에 넣는다. 
                                    //-> 관심영역으로 설정한 부분만 보이게 됨

    /****************HSV,threshold****************/
        cvtColor(roi,hsv,COLOR_BGR2HSV);    //roi를 HSV 컬러영상으로 바꿔 hsv에 저장.
        inRange(hsv,Scalar(20,80,140),Scalar(32,255,255),yellow_msk);   
//hsv에서 Scalar(20,80,140)~Scalar(32,255,255)사이의 구간에 있는 색깔 값을 yellow_msk에 저장.
        inRange(hsv,Scalar(0,0,200),Scalar(180,255,255),white_msk); 
//hsv에서 Scalar(0,0,200)~Scalar(180,255,255)사이의 구간에 있는 색깔 값을 white_msk에 저장.
        addWeighted(yellow_msk,1.0,white_msk,1.0,0.0,msk);  
        //yellow_msk*1.0 + white_msk*1.0 = msk에 저장
        //bitwise_and(roi,roi,hsv,msk); //결과 : hsv
        threshold(msk, msk_final, 160, 255, THRESH_BINARY); 
//msk에서 160이상은 흰색, 160 이하는 검은색으로 이진화 -> msk_final에 저장

    /*****************DetectL*********************/
        GaussianBlur(msk_final,Gblur,Size(5,5),0,0); 
         //가우시안블러. 5x5사이즈로 블러효과를 내어 Gblur에 저장.
        Canny(Gblur,edges,100,200); //Gblur를 케니에지 -> 에지 연산
        cvtColor(edges,cdst,COLOR_GRAY2BGR);    
         //edges를 흑백영상에서 컬러영상으로 바꾼 뒤, cdst에 저장
            HoughLinesP(edges, lines, 1, CV_PI / 180, 100, 100, 20);//hough변환
                for (size_t i = 0; i < lines.size(); i++) {//hough
                    Vec4i l = lines[i];//hough
                    Detectlines = 1;
                    line(out, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255), 15);
                 //직선 그리기. out창에 결과값을 나타내고 Scalar(0,0,255)인 빨간색으로 표시
                }

    /******************Warp,msk*******************/
        warpPerspective(out,newWarp,re_matrix,frame.size());   
    //워프영상인 out창을 re_matrix 행렬에 맞춰 frame사이즈로 newWarp에 저장.
        addWeighted(frame, 1, newWarp, 1, 0, result);   //frame*1 + newWarp*1 = result

    /******************차량검출*******************/
        car.detectMultiScale(result, car_found, 1.1, 5);
        int c, cx1, cx2, cy1, cy2;
        for(c = 0; c<car_found.size(); c++){
            rectangle(result, car_found[c].tl(),car_found[c].br(),Scalar(0, 0, 255), 3);
            cx1 = car_found[c].x;
            cy1 = car_found[c].y;
            cx2 = car_found[c].x + car_found[c].width;
            cy2 = car_found[c].y + car_found[c].height;
        }

        if(cy2 > frame.rows * 0.6){
            if((cx1 > frame.cols/2-65) || (cx2 < frame.cols/2+97))
            Detectlines = 1;
        }
        else
            Detectlines = 0;

    /***************GPIO,영상보기*****************/ 
        gpio();
        imshow(window_name, result);    //result결과값 보기

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
