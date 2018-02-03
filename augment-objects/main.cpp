#include <GL/glew.h>
#include <GL/glut.h>
#include <aruco/aruco.h>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv/cv.hpp>

GLfloat ratioX,ratioY;
aruco::MarkerDetector PPDetector;
aruco::MarkerPoseTracker markerPoseTracker;
cv::VideoCapture TheVideoCapturer;
std::vector<aruco::Marker> TheMarkers;
cv::Mat TheInputImage, TheUndInputImage, TheResizedImage;
aruco::CameraParameters TheCameraParams;
cv::Size TheGlWindowSize;
bool TheCaptureFlag = true;


void displayFunction();
void idleFunction();
void axis(float);
int gl_init();
void onKeyboard(unsigned char Key, int x, int y);
void onMouse(int b, int s, int x, int y);
void readCameraParams(cv::Mat &camera_matrix, cv::Mat &dist_coeffs, int &width, int &height);

int main(int argc, char **argv) {
    glutInit(&argc, argv);

    // read camera parameters
    readCameraParams(TheCameraParams.CameraMatrix, TheCameraParams.Distorsion, TheCameraParams.CamSize.width,
                     TheCameraParams.CamSize.height);
    TheGlWindowSize = TheCameraParams.CamSize;

    //double buffering used to avoid flickering problem in animation
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(TheCameraParams.CamSize.width, TheCameraParams.CamSize.height);

    // create the window
    glutCreateWindow("AruCo");

    // gl init to be called only after context/screen creation
    if (gl_init()==-1)
        return -1;

    // Read video
    TheVideoCapturer.open(0);
    if (!TheVideoCapturer.isOpened()) {
        std::cerr << "Could not open video" << std::endl;
        return -1;
    }

    //Assign  the function used in events
    glutDisplayFunc(displayFunction);
    glutIdleFunc(idleFunction);
    glutKeyboardFunc(onKeyboard);
    glutMouseFunc(onMouse);

    //Let start glut loop
    glutMainLoop();

    return 0;
}

void idleFunction() {
    TheVideoCapturer.grab();
    TheVideoCapturer.retrieve(TheInputImage);
    TheUndInputImage.create(TheInputImage.size(), CV_8UC3);

    // transform color that by default is BGR to RGB because windows systems do not allow reading BGR images with opengl properly
    cv::cvtColor(TheInputImage, TheInputImage, CV_BGR2RGB);

    // remove distortion in image
    cv::undistort(TheInputImage, TheUndInputImage, TheCameraParams.CameraMatrix, TheCameraParams.Distorsion);

    TheCameraParams.CameraMatrix = cv::getOptimalNewCameraMatrix(TheCameraParams.CameraMatrix, TheCameraParams.Distorsion,TheCameraParams.CamSize,1.0);

    TheCameraParams.Distorsion.at<float>(0,0) = 0;
    TheCameraParams.Distorsion.at<float>(0,1) = 0;
    TheCameraParams.Distorsion.at<float>(0,2) = 0;
    TheCameraParams.Distorsion.at<float>(0,3) = 0;
    TheCameraParams.Distorsion.at<float>(0,4) = 0;

    // detect markers
    PPDetector.detect(TheUndInputImage, TheMarkers, TheCameraParams.CameraMatrix);

    // resize the image to the size of the GL window
    cv::resize(TheUndInputImage, TheResizedImage, TheCameraParams.CamSize);
    glutPostRedisplay();
}

void readCameraParams(cv::Mat &camera_matrix, cv::Mat &dist_coeffs, int &width, int &height) {
    cv::FileStorage fs("calib.yaml", cv::FileStorage::READ);
    if (!fs.isOpened()) {
        std::cerr << "Cant open camera calibration file";
        exit(EXIT_FAILURE);
    }

    fs["camera_matrix"] >> camera_matrix;
    fs["distortion_coefficients"] >> dist_coeffs;
    fs["image_width"] >> width;
    fs["image_height"] >> height;
}

void onMouse(int b, int s, int x, int y) {
    if (b == GLUT_LEFT_BUTTON && s == GLUT_DOWN)
        TheCaptureFlag = !TheCaptureFlag;
}

void axis(float size) {
    glLineWidth(2.5);
    glColor3f(1, 0, 0);
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f);  // origin of the line
    glVertex3f(size, 0.0f, 0.0f);  // ending point of the line
    glEnd();

    glLineWidth(2.5);
    glColor3f(0, 1, 0);
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f);  // origin of the line
    glVertex3f(0.0f, size, 0.0f);  // ending point of the line
    glEnd();

    glLineWidth(2.5);
    glColor3f(0, 0, 1);
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f);  // origin of the line
    glVertex3f(0.0f, 0.0f, size);  // ending point of the line
    glEnd();
}

inline void drawLine(){
    glLineWidth(2.5);
    glColor3f(0.0, 0.0, 0.0);

    glBegin(GL_LINES);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.1, 0.1, 0.1);
    glEnd();
}

inline void drawBackground(){
    // draw image in the buffer
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, TheGlWindowSize.width, 0, TheGlWindowSize.height, -1.0, 1.0);
    glViewport(0, 0, TheGlWindowSize.width, TheGlWindowSize.height);
    glDisable(GL_TEXTURE_2D);
    glPixelZoom(1, -1);
    glRasterPos3f(0, TheGlWindowSize.height - 0.5, -1.0);
    glDrawPixels(TheGlWindowSize.width, TheGlWindowSize.height, GL_RGB, GL_UNSIGNED_BYTE, TheResizedImage.ptr(0));
}

inline void drawObjectsOnMarkers(){

    // NOTE I printed the model_view part. It always returned 0. So I removed it
    // NOTE Also Tvec is of no use(as of now). TheMarker.Tvec.ptr<float>(0)[0]
    // NOTE Direction of Rvec vector is the same with the axis of rotation, magnitude of the vector is angle of rotation
    // NOTE Parameters of glTranslate hv units = ratio of screen size. So bottom right corner is (1,1,0)

    for (auto &TheMarker : TheMarkers) {

        // This is the output when i place a marker in image corner -> (x,y)
        // 159=(9.06273,272.557) (11.8356,34.4309) (248.007,31.5478) (246.059,273.382) Txyz=-999999 ...

        // Calculate Tvec and Rvec
        markerPoseTracker.estimatePose(TheMarker, TheCameraParams,TheMarker.size(), 4);
        float TheMarkerSize = 0.1; // in ratio of screen size

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        ratioX = 2*TheMarker.getCenter().x/TheGlWindowSize.width-1;
        ratioY = 1- 2*TheMarker.getCenter().y/TheGlWindowSize.height; // since y is inverted

        glTranslatef(ratioX,ratioY,0);


        glPushMatrix(); // Can add multiple draw calls b/t Push and Pop
        // TODO Small changes in Rvec shud be ignored
        glRotated(cv::norm(TheMarker.Rvec)*57.13,TheMarker.Rvec.at<float>(0),-TheMarker.Rvec.at<float>(1),-TheMarker.Rvec.at<float>(2));
        glColor3f(1, 0.4, 0.4);
        glTranslatef(0, 0, 0);
        axis(TheMarkerSize);
        glPopMatrix();
    }
}

// NOTE x direction is normal 2D one, y direction is inverted
void displayFunction() {
    if (TheResizedImage.rows == 0)  // prevent from going on until the image is initialized
        return;

    // clear
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (TheCaptureFlag) // for debugging purposes
        drawBackground();
    drawObjectsOnMarkers();

    glutSwapBuffers();
}

void onKeyboard(unsigned char Key, int x, int y){
    switch(Key)    {
        case 'o':
            TheCaptureFlag = !TheCaptureFlag;
            break;
        case 27:
            exit(EXIT_SUCCESS);
    };
}

int gl_init(){
    // Initialize GLEW
    glewExperimental = GL_TRUE; // Needed for core profile

    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        return -1;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Cull triangles which normal is not towards the camera
    glEnable(GL_CULL_FACE);

    return 0;
}
