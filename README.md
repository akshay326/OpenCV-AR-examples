# OpenCV-AR-examples
Welcome to my repo for learning AR using OpenCV and other open source tools. **This is not a tutorial** for learning computer vision or Augmented Reality software(*you can easily Google out a lot of them*), rather this repository is meant to be a collection of apps and other programs which you might find useful while learning AR.

![ArUco+OGRE](http://agilis-lab.com/wp-content/uploads/2012/10/imagenogre.jpg)

# Getting Ready
Apart from having a PC/Laptop(with a webcam) or Android Mobile(with camera), you must be familiar with C++ and a few of its libraries:
+ [OpenCV](https://opencv.org/) - An open source computer vision library. Available in C++ and python
+ [OpenGL](https://www.opengl.org/) - An open source industry acclaimmed design libary. Available in C,C++ and Java
+ [ArUco](https://www.uco.es/investiga/grupos/ava/node/26) - A contributing module for OpenCV, provides easy square marker detection and pose estimation. I've used it in most of the programs. 

# Development in Android
Android offers the ease of obtaining image from mobile Camera and recent devices also provide GPU and high resolution graphics integration. Couple that with Native Code([Android NDK](https://developer.android.com/ndk/index.html)), you have the same PC app ready in Android.
+ Since Android supports OpenGL library, the object rendering part in the source code should be omitted
+ As of now, processing speed is not an issue and the RAM + Memory that current devices support is considerate enough.

# Contributing
The collection is not comprehensive, since I'm also learning vision and other stuff myself. Any recommendations, glitches, or error as a pull request are welcomed. 
+ Since most of the examples are in C++, I'll be thankful if you can contribute a few of these programs in python.
