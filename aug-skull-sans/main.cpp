#include <iostream>
#include <opencv2/imgproc.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "render.h"
#include <dlib/opencv.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <opencv/cv.hpp>

const int deviceId = 0;

static dlib::rectangle openCVRectToDlib(const cv::Rect &r);

void readCameraParams(cv::Mat &, cv::Mat &, int &, int &);

void loadPointsToDetect(std::vector<u_int> &, std::vector<cv::Point3d> &);

void loadPointsToAugment(std::vector<cv::Point3d> &);

float findScaleFactor(const std::vector<cv::Point2d> &);

/* Algorithm
 *     1. Detect face pose - a vector V - using dlib
 *     2. Determine scale factor - S TODO how?
 *     3. Load object
 *     4. Scale the object using `S` TODO how?
 *
 * Approach A: Projection using GLM
 *     5. Generate model's 2D projection,P, - in V's direction - using glm TODO how?
 *     6. Paste P onto the image TODO how?
 *
 * Approach B: Projection using opencv
 *     5. Find points in FOV - in V's direction - TODO how?
 *     6. Project them using opencv
 *
 * x -> vertically up
 * y -> -ve of normal y
 */


int main(int argc, const char *argv[]) {

    cv::CascadeClassifier haar_cascade("haarcascade.xml");

    // Get a handle to the Video device:
    cv::VideoCapture cap(deviceId);
    // Check if we can use this device at all:
    if (!cap.isOpened()) {
        std::cerr << "Capture Device ID " << deviceId << "cannot be opened.\n";
        return -1;
    }

    // Camera Calibration
    int width, height;
    cv::Mat camera_matrix, dist_coeffs, rotation_vector, translation_vector;
    readCameraParams(camera_matrix, dist_coeffs, width, height);

    // Output from face detection stored here
    std::vector<cv::Rect_<int>> faces;
    std::vector<dlib::full_object_detection> shapes;

    // Holds the current frame from the Video device:
    cv::Mat original;
    cv::Mat gray;

    dlib::image_window win;

    // Model for estimating pose
    dlib::shape_predictor pose_model;
    dlib::deserialize("shape_predictor_68_face_landmarks.dat") >> pose_model; // FIXME The file's freaking 64MB

    // Loading the object vectors
    std::vector<unsigned short> indices;
    std::vector<cv::Point3d> indexed_vertices;
    std::vector<cv::Point3d> indexed_normals;
    std::vector<cv::Point3d> vertices;
    std::vector<cv::Point3d> normals;

    // Read our .obj file. Max norm(vertex) ~ 7.5 units
    if (!loadOBJ("skull.obj", vertices, normals))
        return EXIT_FAILURE;

    // Index the indexes
    indexVBO(vertices, normals, indices, indexed_vertices, indexed_normals, 0.35f);

    // At this point normals and vertices are of no use now
    vertices.clear();
    normals.clear();
    std::cout << indexed_vertices.size() << " number of vertices\n";

    // FIXME Remove this. add scaling
    for (auto &indexed_vertice : indexed_vertices) {
        indexed_vertice = indexed_vertice * 5;
        indexed_vertice.x -= 18;
        indexed_vertice.y -= 8;
    }

    // Some points out of the 68 Face points
    std::vector<cv::Point2d> image_points;

    // Loading face points of interest

    std::vector<u_int> POI; //2D real time coordinates
    std::vector<cv::Point3d> model_points; // 3D model points
    loadPointsToDetect(POI, model_points);

    // Project a 3D point (0, 0, 30.0) onto the image plane.
    // We use this to draw a line sticking out of the nose

//    std::vector<cv::Point3d> nose_points3D;
    std::vector<cv::Point2d> nose_points2D;
//    loadPointsToAugment(nose_points3D);

    do {

        cap >> original;

        // Convert the current frame to grayscale:
        cvtColor(original, gray, CV_BGR2GRAY);

        // Find the faces in the frame:
        faces.clear();
        haar_cascade.detectMultiScale(gray, faces, 1.2, 3,
                                      0 | CV_HAAR_SCALE_IMAGE | CV_HAAR_FIND_BIGGEST_OBJECT,
                                      cv::Size(65, 65), // FIXME This decides a lot of execution time
                                      cv::Size(200, 200));

        // Turn OpenCV's Mat into something dlib can deal with.  Don't modify Mat `original` while using cimg.
        dlib::cv_image<dlib::bgr_pixel> cimg(original);

        // Find the pose of each face.
        // pose_model provides 68 points on face
        shapes.clear();
        for (const cv::Rect_<int> &face : faces)
            shapes.push_back(pose_model(cimg, openCVRectToDlib(face)));

        if (!shapes.empty()) {
            // 2D face image points

            image_points.clear();

            for (u_int idx:POI) // TODO Currently estimating pose for only first face
                image_points.emplace_back(cv::Point2d(shapes.at(0).part(idx).x(), shapes.at(0).part(idx).y()));

            // Solve for pose
            cv::solvePnP(model_points, image_points, camera_matrix, dist_coeffs, rotation_vector, translation_vector);

            // NOTE Core drawing Part
//            projectPoints(nose_points3D, rotation_vector, translation_vector, camera_matrix, dist_coeffs,nose_points2D);
            projectPoints(indexed_vertices, rotation_vector, translation_vector, camera_matrix, dist_coeffs,nose_points2D);

//            cv::line(original, nose_points2D[0], nose_points2D[1], cv::Scalar(255, 0, 0), 2);
            for(unsigned int i = 0;i<nose_points2D.size()-1;++i)
                cv::line(original, nose_points2D.at(i), nose_points2D.at(i+1), cv::Scalar(0, 180, 0));
        }

        // Display it all on the screen
        win.clear_overlay();
        win.set_image(cimg);
        win.add_overlay(render_face_detections(shapes));

    } while (cv::waitKey(10) != 27);// Exit this loop on escape:

    return 0;
}

inline static dlib::rectangle openCVRectToDlib(const cv::Rect &r) {
    return {(long) r.tl().x, (long) r.tl().y, (long) r.br().x - 1, (long) r.br().y - 1};
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

void loadPointsToDetect(std::vector<u_int> &POI, std::vector<cv::Point3d> &model_points) {
    // I know numbers are wrong(wrt the image) but it works

    POI.emplace_back(17); // left brow left corner
    POI.emplace_back(21); // left brow right corner
    POI.emplace_back(22); // right brow left corner
    POI.emplace_back(26); // right brow right corner
    POI.emplace_back(36); // left eye left corner
    POI.emplace_back(39); // left eye right corner
    POI.emplace_back(42); // right eye left corner
    POI.emplace_back(45); // right eye right corner
    POI.emplace_back(31); // nose left corner
    POI.emplace_back(35); // nose right corner
    POI.emplace_back(48); // mouth left corner
    POI.emplace_back(54); // mouth right corner
    POI.emplace_back(57); // mouth central bottom corner
    POI.emplace_back(8);  // chin corner

    model_points.emplace_back(cv::Point3d(6.825897, 6.760612, 4.402142));     // left brow left corner
    model_points.emplace_back(cv::Point3d(1.330353, 7.122144, 6.903745));     // left brow right corner
    model_points.emplace_back(cv::Point3d(-1.330353, 7.122144, 6.903745));    // right brow left corner
    model_points.emplace_back(cv::Point3d(-6.825897, 6.760612, 4.402142));    // right brow right corner
    model_points.emplace_back(cv::Point3d(5.311432, 5.485328, 3.987654));     // left eye left corner
    model_points.emplace_back(cv::Point3d(1.789930, 5.393625, 4.413414));     // left eye right corner
    model_points.emplace_back(cv::Point3d(-1.789930, 5.393625, 4.413414));    // right eye left corner
    model_points.emplace_back(cv::Point3d(-5.311432, 5.485328, 3.987654));    // right eye right corner
    model_points.emplace_back(cv::Point3d(2.005628, 1.409845, 6.165652));     // nose left corner
    model_points.emplace_back(cv::Point3d(-2.005628, 1.409845, 6.165652));    // nose right corner
    model_points.emplace_back(cv::Point3d(2.774015, -2.080775, 5.048531));    // mouth left corner
    model_points.emplace_back(cv::Point3d(-2.774015, -2.080775, 5.048531));   // mouth right corner
    model_points.emplace_back(cv::Point3d(0.000000, -3.116408, 6.097667));    // mouth central bottom corner
    model_points.emplace_back(cv::Point3d(0.000000, -7.415691, 4.070434));    // chin corner

    // Size check
    if (POI.size() != model_points.size())
        exit(EXIT_FAILURE);
}

void loadPointsToAugment(std::vector<cv::Point3d> &points3D) {
    //    points3D.emplace_back(cv::Point3d(0, 0, 0)); // cartesian 0,0,0
    points3D.emplace_back(cv::Point3d(0, 1.409845, 6.165652)); // nose top
    points3D.emplace_back(cv::Point3d(0, 0, 30.0));
}

/**
 * @brief Finds scaling factor for the realtime image detected
 * @param points Set of 2D points
 */
float findScaleFactor(const std::vector<cv::Point2d> &points) {
    float s = 0;

    if (points.size() != 68 || points.at(27).x < points.at(28).x)
        exit(EXIT_FAILURE);

    return s;
}