#include <GL/glut.h>
#include <iostream>

GLfloat xRotated, yRotated, zRotated;
// Cone
GLdouble base = 1;
GLdouble height = 1.5;
GLint slices = 50;
GLint stacks = 50;


void displayCone() {

    glMatrixMode(GL_MODELVIEW);

    // clear the drawing buffer.
    glClear(GL_COLOR_BUFFER_BIT);

    // clear the identity matrix.
    glLoadIdentity();

    // traslate the draw by z = -4.0
    // Note this when you decrease z like -8.0 the drawing will looks far , or smaller.
    glTranslatef(0.0, 0.0, -4.5);

    // Red color used to draw.
    glColor3f(0.8, 0.2, 0.1);

    // changing in transformation matrix.
    // rotation about X axis
    glRotatef(xRotated, 1.0, 0.0, 0.0);
    // rotation about Y axis
    glRotatef(yRotated, 0.0, 1.0, 0.0);
    // rotation about Z axis
    glRotatef(zRotated, 0.0, 0.0, 1.0);

    // scaling transfomation
    glScalef(1.0, 1.0, 1.0);


    // built-in (glut library) function , draw you a Cone.
    glutSolidCone(base, height, slices, stacks);

    // Flush buffers to screen
    glFlush();

    // sawp buffers called because we are using double buffering
    // glutSwapBuffers();
}

void reshapeCone(int x, int y) {
    if (y == 0 || x == 0) return;  //Nothing is visible then, so return
    //Set a new projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //Angle of view:40 degrees
    //Near clipping plane distance: 0.5
    //Far clipping plane distance: 20.0

    gluPerspective(40.0, (GLdouble) x / (GLdouble) y, 0.5, 20.0);

    glViewport(0, 0, x, y);  //Use the whole window for rendering
}

void idleCone() {

    yRotated += 0.01;

    displayCone();
}

int main(int argc, char **argv) {
    //Initialize GLUT
    glutInit(&argc, argv);
    //double buffering used to avoid flickering problem in animation
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    // window size
    glutInitWindowSize(400, 350);
    // create the window
    glutCreateWindow("Cone Rotating Animation");
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    xRotated = yRotated = zRotated = 30.0;
    xRotated = 33;
    yRotated = 40;
    glClearColor(0.0, 0.0, 0.0, 0.0);

    //Assign  the function used in events
    glutDisplayFunc(displayCone);
    glutReshapeFunc(reshapeCone);
    glutIdleFunc(idleCone);

    //Let start glut loop
    glutMainLoop();
    return 0;
}