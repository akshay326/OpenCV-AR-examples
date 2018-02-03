#include <iostream>
#include <aruco/aruco.h>
#include "GL/glut.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv/cv.hpp>

using namespace cv;
using namespace aruco;
using namespace std;

/*
 * Starts the web cam
 * Detects and tracks Aruco markers if present
 * And places a solar system on them
 *
 */

const float zNear = 0.05;
const float zFar = 500.0;
const int width=640, height=480;
const char* WINDOW_NAME="live";
CvCapture *capture = nullptr;

// Function declarations
CameraParameters readCameraParameters();
double getAngle(Point , Point , Point);
String intToString(int number);
bool pairCompare(const pair<float, Point> &i, const pair<float, Point> &j);
GLfloat *convertMatrixType(const Mat &m);
void renderBackgroundGL(const Mat &image);
void display();
void reshape(int,int);
void generateProjectionModelview(const Mat &calibration, const Mat &rotation, const Mat &translation,
                                 Mat &projection, Mat &modelview);

int main(int argc,char **argv){

    // Read the web cam
    capture = cvCreateCameraCapture( 0 );
    if ( !capture )
        return -1;

    // To control FPS
    cvSetCaptureProperty(capture,CV_CAP_PROP_FPS,10.0); // My lappy gives 30 by def

    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(width, height);
    namedWindow(WINDOW_NAME,WINDOW_NORMAL);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(display);

    glutMainLoop();
}

inline double distanceP2P(const Point &a, const Point &b) {
    return sqrt(norm(b-a));
}

double getAngle(Point s, Point f, Point e) {
    double l1 = distanceP2P(f, s);
    double l2 = distanceP2P(f, e);
    double dot = (s.x - f.x) * (e.x - f.x) + (s.y - f.y) * (e.y - f.y);
    double angle = acos(dot / (l1 * l2));
    angle = angle * 180 / 3.147;
    return angle;
}

String intToString(int number) {
    stringstream ss;
    ss << number;
    string str = ss.str();
    return str;
}

bool pairCompare(const pair<float, Point> &i, const pair<float, Point> &j) {
    return i.first < j.first;

}

GLfloat *convertMatrixType(const Mat &m) {
    typedef double precision;

    Size s = m.size();
    auto *mGL = new GLfloat[s.width * s.height];

    for (int ix = 0; ix < s.width; ix++) {
        for (int iy = 0; iy < s.height; iy++) {
            mGL[ix * s.height + iy] = m.at<precision>(iy, ix);
        }
    }

    return mGL;
}

void generateProjectionModelview(const Mat &calibration, const Mat &rotation, const Mat &translation,
                                 Mat &projection, Mat &modelview) {
    typedef double precision;

    projection.at<precision>(0, 0) = 2 * calibration.at<precision>(0, 0) / width;
    projection.at<precision>(1, 0) = 0;
    projection.at<precision>(2, 0) = 0;
    projection.at<precision>(3, 0) = 0;

    projection.at<precision>(0, 1) = 0;
    projection.at<precision>(1, 1) = 2 * calibration.at<precision>(1, 1) / height;
    projection.at<precision>(2, 1) = 0;
    projection.at<precision>(3, 1) = 0;

    projection.at<precision>(0, 2) = 1 - 2 * calibration.at<precision>(0, 2) / width;
    projection.at<precision>(1, 2) = -1 + (2 * calibration.at<precision>(1, 2) + 2) / height;
    projection.at<precision>(2, 2) = (zNear + zFar) / (zNear - zFar);
    projection.at<precision>(3, 2) = -1;

    projection.at<precision>(0, 3) = 0;
    projection.at<precision>(1, 3) = 0;
    projection.at<precision>(2, 3) = 2 * zNear * zFar / (zNear - zFar);
    projection.at<precision>(3, 3) = 0;


    modelview.at<precision>(0, 0) = rotation.at<precision>(0, 0);
    modelview.at<precision>(1, 0) = rotation.at<precision>(1, 0);
    modelview.at<precision>(2, 0) = rotation.at<precision>(2, 0);
    modelview.at<precision>(3, 0) = 0;

    modelview.at<precision>(0, 1) = rotation.at<precision>(0, 1);
    modelview.at<precision>(1, 1) = rotation.at<precision>(1, 1);
    modelview.at<precision>(2, 1) = rotation.at<precision>(2, 1);
    modelview.at<precision>(3, 1) = 0;

    modelview.at<precision>(0, 2) = rotation.at<precision>(0, 2);
    modelview.at<precision>(1, 2) = rotation.at<precision>(1, 2);
    modelview.at<precision>(2, 2) = rotation.at<precision>(2, 2);
    modelview.at<precision>(3, 2) = 0;

    modelview.at<precision>(0, 3) = translation.at<precision>(0, 0);
    modelview.at<precision>(1, 3) = translation.at<precision>(1, 0);
    modelview.at<precision>(2, 3) = translation.at<precision>(2, 0);
    modelview.at<precision>(3, 3) = 1;

    // This matrix corresponds to the change of coordinate systems.
    static double changeCoordArray[4][4] = {{1, 0,  0,  0},
                                            {0, -1, 0,  0},
                                            {0, 0,  -1, 0},
                                            {0, 0,  0,  1}};
    static Mat changeCoord(4, 4, CV_64FC1, changeCoordArray);

    modelview = changeCoord * modelview;
}

void renderBackgroundGL(const Mat &image) {

    GLint polygonMode[2];
    glGetIntegerv(GL_POLYGON_MODE, polygonMode);
    glPolygonMode(GL_FRONT, GL_FILL);
    glPolygonMode(GL_BACK, GL_FILL);


    glLoadIdentity();
    gluOrtho2D(0.0, 1.0, 0.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();


    static bool textureGenerated = false;
    static GLuint textureId;
    if (!textureGenerated) {
        glGenTextures(1, &textureId);

        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        textureGenerated = true;
    }

    // Copy the image to the texture.
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.size().width, image.size().height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE,
                 image.data);

    // Draw the image.
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_TRIANGLES);
    glNormal3f(0.0, 0.0, 1.0);

    glTexCoord2f(0.0, 1.0);
    glVertex3f(0.0, 0.0, 0.0);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(0.0, 1.0, 0.0);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(1.0, 0.0, 0.0);

    glTexCoord2f(1.0, 1.0);
    glVertex3f(1.0, 0.0, 0.0);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(0.0, 1.0, 0.0);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(1.0, 1.0, 0.0);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    // Clear the depth buffer so the texture forms the background.
    glClear(GL_DEPTH_BUFFER_BIT);

    // Restore the polygon mode state.
    glPolygonMode(GL_FRONT, polygonMode[0]);
    glPolygonMode(GL_BACK, polygonMode[1]);
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

void display(){
    try{
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Mat dis_img = cvarrToMat(cvQueryFrame(capture));
        if (!dis_img.data)
            exit(3);

        imshow(WINDOW_NAME,dis_img);
        glutPostRedisplay();

        /*
            // Marker Info Vectors
            std::vector<int> ids; std::vector<std::vector<Point2f> > corners;

            // Start detection
            MDetector.setDictionary("ARUCO_MIP_36h12");
            MDetector.detect(InImage,Markers);

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

            //for each marker, draw id and axis
            for (auto &Marker : Markers) {

                cout << Marker << endl;
                markerPoseTracker.estimatePose(Marker,cp,Marker.size(),4);

                projectPoints(axisPoints, Marker.Rvec, Marker.Tvec, cp.CameraMatrix, cp.Distorsion, imagePoints);

                // For a torus
//                int r = 20;
//                circle(InImage,imagePoints[3],r,Scalar(0, 0, 255),r);

                // For 3d axis
                line(InImage, imagePoints[0], imagePoints[1], Scalar(0, 0, 255), 3);
                line(InImage, imagePoints[0], imagePoints[2], Scalar(0, 255, 0), 3);
                line(InImage, imagePoints[0], imagePoints[3], Scalar(255, 0, 0), 3);
            }
        */
    } catch (std::exception &ex){
        cout<<"Exception :"<<ex.what()<<endl;
    }
}

void reshape(int x, int y) {
    try{
        if (y == 0 || x == 0) return;

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        glMatrixMode(GL_MODELVIEW);
        glViewport(0, 0, x, y);
    } catch (std::exception &ex){
        cout<<"Exception :"<<ex.what()<<endl;
    }
}

void displayShit() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    namedWindow("live", 1);
    Mat gray1, test, modelview, thresh, img1;

    Mat rvec(3, 1, DataType<double>::type);
    Mat tvec(3, 1, DataType<double>::type);

    modelview.create(4, 4, CV_64FC1);

    vector<Point2f> corners1;
    vector<Point2f> imagePoints1;
    vector<Point3f> objectPoints1;
    double largest_area = 0;
    int largest_contour_index = 0;

    clock_t clock_1 = clock();

    Mat dis_img = cvarrToMat(cvQueryFrame(capture));

    if (!dis_img.data)
        exit(3);

    img1 = dis_img.clone();
    dis_img.copyTo(img1);

    cvtColor(dis_img, dis_img, COLOR_BGR2YCrCb);
    inRange(dis_img, Scalar(0, 133, 77), Scalar(255, 173, 127), thresh);
    clock_t clock_2 = clock();
    cout << "threshold(Skin Color Segmentation) time is :" << (double) (clock_2 - clock_1) << endl;
    dilate(thresh, thresh, Mat());
    blur(thresh, thresh, Size(5, 5), Point(-1, -1), BORDER_DEFAULT);
    vector<vector<Point>> contours;
    vector<Point> FingerTips;
    vector<Vec4i> hierachy;
    vector<Vec4i> defects;
    vector<Point> defect_circle;
    vector<vector<Point>> hull(1);
    Point2f center;
    float radius;
    clock_t clock_3 = clock();
    cout << "image filtering (smoothing) time is :" << (double) (clock_3 - clock_2) << endl;


    findContours(thresh, contours, hierachy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

    ulong cont_size = contours.size();
    for (int i = 0; i < cont_size; i++) {
        double a = contourArea(contours[i], false);
        if (a > largest_area) {
            largest_area = a;
            largest_contour_index = i;
        }
    }

    vector<int> hull_index;
    Rect brect;

    if (largest_area > 0 && contours[largest_contour_index].size() > 5) {

        approxPolyDP(contours[largest_contour_index], contours[largest_contour_index], 8, true);
        convexHull(Mat(contours[largest_contour_index]), hull[0], false, true);
        brect = boundingRect(contours[largest_contour_index]);
        convexHull(Mat(contours[largest_contour_index]), hull_index, true);
        convexityDefects(contours[largest_contour_index], hull_index, defects);


        Scalar colorw = Scalar(0, 255, 0);
        Scalar color1 = Scalar(0, 0, 255);
        ulong defc_size = defects.size();

        Point ptStart;
        Point ptEnd;
        Point ptStart2;
        Point ptEnd2;
        Point ptFar;
        int count = 1;

        ulong startidx2=0;

        int tolerance = brect.height / 5;
        float angleTol = 95;
        for (int in = 0; in < defc_size; in++) {
            int startidx = defects[in].val[0];
            ptStart = contours[largest_contour_index].at(startidx);
            int endidx = defects[in].val[1];
            ptEnd = contours[largest_contour_index].at(endidx);
            int faridx = defects[in].val[2];
            ptFar = contours[largest_contour_index].at(faridx);
            if (in + 1 < defc_size)
                startidx2 = defects[in + 1].val[0];
            ptStart = contours[largest_contour_index].at(startidx);
            ptEnd = contours[largest_contour_index].at(endidx);

            if (distanceP2P(ptStart, ptFar) > tolerance && distanceP2P(ptEnd, ptFar) > tolerance &&
                getAngle(ptStart, ptFar, ptEnd) < angleTol) {
                {
                    if (in + 1 < defc_size) {
                        if (distanceP2P(ptStart, ptEnd2) < tolerance)
                            contours[largest_contour_index][startidx] = ptEnd2;
                        else {
                            if (distanceP2P(ptEnd, ptStart2) < tolerance)
                                contours[largest_contour_index][startidx2] = ptEnd;

                        }
                    }
                    defect_circle.push_back(ptFar);
                    //	cout<<"ptfar"<<ptFar.x<<"&&"<<ptFar.y<<endl;

                    if (count == 1) {
                        FingerTips.push_back(ptStart);
                        circle(img1, ptStart, 2, Scalar(0, 255, 0), 2);
                        putText(img1, intToString(count), ptStart - Point(0, 30), FONT_HERSHEY_PLAIN, 1.2f,
                                Scalar(255, 0, 0), 2);
                    }
                    FingerTips.push_back(ptEnd);
                    count++;
                    putText(img1, intToString(count), ptEnd - Point(0, 30), FONT_HERSHEY_PLAIN, 1.2f, Scalar(255, 0, 0),
                            2);
                    circle(img1, ptEnd, 2, Scalar(0, 255, 0), 2);
                    //circle( img, ptFar,   2, Scalar(255,255,255 ), 2 );

                }
            }
        }

        clock_t clock_4 = clock();
        cout << "fingerTip detection  time is :" << (double) (clock_4 - clock_3) << endl;

        Mat Projection(4, 4, CV_64FC1);

        // We'll need cam callib for pose estimation
        CameraParameters cp = readCameraParameters();

        if (defect_circle.size() == 1) {
            Point fn = FingerTips.back();
            FingerTips.pop_back();
            Point ln = FingerTips.back();
            FingerTips.pop_back();
            Point defect_point = defect_circle.back();
            double curr = getAngle(fn, defect_point, ln);
            curr = curr / 10;
            curr = 10 - curr;
            renderBackgroundGL(img1);
            objectPoints1.push_back(Point3d(9, 6, 0));
            imagePoints1.push_back(defect_point);

            objectPoints1.push_back(Point3d(9, 6, 0));
            imagePoints1.push_back(defect_point);


            objectPoints1.push_back(Point3d(19, 6, 0));
            imagePoints1.push_back(fn);

            objectPoints1.push_back(Point3d(9, 18, 0));
            imagePoints1.push_back(ln);


            // cout<<width<<"  &"<<height<<endl;
            //	cout<<"solvepnp"<<endl;
            solvePnP(Mat(objectPoints1), Mat(imagePoints1), cp.CameraMatrix, cp.Distorsion, rvec, tvec);


            Mat rotation;
            Rodrigues(rvec, rotation);
            double offsetA[3][1] = {9, 6, 6};
            Mat offset(3, 1, CV_64FC1, offsetA);
            tvec = tvec + rotation * offset;


            generateProjectionModelview(cp.CameraMatrix, rotation, tvec, Projection, modelview);
            glMatrixMode(GL_PROJECTION);



            GLfloat *projection = convertMatrixType(Projection);
            glLoadMatrixf(projection);
            delete[] projection;

            glMatrixMode(GL_MODELVIEW);
            GLfloat *modelView = convertMatrixType(modelview);
            glLoadMatrixf(modelView);
            delete[] modelView;

            //glTranslatef(0.0f,0.0f,-5.0f);
            glPushMatrix();
            glColor3f(1.0, 0.0, 0.0);

            glutWireTeapot(10.0 / curr);
            glPopMatrix();
            glColor3f(1.0, 1.0, 1.0);

        }
        //Rotation Module
        if (defect_circle.size() == 4) {

            minEnclosingCircle(defect_circle, center, radius);
            //circle(img, center, (int)radius,Scalar(255,255,255), 2, 8, 0 );
            circle(img1, center, 2, Scalar(0), 2, 8, 0);

            vector<pair<float, Point>> pos;
            for (int in = 0; in < FingerTips.size(); in++) {
                Point p = FingerTips.back();
                FingerTips.pop_back();

                pos.emplace_back(make_pair(distanceP2P(center, p), p));
            }

            sort(pos.begin(), pos.end(), pairCompare);

            Point first,second,third;
            first = pos.back().second;
            pos.pop_back();

            second = pos.back().second;
            pos.pop_back();
            third = pos.back().second;
            pos.pop_back();

            Point FIX_X(0, 0), FIX_Y(0, 0);

            if (third.y < second.y && second.y < first.y) {
                FIX_X.x = center.x + 40;
                FIX_X.y = center.y;

                FIX_Y.x = center.x;
                FIX_Y.y = center.y - 40;
            }

            double skew_x,skew_y;
            skew_x = getAngle(first, center, FIX_X);
            skew_y = getAngle(third, center, FIX_Y);
            cout << skew_x << "&" << skew_y << endl;

            if (first.x < img1.cols)
                line(img1, center, first, Scalar(200, 200, 200), 2, 8, 0);
            line(img1, center, FIX_X, Scalar(200, 200, 200), 2, 8, 0);
            if (second.x < img1.cols)
                line(img1, center, second, Scalar(0, 255, 0), 2, 8, 0);
            if (third.x < img1.cols)
                line(img1, center, third, Scalar(0, 0, 255), 2, 8, 0);
            line(img1, center, FIX_Y, Scalar(0, 0, 255), 2, 8, 0);


            renderBackgroundGL(img1);


            objectPoints1.push_back(Point3d(9, 6, 0));
            imagePoints1.push_back(center);

            objectPoints1.push_back(Point3d(9, 18, 0));
            imagePoints1.push_back(first);

            objectPoints1.push_back(Point3d(19, 6, 0));
            imagePoints1.push_back(third);

            objectPoints1.push_back(Point3d(15, 15, 0));
            imagePoints1.push_back(second);


            solvePnP(Mat(objectPoints1), Mat(imagePoints1), cp.CameraMatrix, cp.Distorsion, rvec, tvec);


            Mat rotation;
            Rodrigues(rvec, rotation);
            double offsetA[3][1] = {9, 6, 0};
            Mat offset(3, 1, CV_64FC1, offsetA);
            tvec = tvec + rotation * offset;


            generateProjectionModelview(cp.CameraMatrix, rotation, tvec, Projection, modelview);

            glMatrixMode(GL_PROJECTION);
            GLfloat *projection = convertMatrixType(Projection);
            glLoadMatrixf(projection);
            delete[] projection;

            glMatrixMode(GL_MODELVIEW);
            GLfloat *modelView = convertMatrixType(modelview);
            glLoadMatrixf(modelView);
            delete[] modelView;

            //glTranslat ef(0.0f,0.0f,-5.0f);
            glPushMatrix();
            glColor3f(1.0, 0.0, 0.0);
            glRotatef(skew_x, 1.0, 0.0, 0.0);
            glRotatef(skew_y, 0.0, 1.0, 0.0);
            glutWireTeapot(10.0);
            glPopMatrix();
            glColor3f(1.0, 1.0, 1.0);
            clock_t clock_5 = clock();
            cout << "interaction time is :" << (double) (clock_5 - clock_4) << endl;
        }
        imshow("live", img1);
        cout << "----------------------------------------------" << endl;

        glFlush();
        glutSwapBuffers();
    }
    waitKey(27);
    glutPostRedisplay();
}
