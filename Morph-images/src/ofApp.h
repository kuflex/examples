#pragma once

//---------------------------------------------------------------
//This is example of using OpenCL for creating particle system,
//which morphs between two images
//
//Control keys: 1 - morph to image1, 2 - morph to image2
//
//All drawn particles have equal brightness, so we achieve image-like
//particles configuration by placing different number of particles
//at each pixel and draw them using "addition blending" mode.
//
//Project is developed for openFrameworks 8.4_osx and is based
//on example-Particles example of ofxMSAOpenCL adoon.
//It uses addons ofxMSAOpenCL and ofxMSAPingPong.
//For simplicity this addons are placed right in the project's folder.
//
//Kuflex.com, 2015: Denis Perevalov, Igor Sodazot.
//---------------------------------------------------------------


#include "ofMain.h"

class ofApp : public ofBaseApp{
    
public:
    void setup();
    void update();
    void draw();
    
    void morphToImage( string fileName, bool immediately );
    
    ofEasyCam cam; // add mouse controls for camera movement

    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
};
