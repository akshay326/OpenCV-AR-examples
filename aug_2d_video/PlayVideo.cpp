#include <stdio.h>
#include <stdlib.h>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>

CvPoint2D32f p[4];

bool insidePolygon(int x,int y){
    // TODO a fair approximation, later turn in to exact polygon
    bool flag = true;
    flag &= p[0].x <= x;
    flag &= p[1].x >= x;
    flag &= p[0].y <= y;
    flag &= p[1].y >= y;

    return flag;
}

void onMouse( int event, int x, int y, int, void* ){
    if(event == CV_EVENT_FLAG_LBUTTON && insidePolygon(x,y))
        std::cout<<"CLicked Me"<< std::endl;
}

int main(){
    CvCapture *capture = 0;
    IplImage  *image = 0;
    IplImage *frame = 0;
    IplImage *neg_img,*cpy_img;
    int key = 0;
    int option = 0;

    capture = cvCreateCameraCapture( 0 );
    if ( !capture )
        return -1;

    //Use a video with aspect ratio 4:3
    CvCapture* vid = cvCreateFileCapture("demo.ogv");
    if ( !vid )
        return -1;

    IplImage *pic = cvLoadImage("pic.jpg");
    cvFlip(pic,pic,1);

    int b_width  = 5;
    int b_height = 4;
    int b_squares = 20;
    CvSize b_size = cvSize( b_width, b_height );
    //The pattern actually has 6 x 5 squares, but has 5 x 4 = 20 'ENCLOSED' corners

    CvMat* warp_matrix = cvCreateMat(3,3,CV_32FC1);
    CvPoint2D32f* corners = new CvPoint2D32f[ b_squares ];
    int corner_count;

    printf("Select an option to run the program\n\n");
    printf("1. Show an Image over the pattern.\n");
    printf("2. Play a Clip over the pattern.\n");
    printf("3. Mark the pattern.\n\n");
    scanf("%d",&option);

    //Quit on invalid entry
    if(!(option>=1 && option<=3))
    {
        printf("Invalid selection.");
        return -1;
    }

    cvNamedWindow("Video",CV_WINDOW_AUTOSIZE);

    while(key!='q')
    {
        image = cvQueryFrame( capture );
        if( !image ) break;
        cvFlip(image,image,1);

        cpy_img = cvCreateImage( cvGetSize(image), 8, 3 );
        neg_img = cvCreateImage( cvGetSize(image), 8, 3 );

        IplImage* gray = cvCreateImage( cvGetSize(image), image->depth, 1);
        int found = cvFindChessboardCorners(image, b_size, corners, &corner_count,
                                            CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS);

        cvCvtColor(image, gray, CV_BGR2GRAY);

        //This function identifies the pattern from the gray image, saves the valid group of corners
        cvFindCornerSubPix(gray, corners, corner_count,  cvSize(11,11),cvSize(-1,-1),
                           cvTermCriteria(CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1 ));

        if( corner_count == b_squares )
        {
            if(option == 1){
                CvPoint2D32f q[4];

                IplImage* blank  = cvCreateImage( cvGetSize(pic), 8, 3);
                cvZero(blank);
                cvNot(blank,blank);

                //Set of source points to calculate Perspective matrix
                q[0].x= (float) pic->width * 0;
                q[0].y= (float) pic->height * 0;
                q[1].x= (float) pic->width;
                q[1].y= (float) pic->height * 0;

                q[2].x= (float) pic->width;
                q[2].y= (float) pic->height;
                q[3].x= (float) pic->width * 0;
                q[3].y= (float) pic->height;

                //Set of destination points to calculate Perspective matrix
                p[0].x= corners[0].x;
                p[0].y= corners[0].y;
                p[1].x= corners[4].x;
                p[1].y= corners[4].y;

                p[2].x= corners[19].x;
                p[2].y= corners[19].y;
                p[3].x= corners[15].x;
                p[3].y= corners[15].y;

                //Calculate Perspective matrix
                cvGetPerspectiveTransform(q,p,warp_matrix);

                //Boolean juggle to obtain 2D-Augmentation
                cvZero(neg_img);
                cvZero(cpy_img);

                cvWarpPerspective( pic, neg_img, warp_matrix);
                cvWarpPerspective( blank, cpy_img, warp_matrix);
                cvNot(cpy_img,cpy_img);

                cvAnd(cpy_img,image,cpy_img);
                cvOr(cpy_img,neg_img,image);

                cvShowImage( "Video", image);
                cvSetMouseCallback("Video",onMouse,0);

            }
            else if(option == 2)
            {
                CvPoint2D32f p[4];
                CvPoint2D32f q[4];

                frame = cvQueryFrame(vid);
                if (!frame)
                    printf("error frame");

                IplImage* blank  = cvCreateImage( cvGetSize(frame), 8, 3);
                cvZero(blank);
                cvNot(blank,blank);

                q[0].x= (float) frame->width * 0;
                q[0].y= (float) frame->height * 0;
                q[1].x= (float) frame->width;
                q[1].y= (float) frame->height * 0;

                q[2].x= (float) frame->width;
                q[2].y= (float) frame->height;
                q[3].x= (float) frame->width * 0;
                q[3].y= (float) frame->height;

                p[0].x= corners[0].x;
                p[0].y= corners[0].y;
                p[1].x= corners[4].x;
                p[1].y= corners[4].y;

                p[2].x= corners[19].x;
                p[2].y= corners[19].y;
                p[3].x= corners[15].x;
                p[3].y= corners[15].y;

                cvGetPerspectiveTransform(q,p,warp_matrix);

                //Boolean juggle to obtain 2D-Augmentation
                cvZero(neg_img);
                cvZero(cpy_img);

                cvWarpPerspective( frame, neg_img, warp_matrix);
                cvWarpPerspective( blank, cpy_img, warp_matrix);
                cvNot(cpy_img,cpy_img);

                cvAnd(cpy_img,image,cpy_img);
                cvOr(cpy_img,neg_img,image);

                cvShowImage( "Video", image);
            }
            else
            {
                CvPoint p[4];

                p[0].x=(int)corners[0].x;
                p[0].y=(int)corners[0].y;
                p[1].x=(int)corners[4].x;
                p[1].y=(int)corners[4].y;

                p[2].x=(int)corners[19].x;
                p[2].y=(int)corners[19].y;
                p[3].x=(int)corners[15].x;
                p[3].y=(int)corners[15].y;

                cvLine( image, p[0], p[1], CV_RGB(255,0,0),2);
                cvLine( image, p[1], p[2], CV_RGB(0,255,0),2);
                cvLine( image, p[2], p[3], CV_RGB(0,0,255),2);
                cvLine( image, p[3], p[0], CV_RGB(255,255,0),2);

                //or simply
                //cvDrawChessboardCorners(image, b_size, corners, corner_count, found);

                cvShowImage( "Video", image);
            }
        }
        else
        {
            //Show gray image when pattern is not detected
            cvFlip(gray,gray);
            cvShowImage( "Video", gray );

        }
        key = cvWaitKey(1);

    }

    cvDestroyWindow( "Video" );
    cvReleaseCapture( &vid );
    cvReleaseMat(&warp_matrix);
    cvReleaseCapture( &capture );

    return 0;
}


