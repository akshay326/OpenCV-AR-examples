#include <iostream>
#include <aruco/aruco.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv/cv.hpp>

using namespace cv;
using namespace aruco;
using namespace std;

#define RED Scalar(0,0,255)

/*
 * Starts the web cam
 * Detects and tracks straight lines
 * I take parallel ones as arms or legs
 */

int main(int argc,char **argv){
    try{

        // Read the web cam
        CvCapture *capture = nullptr;
        capture = cvCreateCameraCapture( 0 );
        if ( !capture )
            return -1;

        // To control FPS
        cvSetCaptureProperty(capture,CV_CAP_PROP_FPS,10.0); // My lappy gives 30 by def
//        double d = cvGetCaptureProperty(capture,CV_CAP_PROP_FPS);

        // start the infinite loop
        int key=0;
        IplImage *image = nullptr;

        int threshold1 = 50;
        int threshold2 = 200;

        Mat dst, cdst, InImage;

        while(key != 'q') {
            //read the input image
            image = cvQueryFrame(capture);

            // Gotta cast the image to Matrix
            // Remember all computations in Mat form, but all visuals aren't so
            InImage = cvarrToMat(image);

            // TO play with params at run time
            switch (key){
                case 'a':
                    threshold1 +=10;
                    break;
                case 's':
                    threshold2 +=10;
                    break;
                case 'z':
                    threshold1 -=10;
                    break;
                case 'x':
                    threshold1 -=10;
                    break;
                default:
                    break;
            }

            Canny(InImage, dst, threshold1, threshold2, 3);
            cvtColor(dst, cdst, CV_GRAY2BGR);

//            vector<Vec2f> lines;
//            HoughLines(dst, lines, 1, CV_PI/180, 100, 0, 0 );
//
//            for( size_t i = 0; i < lines.size(); i++ ){
//                float rho = lines[i][0], theta = lines[i][1];
//                Point pt1, pt2;
//                double a = cos(theta), b = sin(theta);
//                double x0 = a*rho, y0 = b*rho;
//                pt1.x = cvRound(x0 + 1000*(-b));
//                pt1.y = cvRound(y0 + 1000*(a));
//                pt2.x = cvRound(x0 - 1000*(-b));
//                pt2.y = cvRound(y0 - 1000*(a));
//                line( cdst, pt1, pt2, Scalar(0,0,255), 3, CV_AA);
//            }

            vector<Vec4i> lines;
            HoughLinesP(dst, lines, 1, CV_PI/180, 50, 50, 10 );
            for( size_t i = 0; i < lines.size(); i++ ){
                Vec4i l = lines[i];
                line( cdst, Point(l[0], l[1]), Point(l[2], l[3]),RED, 3, CV_AA);
            }

            imshow("source", InImage);
            imshow("detected lines", cdst);

            key = cv::waitKey(1);//wait for key to be pressed
        }

    } catch (std::exception &ex){
        cout<<"Exception :"<<ex.what()<<endl;
    }
}