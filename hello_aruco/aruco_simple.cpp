#include <iostream>
#include <aruco/aruco.h>
#include <opencv2/highgui/highgui.hpp>
using namespace cv;
using namespace aruco;
using namespace std;

/*
 * Starts the web cam
 * Detects and tracks Aruco markers if present
 */

int main(int argc,char **argv){
    try{
        MarkerDetector MDetector;
        vector<Marker> Markers;

        // Read the web cam
        CvCapture *capture = 0;
        capture = cvCreateCameraCapture( 0 );
        if ( !capture )
            return -1;

        // start the infinite loop
        int key=0;
        IplImage *image = 0;
        while(key != 'q') {
            //read the input image
            image = cvQueryFrame(capture);

            // Gotta cast the image to Matrix
            // Remember all computations in Mat form, but all visuals aren't so
            cv::Mat InImage;
            InImage = cv::cvarrToMat(image);

            //Ok, let's detect
            MDetector.detect(InImage, Markers);

            //for each marker, draw info and its boundaries in the image
            for (auto &Marker : Markers) {
                cout << Marker << endl;
                Marker.draw(InImage, Scalar(0, 0, 255), 2);
            }

            cv::imshow("in", InImage);
            key = cv::waitKey(1);//wait for key to be pressed
        }

    } catch (std::exception &ex)
    {
        cout<<"Exception :"<<ex.what()<<endl;
    }
}


