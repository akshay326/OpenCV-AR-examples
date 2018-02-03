#include <GL/gl.h>
#include <GL/glut.h>
#include <cmath>
#include <vector>
#include <iostream>

//static std::vector<int> colors;

class Planet{
    int stacks;
    int slices;
    int pos;
    GLfloat radius;
    GLdouble orbitRadius;

    GLUquadricObj *quadric; // Create pointer for storage space for object

    double colorRGB[3];

    void initColor(int hexValue) {
//        colors.push_back(hexValue);

        colorRGB[0] = ((hexValue >> 16) & 0xFF) / 255.0;  // Extract the RR byte
        colorRGB[1] = ((hexValue >> 8) & 0xFF) / 255.0;   // Extract the GG byte
        colorRGB[2] = ((hexValue) & 0xFF) / 255.0;        // Extract the BB byte
    }

public:
    Planet() = default;

    Planet(GLfloat radius, int vertices, int hexColor){
        this->radius = radius;
        this->slices = vertices;
        this->stacks = vertices;
        pos = 0;

        initColor(hexColor);

        quadric = gluNewQuadric(); // Create our new quadric object
        gluQuadricDrawStyle( quadric, GLU_FILL); //FILL also can be line(wire)
        gluQuadricNormals( quadric, GLU_SMOOTH); // For if lighting is to be used.
        gluQuadricOrientation( quadric,GLU_OUTSIDE);
        gluQuadricTexture( quadric, GL_TRUE);// if you want to map a texture to it.
    }

    void draw(){
        glColor3d(colorRGB[0],colorRGB[1],colorRGB[2]);
        glutSolidSphere(radius,slices,stacks);
    }

    /**
     * Makes current planet revolve around the other in an orbit
     *
     * @param host  The Planet about which current planet revolves
     * @param OR orbitRadius, In metres
     * @param drawOrbit Boolean, orbit color is green for all
     */
    void revolveAround(Planet host, double OR, bool drawOrbit){
        // Note we translate the axis, not object. Object always drawn at origin
        // While earth revolves, the frame rotates abt z axis
        pos = (pos + 1)%36000;
        orbitRadius = OR;

        if (drawOrbit)
            sketchOrbit();

        glTranslated(orbitRadius*cos(pos*0.000175), orbitRadius*sin(pos*0.000175), 0);
    }

    void sketchOrbit(){
        glColor3f(0.0, 1.0, 0.0); // orbit color
        gluDisk(quadric,orbitRadius,orbitRadius+0.01,360,1);
    }
};