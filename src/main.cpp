/*
* main.cpp
*
*  Created on: Nov 27, 2015
*      Author: Team Simon
*
*  Modified on: Nov 28, 2015
*      Author: Jason Dippel
*
*
* OpenCV VideoCapture and Color Tracking with Webcam
* 
*/

#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <stdio.h>

using namespace cv;
using namespace std;

//hide the local functions in an anon namespace
namespace {

    void help(char** av) {
        cout << "The program captures frames from a camera connected to your computer." << endl
             << "Usage:\n" << av[0] << " [ device number = 0 ]" << endl
             << "\tTo find the device number, try ls /dev/video*" << endl
             << "\texample: " << av[0] << " 0" << endl;
    }

    int process(VideoCapture& capture) {
        int n = 0;
        char filename[200];
        string window_name = "Team Simon | Webcam";
        cout << "press q or esc to quit" << endl;
        namedWindow(window_name, WINDOW_KEEPRATIO); // resizable window;
        Mat frame;
        Mat hsv;

        for (;;) {
            capture >> frame;
            if (frame.empty())
                break;

            // where's your balls mister? (green pb lid)
                // convert to hsv so we can eliminate colors
            cvtColor(frame, hsv, CV_BGR2HSV);

                // only allow green
            Scalar min(70, 200, 50);
            Scalar max(100, 255, 255);
            Mat threshold_frame;
            inRange(hsv, min, max, threshold_frame);

                // get bounding circles
            vector< vector<Point> > contours;
            vector< Vec4i > heirarchy;
            vector< Point2i > center;
            vector< int > radius;

            findContours( threshold_frame.clone(), contours, heirarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE);

            size_t count = contours.size();
            Scalar red(0, 0, 255);

            cout << "Count: " << count << endl;
            int temp = 0;

                // draw circles
            for(int i = 0; i < count; i++) {
                Point2f c;
                float r;
                minEnclosingCircle( contours[i], c, r);

                // eliminate "meaningless" spots
                if(r >= 20) {
                    temp++;
                    center.push_back(c);
                    radius.push_back(r);

                    // cout << "\tCircle" << i << ": " << endl
                    //     << "\t\t Center: " << c << endl
                    //     << "\t\t Radius: " << r << endl;

                    circle(frame, c, r, red, 3);

                }

            }

            cout << "Actual Count: " << temp << endl;

            // flip frame for wimpy humans
            cv::flip(frame, frame, 1);

            imshow(window_name, frame);
            char key = (char)waitKey(2); // delay N ms, usually long enough to display and capture input
                                         // must have waitKey(x); if removed, we see nothing (not like a boss)

            switch (key) {
            case 'q':
            case 'Q':
            case 27: // escape key
                return 0;
            default: // do nothing like a boss
                break;
            }
        }
        return 0;
    }
}

int main(int ac, char** av) {

    int devNum = 0;

    if (ac == 2) {
        devNum = atoi(av[1]);
    } else if (ac > 2) {
        help(av);
        return 1;
    }
    
    VideoCapture capture(devNum);
    if (!capture.isOpened()) {
        cerr << "Failed to open the video device!\n" << endl;
        help(av);
        return 1;
    }

    return process(capture);
}
