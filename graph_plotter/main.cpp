#include <iostream>
#include <aruco/aruco.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv/cv.hpp>

using namespace cv;
using namespace aruco;
using namespace std;

#define LINE_THICKNESS 3
#define AXIS_LENGTH 4.0f
#define NO_OF_SLICES 20
#define PI 3.1457f
#define VIEWPORT (3*NO_OF_SLICES)
#define EPS 0.001f

float factor = 0.1,wavelength=AXIS_LENGTH/8, amplitude = AXIS_LENGTH/3;

static CameraParameters readCameraParameters();

inline float plot_function2d(float);
inline float plot_function3d(float,float);

// Draw a 2d graph
int main(int argc,char **argv) {
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
        vector <Point3f> axis_points; // sizes in cms
        axis_points.emplace_back(Point3f(0, 0, 0));
        axis_points.emplace_back(Point3f(AXIS_LENGTH, 0, 0));
        axis_points.emplace_back(Point3f(0, AXIS_LENGTH, 0));
        axis_points.emplace_back(Point3f(0, 0, AXIS_LENGTH));

        // sine curve points in range [AXIS_LENGTH,AXIS_LENGTH]^3
        vector <Point3f> function_points; // sizes in cms
        
        // We'll need cam callib for pose estimation
        CameraParameters cp = readCameraParameters();

        // Gotta cast the image to Matrix
        // Remember all computations in Mat form, but all visuals aren't so
        cv::Mat InImage;

        vector <Point2f> image_points;
        vector <Point2f> function_image_points;
        vector <float> function_values;
        vector <Scalar> color_map; // need to specify colors also

        MarkerPoseTracker markerPoseTracker;

        // Marker Info Vectors
        std::vector<int> ids; std::vector<std::vector<cv::Point2f> > corners;

        // To avoid redeclaration
        float x,y,z, min_val,max_val;
        int i,j;

        while(key != 'q') {
            switch(key){
                case 'a':
                    amplitude+=factor;
                    break;
                case 'z':
                    amplitude-=factor;
                    break;
                case 's':
                    wavelength-=factor;
                    break;
                case 'w':
                    wavelength+=factor;
                    break;
                case 'f':
                    factor+=0.1;
                    break;
                case 'v':
                    factor-=0.1;
                    break;
                default:break;
            }

            // Get 3d function points
            function_points.clear();
            function_values.clear();

//            // for univariable functions
//            for(i=-VIEWPORT;i<VIEWPORT;++i) {
//                x = i * 1.0f / NO_OF_SLICES;
//
//                y = plot_function2d(x);
//                function_values.emplace_back(y);
//
//                function_points.emplace_back(Point3f(1.0, 1.0, 1.0) + Point3f(x, y, 0));
//            }

            // for bi variable functions
            for(i=-VIEWPORT;i<VIEWPORT;++i) {
                x = i * 1.0f / NO_OF_SLICES;
                for(j=-VIEWPORT;j<VIEWPORT;++j) {
                    y = j * 1.0f / NO_OF_SLICES;

                    z = plot_function3d(x,y);
                    function_values.emplace_back(z);

                    function_points.emplace_back(Point3f(1.0, 1.0, 1.0) + Point3f(x, y, z));
                }
            }

            // Define the color map
            max_val = *max_element(function_values.begin(),function_values.end());
            min_val = *min_element(function_values.begin(),function_values.end());

            for(i=0;i<function_points.size();++i)
                color_map.emplace_back(Scalar(255 * function_values.at(i)/(max_val - min_val+EPS),
                                              0,255*(1-function_values.at(i)/(max_val - min_val+EPS))));


            //read the input image
            image = cvQueryFrame(capture);
            InImage = cv::cvarrToMat(image);

            // Start detection
            MDetector.detect(InImage,Markers);

            //for each marker, draw id and axis
            for (auto &Marker : Markers) {

//                cout << Marker << endl;
                markerPoseTracker.estimatePose(Marker,cp,Marker.size(),4);

                // TODO Most important function
                projectPoints(axis_points, Marker.Rvec, Marker.Tvec, cp.CameraMatrix, cp.Distorsion, image_points);
                projectPoints(function_points, Marker.Rvec, Marker.Tvec, cp.CameraMatrix, cp.Distorsion, function_image_points);

                // For 3d axis
                line(InImage, image_points[0], image_points[1], Scalar(0, 0, 255), LINE_THICKNESS);
                line(InImage, image_points[0], image_points[2], Scalar(0, 255, 0), LINE_THICKNESS);
                line(InImage, image_points[0], image_points[3], Scalar(255, 0, 0), LINE_THICKNESS);
                
                // for the function
                // currently using sin curve
                for(unsigned long k = 0;k<function_image_points.size();++k)
                    circle(InImage,function_image_points.at(k),2,color_map.at(k));
            }

            cv::imshow("in", InImage);
            key = cv::waitKey(1);//wait for key to be pressed
        }

    } catch (std::exception &ex){
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

inline float plot_function2d(float x){
    return amplitude*sin(2 * PI *x/wavelength);
}

inline float plot_function3d(float x, float y){

//    x = powf(x,2) + powf(y,2) + EPS;
    // abelson
//    return 2*sinf(x)/sqrtf(x);

//     paraboloid
    return (powf(x,2) + powf(y,2) - 2)/4;
}


