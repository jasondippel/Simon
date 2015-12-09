/*
* main.cpp
*
*  Created on: Nov 27, 2015
*      Author: Team Simon
*
*  Modified on: Dec 8, 2015
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

// hide the local functions/vars in an anon namespace
namespace {
    const int FRAME_WIDTH = 640;
    const int FRAME_HEIGHT = 480;
    int H_MIN = 0;
    int H_MAX = 256;
    int S_MIN = 0;
    int S_MAX = 256;
    int V_MIN = 0;
    int V_MAX = 256;
    const int MAX_NUM_OBJECTS = 1;
    bool mouseIsDragging; // used for showing a rectangle on screen as user clicks and drags mouse
    bool mouseMove;
    bool rectangleSelected;
    bool trackObject;
    cv::Point initialClickPoint, currentMousePoint; // keep track of initial point clicked and current position of mouse
    cv::Rect rectangleROI; // this is the ROI that the user has selected
    vector<int> H_ROI, S_ROI, V_ROI; // HSV values from the click/drag ROI region stored in separate vectors so that we can sort them easily

    /*
     * Basic help function, explains how to use program
     * @param: char** av - params used to call program
     *
     */
    void help(char** av) {
        cout << "The program captures frames from a camera connected to your computer." << endl
             << "Usage:\n" << av[0] << " [ device number = 0 [ FLAGS ] ]" << endl
             << "\tPossible Flags:" << endl
             << "\t\ts - Save video" << endl
             << "\tTo find the device number, try ls /dev/video*" << endl
             << "\texample: " << av[0] << " 0 -s" << endl;
    }


    /*
     * Used to get selection rectangle and set boolean flag to enable detection
     * @param: 
     *
     */
    void clickAndDrag_Rectangle(int event, int x, int y, int flags, void* param) {

        // get handle to video feed passed in as "param" and cast as Mat pointer
        Mat* videoFeed = (Mat*)param;

        if (event == CV_EVENT_LBUTTONDOWN && mouseIsDragging == false) {
            // keep track of initial point clicked
            initialClickPoint = cv::Point(x, y);
            // user has begun dragging the mouse
            mouseIsDragging = true;
        }

        /* user is dragging the mouse */
        if (event == CV_EVENT_MOUSEMOVE && mouseIsDragging == true) {
            // keep track of current mouse point
            currentMousePoint = cv::Point(x, y);
            // user has moved the mouse while clicking and dragging
            mouseMove = true;
        }

        /* user has released left button */
        if (event == CV_EVENT_LBUTTONUP && mouseIsDragging == true) {
            // set rectangle ROI to the rectangle that the user has selected
            rectangleROI = Rect(initialClickPoint, currentMousePoint);

            // reset boolean variables
            mouseIsDragging = false;
            mouseMove = false;
            rectangleSelected = true;
            trackObject = true;
        }

        if (event == CV_EVENT_RBUTTONDOWN) {
            // user has clicked right mouse button
            // Reset HSV Values
            H_MIN = 0;
            S_MIN = 0;
            V_MIN = 0;
            H_MAX = 255;
            S_MAX = 255;
            V_MAX = 255;
        }

        if (event == CV_EVENT_MBUTTONDOWN) {
            // user has clicked middle mouse button
            // enter code here if needed.
        }

    }


    /*
     * Determines appropriate range for HSV values based on selected rectangle
     * @param: Mat &frame - sorce image
     * @param: Mat &hsv_frame - sorce image converted to HSV format
     *
     */
    void recordHSV_Values(Mat &frame, Mat &hsv_frame) {

        // save HSV values for ROI that user selected to a vector
        if (mouseMove == false && rectangleSelected == true) {
		
            // clear previous vector values
            if (H_ROI.size()>0) H_ROI.clear();
            if (S_ROI.size()>0) S_ROI.clear();
            if (V_ROI.size()>0 ) V_ROI.clear();

            // if the rectangle has no width or height (user has only dragged a line) then we don't try to iterate over the width or height
            if (rectangleROI.width < 1 || rectangleROI.height < 1) {
                cout << "Please drag a rectangle, not a line" << endl;
                // don't return, must reset params below

            } else {
                for (int i = rectangleROI.x; i<rectangleROI.x + rectangleROI.width; i++) {

                    // iterate through both x and y direction and save HSV values at every point
                    for (int j = rectangleROI.y; j<rectangleROI.y + rectangleROI.height; j++) {
                        // save HSV value at this point
                        H_ROI.push_back((int)hsv_frame.at<cv::Vec3b>(j, i)[0]);
                        S_ROI.push_back((int)hsv_frame.at<cv::Vec3b>(j, i)[1]);
                        V_ROI.push_back((int)hsv_frame.at<cv::Vec3b>(j, i)[2]);
                    }

                }
            }

            // reset rectangleSelected so user can select another region if necessary
            rectangleSelected = false;

            // set min and max HSV values from min and max elements of each array

            if (H_ROI.size()>0) {
                // NOTE: min_element and max_element return iterators so we must dereference them with "*"
                H_MIN = *std::min_element(H_ROI.begin(), H_ROI.end());
                H_MAX = *std::max_element(H_ROI.begin(), H_ROI.end());
                cout << "MIN 'H' VALUE: " << H_MIN << endl;
                cout << "MAX 'H' VALUE: " << H_MAX << endl;
            }
            if (S_ROI.size()>0) {
                S_MIN = *std::min_element(S_ROI.begin(), S_ROI.end());
                S_MAX = *std::max_element(S_ROI.begin(), S_ROI.end());
                cout << "MIN 'S' VALUE: " << S_MIN << endl;
                cout << "MAX 'S' VALUE: " << S_MAX << endl;
            }
            if (V_ROI.size()>0) {
                V_MIN = *std::min_element(V_ROI.begin(), V_ROI.end());
                V_MAX = *std::max_element(V_ROI.begin(), V_ROI.end());
                cout << "MIN 'V' VALUE: " << V_MIN << endl;
                cout << "MAX 'V' VALUE: " << V_MAX << endl;
            }

        }

        if (mouseMove == true) {
            // if the mouse is held down, we will draw the click and dragged rectangle to the screen
            rectangle(frame, initialClickPoint, Point(currentMousePoint.x, currentMousePoint.y), Scalar(0, 255, 0), 1, 8, 0);
        }

    }


    /*
     * Applies basic image morphing functions in attempt to reduce noice and smooth image
     * @param: Mat &src - source image to be modified
     *
     */
    void morphOps(Mat &src) {
        Mat erodeElement = getStructuringElement(MORPH_RECT, Size(3, 3));
        Mat dilateElement = getStructuringElement(MORPH_RECT, Size(8, 8));

        erode(src, src, erodeElement);
        erode(src, src, erodeElement);

        dilate(src, src, dilateElement);
        dilate(src, src, dilateElement);
    }


    /*
     * Finds areas that contain the color we're looking for
     * @param: Mat &src - reference to source image
     * @param: Mat &dst - reference to destination image
     *
     */
    void findBasicColor(Mat &src, Mat &dst) {
        Mat hsv;

        // where's your balls mister? (orange road hockey ball)
            // convert to hsv so we can eliminate colors
        cvtColor(src, hsv, CV_BGR2HSV);

        recordHSV_Values(src, hsv);
        inRange(hsv, Scalar(H_MIN, S_MIN, V_MIN), Scalar(H_MAX, S_MAX, V_MAX), dst);

        // remove some noise
        morphOps(dst);
    }

    /*
     * Highlights areas of interest with red circles
     * @param: 
     *
     */
    void trackFilteredObject(int &x, int &y, Mat &result, Mat &frame) {
        vector< vector<Point> > contours;
        vector< Vec4i > heirarchy;
        vector< Point2i > center;
        vector< int > radius;

        findContours( result.clone(), contours, heirarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE);

        size_t count = contours.size();
        Scalar red(0, 0, 255);

        // cout << "Count: " << count << endl;
        // int temp = 0;

        // draw circles
        for(int i = 0; i < count; i++) {
            Point2f c;
            float r;
            minEnclosingCircle( contours[i], c, r);

            // eliminate "meaningless" spots
            if(r >= 15) {
                // temp++;
                center.push_back(c);
                radius.push_back(r);

                // cout << "\tCircle" << i << ": " << endl
                //     << "\t\t Center: " << c << endl
                //     << "\t\t Radius: " << r << endl;

                circle(frame, c, r, red, 3);

            }

        }
    }


    /*
     * Processes the video feed
     * @param: VideoCapture &capture - video feed
     *
     */
    int process(VideoCapture &capture, bool saveVid) {
        int n = 0;
        char filename[200];
        int frameWidth = capture.get(CV_CAP_PROP_FRAME_WIDTH);
        int frameHeight = capture.get(CV_CAP_PROP_FRAME_HEIGHT);
        VideoWriter video;
        string windowName = "Team Simon | Webcam";
        Mat frame;
        Mat result;
        int x = 0, y = 0;

        namedWindow(windowName);
        cout << "press q or esc to quit" << endl;

        setMouseCallback(windowName, clickAndDrag_Rectangle, &frame);

        if(saveVid) {
            video.open("simon_output_normal.avi", CV_FOURCC('M', 'J', 'P', 'G'), 10 ,Size(frameWidth, frameHeight), true);
        }

        for (;;) {
            capture >> frame;
            if(frame.empty())
                break;

            // find object
            findBasicColor(frame, result);

            if (trackObject) {
                trackFilteredObject(x, y, result, frame);
            }

            // flip frame for wimpy humans TODO: fix this; selection rectangle not working correctly if frame flipped
            // flip(frame, frame, 1);
            // flip(result, result, 1);

            imshow(windowName, frame);
            imshow("Result", result);

            // save video
            if(saveVid) {
                video.write(frame);
            }

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

/*
 * Main function
 *
 */
int main(int ac, char** av) {

    int devNum = 0;
    bool saveVid = false;

    if (ac == 2) {
        devNum = atoi(av[1]);
    } else if (ac == 3) {
        devNum = atoi(av[1]);
        if(strcmp(av[2],"-s") == 0) {
            saveVid = true;
        } else {
            help(av);
            cout << "Flag: " << av[2] << endl;
        }

    } else if (ac > 3) {
        help(av);
        return 1;
    }
    
    VideoCapture capture(devNum);
    if (!capture.isOpened()) {
        cerr << "Failed to open the video device!\n" << endl;
        help(av);
        return 1;
    }

    //set height and width of capture frame
    capture.set(CV_CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
    capture.set(CV_CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);

    //initiate mouse move and drag to false 
    mouseIsDragging = false;
    mouseMove = false;
    rectangleSelected = false;
    trackObject = false;

    if(saveVid) cout << "Save Video: Yes" << endl;
    else cout << "Save Video: No" << endl;

    return process(capture, saveVid);
}
