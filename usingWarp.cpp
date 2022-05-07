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
Mat msk, roi_msk, roi_msk1, roi_msk2, roi1, roi2, roi;
Mat hsv, yellow_msk, white_msk, msk_final;
Mat edges, cdst;
vector<Vec4i> lines;
int kernel_size=5;

void drawHist(int histogram[])
{
    int hist_w = 512;
    int hist_h = 400;
    int bin_w = cvRound((double)hist_w/256);

    Mat histImg(hist_h,hist_w,CV_8UC3,Scalar(255,255,255));
    int max = histogram[0];
    for(int i = 1; i<256; i++){
        if(max<histogram[i])
            max = histogram[i];
    }
    for(int i = 0; i<255; i++){
        histogram[i] = floor(((double)histogram[i]/max)*histImg.rows);
    }
    for(int i=0; i<255; i++){
        line(histImg,Point(bin_w*(i),hist_h),Point(bin_w*(i),hist_h - histogram[i]),Scalar(0,0,255));
    }
    //imshow("histogram",histImg);
}
void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
    if (event == EVENT_LBUTTONDOWN)
    {
        cout << "왼쪽 마우스 버튼 클릭.. 좌표 = (" << x << ", " << y << ")" << endl;
    }
}

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
    /******************warp*****************/    
        Point2f inputp[4];
        inputp[0] = Point(frame.cols/2-30,frame.rows*0.53);
        inputp[1] = Point(frame.cols/2+60, frame.rows*0.53);
        inputp[2] = Point(frame.cols*0.3,frame.rows);
        inputp[3] = Point(frame.cols, frame.rows);
        Point2f outputp[4];
        outputp[0] = Point(0,0);
        outputp[1] = Point(frame.cols-350,0);
        outputp[2] = Point(400, frame.rows);
        outputp[3] = Point(frame.cols-150, frame.rows);
        Mat transform_matrix = getPerspectiveTransform(inputp, outputp);
        Mat re_matrix = getPerspectiveTransform(outputp,inputp);
        Mat out;
        warpPerspective(frame,out,transform_matrix,frame.size());
    /****************관심영역 설정****************/
        roi_msk1 = Mat::zeros(out.size(), CV_8UC3);
        Point points1[1][4];
        points1[0][0] = Point(out.cols*0.1,out.rows);
        points1[0][1] = Point(out.cols*0.1,out.rows*0.1);
        points1[0][2] = Point(out.cols*0.4,out.rows*0.1);
        points1[0][3] = Point(out.cols*0.4,out.rows);
        const Point* ppt1[1] = {points1[0]};
        int npt1[] = {4};
        fillPoly(roi_msk1, ppt1, npt1, 1, Scalar(255,255,255), 8);   //mask1 제작
        roi_msk2 = Mat::zeros(out.size(), CV_8UC3);
        Point points2[1][4];
        points2[0][0] = Point(out.cols*0.7,out.rows);
        points2[0][1] = Point(out.cols*0.7,out.rows*0.1);
        points2[0][2] = Point(out.cols*0.9,out.rows*0.1);
        points2[0][3] = Point(out.cols*0.9,out.rows);
        const Point* ppt2[1] = {points2[0]};
        int npt2[] = {4};
        fillPoly(roi_msk2, ppt2, npt2, 1, Scalar(255,255,255), 8);   //mask2 제작
        bitwise_or(roi_msk1,roi_msk2,roi_msk);
        bitwise_and(out,roi_msk,roi);
    /****************HSV,threshold****************/
        cvtColor(roi,hsv,COLOR_BGR2HSV);
        inRange(hsv,Scalar(20,80,140),Scalar(32,255,255),yellow_msk);
        inRange(hsv,Scalar(0,0,200),Scalar(180,255,255),white_msk);
        addWeighted(yellow_msk,1.0,white_msk,1.0,0.0,msk);
        bitwise_and(roi,roi,hsv,msk); //결과 : hsv
        threshold(msk, msk_final, 160, 255, THRESH_BINARY);
    /******************histogram******************/
        int histogram[256] = {0};
        for(int y =0; y<msk_final.rows; y++)
            for(int x=0; x<msk_final.cols; x++)
                histogram[(int)msk_final.at<uchar>(y,x)]++;

        drawHist(histogram);
        setMouseCallback(window_name, CallBackFunc, NULL);//마우스
    /******************window ROI*****************/
        for(;;)
        {
        imshow(window_name, msk_final);
        char c = (char)waitKey(0);
        if( c == 27 )
        { break; }
        else if( c == 'i' )
        { pyrUp( src, src, Size( src.cols*2, src.rows*2 ) );
            printf( "** Zoom In: Image x 2 \n" );
        }
        else if( c == 'o' )
        { pyrDown( src, src, Size( src.cols/2, src.rows/2 ) );
            printf( "** Zoom Out: Image / 2 \n" );
        }
        }
    /*****************DetectL*********************/
        //Canny(msk_final,edges,100,400,kernel_size);
        //cvtColor(edges,cdst,COLOR_GRAY2BGR);
            //HoughLinesP(edges, lines, 1, CV_PI / 180, 50, 100, 20);//hough
                //for (size_t i = 0; i < lines.size(); i++) {//hough
                    //Vec4i l = lines[i];//hough
                    //line(out, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255), 3);//hough
                //}
        
    /******************영상보기*******************/
        imshow(window_name, msk_final);
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
