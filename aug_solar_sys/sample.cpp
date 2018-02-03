#include <iostream>
#include <GL/gl.h>
#include <GL/glu.h>
#include <aruco/aruco.h>
#include <opencv/cv.hpp>
#include "opencv2/core/opengl.hpp"
#include "opencv2/core/cuda.hpp"

using namespace std;
using namespace cv;
using namespace aruco;
using namespace cv::cuda;

const int win_width = 640;
const int win_height = 480;
const char* WIN_NAME = "OpenGL";

struct DrawData {
    ogl::Arrays arr;
    ogl::Texture2D tex;
    ogl::Buffer indices;
    vector<Point2f> points;
    Mat img;
};

void draw(void *userdata);
CameraParameters readCameraParameters();

int main(int argc, char *argv[]) {

    // Read the web cam
    CvCapture *capture = nullptr;
    capture = cvCreateCameraCapture( 0 );
    if ( !capture )
        return -1;

    // To control FPS
    cvSetCaptureProperty(capture,CV_CAP_PROP_FPS,10.0); // My lappy gives 30 by def

    namedWindow(WIN_NAME, WINDOW_OPENGL);
    resizeWindow(WIN_NAME, win_width, win_height);

    Mat_<Vec2f> vertex(1, 4);
    vertex << Vec2f(-1, 1), Vec2f(-1, -1), Vec2f(1, -1), Vec2f(1, 1);

    Mat_<Vec2f> texCoords(1, 4);
    texCoords << Vec2f(0, 0), Vec2f(0, 1), Vec2f(1, 1), Vec2f(1, 0);

    Mat_<int> indices(1, 6);
    indices << 0, 1, 2, 2, 3, 0;

    DrawData data = DrawData();

    data.arr.setVertexArray(vertex);
    data.arr.setTexCoordArray(texCoords);
    data.indices.copyFrom(indices);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(35.0, (double) win_width / win_height, 0.1, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0, 0, 3, 0, 0, 0, 0, 1, 0);

    glEnable(GL_TEXTURE_2D);
    data.tex.bind();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    glDisable(GL_CULL_FACE);

    setOpenGlDrawCallback(WIN_NAME, draw, &data);

    // arUco Detector Structs
    MarkerDetector MDetector;
    vector<Marker> Markers;

    // Marker Info Vectors
    std::vector<int> ids; std::vector<std::vector<cv::Point2f> > corners;

    // Start detection
    MDetector.setDictionary("ARUCO_MIP_36h12");

    // We'll need cam callib for pose estimation
    CameraParameters cp = readCameraParameters();

    // Static points
    // project axis points
    vector <Point3f> axisPoints; // sizes in cms
    axisPoints.emplace_back(Point3f(0, 0, 0));
    axisPoints.emplace_back(Point3f(2, 0, 0));
    axisPoints.emplace_back(Point3f(0, 2, 0));
    axisPoints.emplace_back(Point3f(0, 0, 2));

    vector <Point2f> imagePoints;
    MarkerPoseTracker markerPoseTracker;

    int key=0;
    Mat img;
    while(key != 'q') { // The Main Loop
        img = cvarrToMat(cvQueryFrame(capture));
        data.tex.copyFrom(img);
        data.img = img;

        MDetector.detect(img,Markers);

        //detect markers and update the data
        for (auto &Marker : Markers) {
            cout << Marker << endl;
            markerPoseTracker.estimatePose(Marker, cp, Marker.size(), 4);

            projectPoints(axisPoints, Marker.Rvec, Marker.Tvec, cp.CameraMatrix, cp.Distorsion, imagePoints);
        }

        data.points = imagePoints;

        updateWindow(WIN_NAME);
        key = waitKey(1);
    }

    setOpenGlDrawCallback(WIN_NAME, nullptr, nullptr);
    destroyAllWindows();

    return 0;
}

void draw(void *userdata) {
    auto *data = static_cast<DrawData *>(userdata);

    glRotated(0.6, 0, 1, 0);

    /*
     * Note: To display an image as background
     * U gotta display it as a quad or 2 triangles
     *
     * Ref https://stackoverflow.com/questions/7006213/how-do-you-display-images-in-opengl
     */

    ogl::render(data->arr, data->indices, ogl::TRIANGLES);

    if (data->points.size() > 1) {

        glLineWidth(2.5);
        glColor3f(1.0, 0.0, 0.0);

        glBegin(GL_LINE);
            glVertex2i(0,0);
            glVertex2i(1,1);
//            glVertex2f(data->points[0].x, data->points[0].y);
//            glVertex2f(data->points[1].x, data->points[1].y);
        glEnd();
//    line(data->img, data->points[0], data->points[1], Scalar(0, 0, 255), 3);
//    line(data->img, data->points[0], data->points[2], Scalar(0, 255, 0), 3);
//    line(data->img, data->points[0], data->points[3], Scalar(255, 0, 0), 3);
    }
    // For 3d axis

//    line(data->tex, data->imagePoints[0], data->imagePoints[1], Scalar(0, 0, 255), 3);
//    line(data->tex, data->imagePoints[0], data->imagePoints[2], Scalar(0, 255, 0), 3);
//    line(data->tex, data->imagePoints[0], data->imagePoints[3], Scalar(255, 0, 0), 3);
}

CameraParameters readCameraParameters() {
    try{
        FileStorage fs("calib.yaml", FileStorage::READ);
        if(!fs.isOpened())
            return CameraParameters();

        CameraParameters cp;

        fs["camera_matrix"] >> cp.CameraMatrix;
        fs["distortion_coefficients"] >> cp.Distorsion;
        fs["image_width"] >> cp.CamSize.width;
        fs["image_height"] >> cp.CamSize.height;
        return cp;
    } catch (std::exception &ex){
        cout<<"Exception :"<<ex.what()<<endl;
    }
}