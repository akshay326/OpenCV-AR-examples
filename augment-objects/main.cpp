#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

#include <aruco/aruco.h>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv/cv.hpp>
#include "render.h"
#include "common/shader.hpp"
#include "common/texture.hpp"


// Main Window
GLFWwindow *window;

// OpenCV + ArUco variables
GLfloat ratioX,ratioY;
aruco::MarkerDetector PPDetector;
aruco::MarkerPoseTracker markerPoseTracker;
cv::VideoCapture TheVideoCapturer;
std::vector<aruco::Marker> TheMarkers;
cv::Mat TheInputImage, TheUndInputImage, TheResizedImage;
aruco::CameraParameters TheCameraParams;
cv::Size TheGlWindowSize;
bool TheCaptureFlag = true;


// IDs need to free up resources
GLuint vertexbuffer;
GLuint normalbuffer;
GLuint elementbuffer;
GLuint programID;
GLuint Texture;
GLuint VertexArrayID;

// Loading the object vectors
std::vector<unsigned short> indices;
std::vector<cv::Point3d> indexed_vertices;
std::vector<cv::Point3d> indexed_normals;
std::vector<cv::Point3d> vertices;
std::vector<cv::Point3d> normals;

void displayFunction();
void idleFunction();
void axis(float);
int glew_init();
int glfw_init();
void glfw_exit();
int loadObjectModels();
void loadSkull();
void resizeCallback(GLFWwindow*, int,int);
void onKeyboard(GLFWwindow*);
void readCameraParams(cv::Mat &camera_matrix, cv::Mat &dist_coeffs, int &width, int &height);

int main(int argc, char **argv) {
    // read camera parameters
    readCameraParams(TheCameraParams.CameraMatrix, TheCameraParams.Distorsion, TheCameraParams.CamSize.width,
                     TheCameraParams.CamSize.height);
    TheGlWindowSize = TheCameraParams.CamSize;

    if (glfw_init()==-1) // Needs CamSize
        return -1;

    if (glew_init()==-1) // gl init to be called only after context/screen creation
        glfw_exit();

    // Read video
    TheVideoCapturer.open(0);
    if (!TheVideoCapturer.isOpened()) {
        std::cerr << "Could not open video" << std::endl;
        glfw_exit();
    }

    if (loadObjectModels() == -1)
        glfw_exit();

    onKeyboard(window);
    //Assign  the function used in events
//    glutDisplayFunc(displayFunction);
//    glutIdleFunc(idleFunction);

    // Main Loop
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0) {
        idleFunction();
        displayFunction();
    }

    glfw_exit();
    return EXIT_SUCCESS;
}

void glfw_exit(){
    // Cleanup VBO and shader
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &normalbuffer);
    glDeleteBuffers(1, &elementbuffer);
    glDeleteProgram(programID);
    glDeleteTextures(1, &Texture);
    glDeleteVertexArrays(1, &VertexArrayID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();
}

int loadObjectModels(){
    // Read our .obj file
    if (!loadOBJ("skull.obj", vertices, normals))
        return -1;

    indexVBO(vertices, normals, indices, indexed_vertices, indexed_normals, 0.35f);

    // At this point normals and vertices are of no use now
    vertices.clear();
    normals.clear();
    std::cout << indexed_vertices.size() << " number of vertices to display\n";

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
    glfwPostEmptyEvent();
//    glutPostRedisplay();
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
//
//
//    // Give the image to OpenGL
//    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, TheGlWindowSize.width, TheGlWindowSize.height, 0, GL_BGR, GL_UNSIGNED_BYTE, TheResizedImage.data);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

//    glDisable(GL_DEPTH_TEST);
//    glMatrixMode(GL_MODELVIEW);
//    glLoadIdentity();
//    glMatrixMode(GL_PROJECTION);
//    glLoadIdentity();
//    glOrtho(0, TheGlWindowSize.width, 0, TheGlWindowSize.height, -1.0, 1.0);
//    glViewport(0, 0, TheGlWindowSize.width, TheGlWindowSize.height);
//    glDisable(GL_TEXTURE_2D);
//    glPixelZoom(1, -1);
//    glRasterPos3f(0, TheGlWindowSize.height - 0.5, -1.0);
//    glDrawPixels(TheGlWindowSize.width, TheGlWindowSize.height, GL_RGB, GL_UNSIGNED_BYTE, TheResizedImage.ptr(0));
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
        loadSkull();
        axis(TheMarkerSize);
        glPopMatrix();
    }
}

// NOTE x direction is normal 2D one, y direction is inverted
void displayFunction() {
    if (TheResizedImage.rows == 0)  // prevent from going on until the image is initialized
        return;

    // Clear the screen
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use our shader
    glUseProgram(programID);

    if (TheCaptureFlag) // for debugging purposes
        drawBackground();
    drawObjectsOnMarkers();

    // Swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
}

void onKeyboard(GLFWwindow *window){
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

int glew_init(){
    // Initialize GLEW
    glewExperimental = GL_TRUE; // Needed for core profile

    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        return -1;
    }
}

void loadSkull(){
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
    programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");

    // Get a handle for our "MVP" uniform
    auto MatrixID = (GLuint) glGetUniformLocation(programID, "MVP");
    auto ViewMatrixID = (GLuint) glGetUniformLocation(programID, "V");
    auto ModelMatrixID = (GLuint) glGetUniformLocation(programID, "M");

    // Load the texture
    Texture = loadDDS("uvmap.DDS");

    // Get a handle for our "myTextureSampler" uniform
    auto TextureID = (GLuint) glGetUniformLocation(programID, "myTextureSampler");
}

int glfw_init(){
    // Initialise GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        getchar();
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(TheGlWindowSize.width, TheGlWindowSize.height, "ArUco", nullptr, nullptr);
    if (window == nullptr) {
        fprintf(stderr,
                "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetFramebufferSizeCallback(window, resizeCallback);

    return 0;
}

void resizeCallback(GLFWwindow* window, int width, int height){
    glViewport(0, 0, width, height);
}