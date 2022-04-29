#include <GL/glut.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <GL/glui.h>
#include "OpenFileDialog.h"
#include "SaveFileDialog.h"
#include <vector>
#include <algorithm>

using namespace cv;
using namespace std;

int main_window;
String Filename;
VideoCapture cap;
VideoWriter video;
int ch1, btn;
int playmod;
Mat frame, gray, dst, hough;
Mat cdst;// Hough

Mat videoframe;
int pos = cap.get(CAP_PROP_POS_FRAMES);
int fps, w, h;

vector<Vec4i> lines;

void A() {
    if (playmod == 0) {
        if (cap.get(CAP_PROP_POS_FRAMES) < cap.get(CAP_PROP_FRAME_COUNT)) {
                cap.set(CAP_PROP_POS_FRAMES, pos);
                pos += 3;
                cap >> frame;
                cvtColor(frame, gray, COLOR_BGR2GRAY);
                Canny(gray, dst, 100, 200);
                cvtColor(dst, cdst, COLOR_GRAY2BGR);
                HoughLinesP(dst, lines, 1, CV_PI / 180, 50, 100, 20);
                for (size_t i = 0; i < lines.size(); i++) {
                    Vec4i l = lines[i];
                    line(cdst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255), 3, 4);
                }
                imshow("video", cdst);


                if (ch1 == 1) {
                    //cvtColor(cdst, videoframe, COLOR_GRAY2BGR);
                    video.write(cdst);
                }
        }
    }
}

void button(int id) 
{
    playmod = id;
}

void fileCallBack(int id)  
{
    btn = id;
    OpenFileDialog* openFileDialog = new OpenFileDialog();
    if (openFileDialog->ShowDialog()) {
        Filename = openFileDialog->FileName;
        cap.open(Filename);
        //namedWindow("video", 1);
        pos = 0;
        int count = cap.get(CAP_PROP_FRAME_COUNT);
        printf("count : %d\n", count);
        glutIdleFunc(A);
    }
}

void save(int id)   
{
    if (ch1 == 1) {
        SaveFileDialog* openFileDialog = new SaveFileDialog();
        if (openFileDialog->ShowDialog()) {
            Filename = openFileDialog->FileName;
            int fourcc = VideoWriter::fourcc('M', 'J', 'P', 'G');
            if (btn == 1) {
                fps = cap.get(CAP_PROP_FPS);
                w = cap.get(CAP_PROP_FRAME_WIDTH);
                h = cap.get(CAP_PROP_FRAME_HEIGHT);
                video.open(Filename, fourcc, fps, Size(w, h), 1);
                glutIdleFunc(A);
            }
        }
    }
    else if (ch1 == 0) {
        video.release();
    }
}


int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    GLUI* glui = GLUI_Master.create_glui("week5", 0);
    main_window = glui->get_glut_window_id();
    GLUI_Master.set_glutIdleFunc(NULL);
    GLUI_Panel* panel0 = glui->add_panel("", GLUI_PANEL_NONE);
    GLUI_Panel* panel1 = glui->add_panel_to_panel(panel0, "", GLUI_PANEL_NONE);
    GLUI_Button* button0_1 = glui->add_button_to_panel(panel1, "File", 1, fileCallBack);
    //glui->add_column_to_panel(panel1, false);
    //GLUI_Checkbox* ch0_3 = glui->add_checkbox_to_panel(panel1, "Save", &ch1, 2, save);
    glui->add_separator_to_panel(panel0);
    new GLUI_Button(glui, "QUIT", 0, (GLUI_Update_CB)exit);   
    glui->set_main_gfx_window(main_window);
    glutMainLoop();
    cap.release();
    return EXIT_SUCCESS;
}
