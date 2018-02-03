#include <GL/glut.h>
#include "models.cpp"

/*
 * Simple program
 * Creates a simple Milky Way
 * With only moon, earth, sun
 *
 * Use Models.cpp for creating more planets
 */

GLfloat rotX, rotY;
Planet earth,sun, moon;

void displayMilkyWay() {

    glMatrixMode(GL_MODELVIEW);

    // clear the drawing buffer.
    glClear(GL_COLOR_BUFFER_BIT);

    // clear the identity matrix.
    glLoadIdentity();

    glEnable( GL_TEXTURE_2D );

    // traslate the draw by z = -4.0
    // Note this when you decrease z like -8.0 the drawing will looks far , or smaller.
    glTranslatef(0.0, 0.0, -5);

    glRotatef(rotX,1,0,0);
    glRotatef(rotY,0,1,0);

    // scaling transfomation
    glScalef(1.0, 1.0, 1.0);

    sun.draw();
    earth.revolveAround(sun,1.3, true);
    earth.draw();

    glRotatef(45,1,1,0);
    moon.revolveAround(earth,0.5, true);
    moon.draw();

    // Flush buffers to screen
    glFlush();

    // sawp buffers called because we are using double buffering
    // glutSwapBuffers();
}

void reshapeMilkyWay(int x, int y) {
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

void idleMilkyWay() {
    displayMilkyWay();
}

void onMotion(int Mx, int My){
    rotX = Mx;
    rotY = My;
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);

    //double buffering used to avoid flickering problem in animation
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(450, 450);

    // create the window
    glutCreateWindow("MilkyWay Rotating Animation");
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glClearColor(0.0, 0.0, 0.0, 0.0);

    //Assign  the function used in events
    glutDisplayFunc(displayMilkyWay);
    glutReshapeFunc(reshapeMilkyWay);
    glutIdleFunc(idleMilkyWay);
    glutMotionFunc(onMotion);

    // Init planets
    sun = Planet(0.5,60,0xFCCC43);
    earth = Planet(0.1,20,0x0022EE);
    moon = Planet(0.05,10,0xCCCCCC);

    //Let start glut loop
    glutMainLoop();

    return 0;
}