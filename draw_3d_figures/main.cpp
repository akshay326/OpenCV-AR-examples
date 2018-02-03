#include <iostream>
#include <aruco/aruco.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv/cv.hpp>

using namespace cv;
using namespace aruco;
using namespace std;

#define PI 3.14159265

/*
 * Starts the web cam
 * Detects and tracks Aruco markers if present
 * And also places a 3d axis (cartesian) on them
 *
 * Ref
 * https://docs.opencv.org/3.1.0/d5/dae/tutorial_aruco_detection.html
 */

static CameraParameters readCameraParameters();

int main(int argc,char **argv){
    try{
        MarkerDetector MDetector;
        vector<Marker> Markers;
        MDetector.setDictionary("ARUCO_MIP_36h12");

        // Read the web cam
        CvCapture *capture = nullptr;
        capture = cvCreateCameraCapture( 0 );
        if ( !capture )
            return -1;

        // To control FPS
        cvSetCaptureProperty(capture,CV_CAP_PROP_FPS,4.0); // My lappy gives 30 by def
//        double d = cvGetCaptureProperty(capture,CV_CAP_PROP_FPS);

        // start the infinite loop
        int key=0;
        IplImage *image = nullptr;

        // Static points
        // project axis points
        vector <Point3f> axisPoints; // sizes in cms
        axisPoints.emplace_back(Point3f(0, 0, 0));
        axisPoints.emplace_back(Point3f(2, 0, 0));
        axisPoints.emplace_back(Point3f(0, 2, 0));
        axisPoints.emplace_back(Point3f(0, 0, 2));

        vector <Point3f> SEPoints; // Sun - Earth Points
        int vertices = 10; // no of vertices in the circle u want
        float SEORadius = 2.5;
        for(int i=-vertices;i<vertices;++i)
            SEPoints.emplace_back(Point3f(SEORadius*cosf(PI*i/vertices),SEORadius*sinf(PI*i/vertices),2)); // See that z!=0, for some elevation

        vector <Point3f> MEPoints; // Moon - Earth points
        float MEORadius = 1;

        // We'll need cam callib for pose estimation
        CameraParameters cp = readCameraParameters();

        // Gotta cast the image to Matrix
        // Remember all computations in Mat form, but all visuals aren't so
        cv::Mat InImage;

        vector <Point2f> imagePoints;

        int rSun = 20;
        int rEarth = 10;
        int rMoon = 5;
        vector <Point2f> EarthPP; // Earth's Projected Points
        vector <Point2f> MoonPP; // Moons's Projected Points

        MarkerPoseTracker markerPoseTracker;
        int earthPosition = 0;
        int moonPosition = 15; // So that they appear distinct

        // Marker Info Vectors
        std::vector<int> ids; std::vector<std::vector<cv::Point2f> > corners;

        while(key != 'q') {
            //read the input image
            image = cvQueryFrame(capture);
            InImage = cv::cvarrToMat(image);

            // Moon points not set yet
            MEPoints.clear();
            for(int i=-vertices;i<vertices;++i) // Made it gucking complex
                MEPoints.emplace_back(Point3f(SEPoints.at(earthPosition).x,SEPoints.at(earthPosition).y+MEORadius*sinf(PI*i/vertices),2+MEORadius*cosf(PI*i/vertices))); // See that z!=0, for some elevation

            // Start detection
            MDetector.detect(InImage,Markers);

            //for each marker, draw id and axis
            for (auto &Marker : Markers) {

//                cout << Marker << endl;
                markerPoseTracker.estimatePose(Marker,cp,Marker.size(),4);

                // TODO Most important function
                projectPoints(axisPoints, Marker.Rvec, Marker.Tvec, cp.CameraMatrix, cp.Distorsion, imagePoints);
                projectPoints(SEPoints, Marker.Rvec, Marker.Tvec, cp.CameraMatrix, cp.Distorsion, EarthPP);
                projectPoints(MEPoints, Marker.Rvec, Marker.Tvec, cp.CameraMatrix, cp.Distorsion, MoonPP);

//                // For a torus
//                int r = 20;
//                circle(InImage,imagePoints[3],r,Scalar(0, 0, 255),r);

//                // For 3d axis
//                line(InImage, imagePoints[0], imagePoints[1], Scalar(0, 0, 255), 3);
//                line(InImage, imagePoints[0], imagePoints[2], Scalar(0, 255, 0), 3);
//                line(InImage, imagePoints[0], imagePoints[3], Scalar(255, 0, 0), 3);

                // For a demo solar sys
                // Sun Earth orbit
                for (ulong j=0;j<2*vertices-1;++j) {
                    line(InImage, (Point) EarthPP.at(j), (Point) EarthPP.at(j+1), Scalar(0, 0, 255), 2);
                }
                line(InImage, (Point) EarthPP.at(0), (Point) EarthPP.at(2*vertices-1), Scalar(0, 0, 255), 2);

                // Sun
                circle(InImage,imagePoints[3],rSun,Scalar(0,255, 255),-1);

                // Now displaying earth
                earthPosition = (earthPosition+1)%(2*vertices);
                circle(InImage,(Point) EarthPP.at(earthPosition),rEarth,Scalar(255,20, 0),-1);
                
                // Now displaying moon and orbit
                for (ulong j=0;j<2*vertices-1;++j) {
                    line(InImage, (Point) MoonPP.at(j), (Point) MoonPP.at(j+1), Scalar(225, 0, 255), 2);
                }
                line(InImage, (Point) MoonPP.at(0), (Point) MoonPP.at(2*vertices-1), Scalar(225, 0, 255), 2);
                
                // Now displaying moon
                moonPosition = (moonPosition+1)%(2*vertices);
                circle(InImage,(Point) MoonPP.at(moonPosition),rMoon,Scalar(200,200, 200),-1);

            }

            cv::imshow("in", InImage);
            key = cv::waitKey(1);//wait for key to be pressed
        }

    } catch (std::exception &ex)
    {
        cout<<"Exception :"<<ex.what()<<endl;
    }
}

static CameraParameters readCameraParameters() {
    cv::FileStorage fs("calib.yaml", cv::FileStorage::READ);
    if(!fs.isOpened())
        return CameraParameters();

    CameraParameters cp;

    fs["camera_matrix"] >> cp.CameraMatrix;
    fs["distortion_coefficients"] >> cp.Distorsion;
    fs["image_width"] >> cp.CamSize.width;
    fs["image_height"] >> cp.CamSize.height;
    return cp;
}
