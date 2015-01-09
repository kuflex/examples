#include "testApp.h"

int w = 1024;
int h = 768;

ofShader shader;
ofFbo fbo, fboOut;
ofImage image[2];
int alpha;

ofPixels pix;
int ind = 0;

float _time0 = 0;
float _time = 0;

bool showHelp = true;
float speed = 1.0;


//--------------------------------------------------------------
void testApp::setup(){
    ofSetFrameRate( 60 );
    ofSetWindowTitle( "FracShader" );
    ofSetWindowShape( w, h );
    
	shader.load("shaderVert.c", "shaderFrag.c");

	fbo.allocate(w, h, GL_RGB);
    fboOut.allocate(w, h, GL_RGB);
    
    pix.allocate(w, h, 3);
    
    reinitImage();
    reinitImage();
    alpha = 255;
}

//--------------------------------------------------------------
void testApp::exit() {

}

//--------------------------------------------------------------
void testApp::reinitImage() {
    //generate random colors image
    for (int y=0; y<h; y++) {
        for (int x=0; x<w; x++) {
            pix.setColor( x, y, ofColor( ofRandom(0,256), ofRandom(0,256), ofRandom( 0, 256)));
        }
        
    }
    ind = 1-ind;
    image[ind].setFromPixels( pix );
    alpha = 0;
    
}


//--------------------------------------------------------------
void testApp::update(){
    //fading random images
    alpha += 3;
    alpha = ofClamp( alpha, 0, 255 );
    
    float time = ofGetElapsedTimef();
    float dt = ofClamp( time - _time0, 0.001, 0.1 );
    _time0 = time;
    _time += dt * speed;
}

//--------------------------------------------------------------
void testApp::draw(){
    //Preparing fbo-image as a mix of two random images
	fbo.begin();

	ofSetColor( 255 );
	image[1-ind].draw( 0, 0 );
    ofSetColor( 255, alpha );
    image[ind].draw( 0, 0 );

	fbo.end();

    //Passing fbo-image through shader
    fboOut.begin();
	shader.begin();

	shader.setUniform1f( "time", _time );
    shader.setUniform3f( "iResolution", w, h, 100 );
    
	ofSetColor( 255, 255, 255 );
	fbo.draw( 0, 0 );

	shader.end();
    fboOut.end();
    
    //Output to screen
    ofBackground( 0 );
    ofSetColor( 255 );
    fboOut.draw(0,0, ofGetWidth(), ofGetHeight());
    
    if ( showHelp ) {
        ofSetColor( 0, 0, 255 );
        ofDrawBitmapString( "Keys: Enter-hide this help, s-screenshot, Space-reinit, 1,2-adjust speed, speed=" + ofToString( speed ), 20, 20 );
    }
}

//--------------------------------------------------------------
void testApp::keyPressed  (int key){
    if ( key == ' ' ) reinitImage();
    if ( key == OF_KEY_RETURN ) showHelp = !showHelp;
    if ( key == '1' ) { speed -= 0.05; }
    if ( key == '2' ) { speed += 0.05; }
    speed = ofClamp( speed, 0, 2 );
    
    if ( key == 's' ) {
        string fileName = ofToString(int(ofRandom(0,1000))) + ".png";
        ofSaveScreen(fileName);
    }
    
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){ 

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}

