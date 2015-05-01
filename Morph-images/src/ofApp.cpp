#include "ofApp.h"

#include "MSAOpenCL.h"

//Particle type - contains all information about particle except particle's position.
typedef struct{
	float4 target;  //target point where to fly
	float4 targetColor;  //target point where to fly
	float speed;    //speed of flying
	float dummy1;
	float dummy2;
	float dummy3;
} Particle;

/*
  Dummy fields are needed to comply OpenCL alignment rule:
  sizeof(float4) = 4*4=16,
  sizeof(float) = 4,
  so overall structure size should divide to 16 and 4.
  Without dummies the size if sizeof(float4)+sizeof(float)=20, so we add
  three dummies to have size 32 bytes.
 */

msa::OpenCL			opencl;

msa::OpenCLBufferManagedT<Particle>	particles;
msa::OpenCLBufferManagedT<float4> particlePos;
msa::OpenCLBufferManagedT<float4> particleColor;

GLuint vbo_pos;
GLuint vbo_color;


int N = 1000000; //Number of particles

//--------------------------------------------------------------
void ofApp::setup(){
    //Screen setup
    ofSetWindowTitle("Morph OpenCL colored example");
    ofSetFrameRate( 60 );
	ofSetVerticalSync(false);
    
    //Camera
	cam.setDistance(600);
    cam.disableMouseInput();    //disable mouse control - we will rotate camera by ourselves
    
    //OpenCL
	opencl.setupFromOpenGL();
	opencl.loadProgramFromFile("Particle.cl");
	opencl.loadKernel("updateParticle");
    
    //create vbo which holds particles positions - particlePos, for drawing
    glGenBuffersARB(1, &vbo_pos);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo_pos);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(float4) * N, 0, GL_DYNAMIC_COPY_ARB);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    
    // init host and CL buffers
    particles.initBuffer( N );
    particlePos.initFromGLObject( vbo_pos, N );
    
    //init vbo_color
    glGenBuffersARB(1, &vbo_color);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo_color);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(float4) * N, 0, GL_DYNAMIC_COPY_ARB);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    
    particleColor.initBuffer( N );
    for (int i=0; i<N; i++) {
        particleColor[i] = //float4(1, 1, 0, 1);
        float4( ofRandom( 0, 1 ), ofRandom( 0, 1 ), ofRandom( 0, 0 ), 0.1 );
    }
    particleColor.initFromGLObject( vbo_color, N );
    particleColor.writeToDevice();
    
    
    morphToImage( "image1.png", true );
	
}

//--------------------------------------------------------------
void ofApp::update(){
    //Update particles positions
    
    //Link parameters to OpenCL (see Particle.cl):
    opencl.kernel("updateParticle")->setArg(0, particles.getCLMem());
	opencl.kernel("updateParticle")->setArg(1, particlePos.getCLMem());
	opencl.kernel("updateParticle")->setArg(2, particleColor.getCLMem());
   
    //Execute OpenCL computation and wait it finishes
    opencl.kernel("updateParticle")->run1D( N );
	opencl.finish();
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(0, 0, 0);
    
    //camera rotate
    float time = ofGetElapsedTimef();
    cam.orbit( sin(time*0.5) * 12, 0, 600, ofPoint( 0, 0, 0 ) );
    cam.begin();
    
    //------------
    //Enabling "addition" blending mode to sum up particles brightnesses
    ofEnableBlendMode( OF_BLENDMODE_ADD );
    
    ofSetColor( 16, 16, 16 );
    glPointSize( 1 );
    
    //link VBOs
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo_pos);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof( float4 ), 0);
    
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo_color );
    glEnableClientState( GL_COLOR_ARRAY );
	glColorPointer(4, GL_FLOAT, sizeof( float4 ), 0);

    
    //Drawing particles
	glDrawArrays(GL_POINTS, 0, N );
    
    //unlink VBOs
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    
    //------------
    
    ofEnableAlphaBlending();    //Restore from "addition" blending mode
    
    cam.end();
    
    ofSetColor( ofColor::white );
    ofDrawBitmapString( "1 - morph to image1, 2 - morph to image2", 20, 20 );
    
}


//--------------------------------------------------------------
void ofApp::morphToImage( string fileName, bool immediately ) {
    
    //Loading image
    ofPixels pix;
    ofLoadImage(pix, fileName );
    int w = pix.getWidth();
    int h = pix.getHeight();

    //Build "distribution array" of brightness
    int sum = 0;
    for (int y=0; y<h; y++) {
        for (int x=0; x<w; x++) {
            sum += pix.getColor(x, y).getBrightness();
        }
    }
    vector<ofPoint> tPnt( sum );
    
    int q = 0;
    for (int y=0; y<h; y++) {
        for (int x=0; x<w; x++) {
            int v = pix.getColor(x, y).getBrightness();
            for (int i=0; i<v; i++) {
                tPnt[q++] = ofPoint( x, y );
            }
        }
    }
    
    //Set up particles
    float scl = 2.0;
    float noisex = 2.5;
    float noisey = 0.5;
    float noisez = 5.0;

    float alpha = 0.1;      //alpha of single particle

    //---------
    //Density of the particles in the particular point of space depends on the image's brightness.
    //So to achive that all drawn particles have equal brightness, uncomment "normalize color" block,
    //and increate "alpha" value.
    //It will give more natural view of the original image.
    //---------

    
    for(int i=0; i<N; i++) {
		Particle &p = particles[i];
        
        //target point and color
        int q = ofRandom( 0, sum );
        ofPoint pnt = tPnt[ q ];
        ofColor color0 = pix.getColor( pnt.x, pnt.y );
        //in OpenGL color is float from 0 to 1, so we divide color here.
        float4 color( color0.r / 255.0, color0.g / 255.0, color0.b / 255.0, color0.a / 255.0 );
        //normalize color ----
        //float bright = color.x + color.y + color.z;
        //color.x /= bright;
        //color.y /= bright;
        //color.z /= bright;
        //------
        color.w *= alpha;
        
        
        pnt.x -= w/2;   //centering
        pnt.y -= h/2;
        pnt.x *= scl;       //scaling
        pnt.y *= -scl;
        
        //add noise to x, y
        pnt.x += ofRandom( -scl/2, scl/2 );
        pnt.y += ofRandom( -scl/2, scl/2 );
        
        pnt.x += ofRandom( -noisex, noisex );
        pnt.y += ofRandom( -noisey, noisey );
        
        //peojection on cylinder
        float Rad = w * scl * 1.0;
        pnt.z = sqrt( fabs( Rad * Rad - pnt.x * pnt.x ) ) - Rad;
        
        //add noise to z
        pnt.z += ofRandom( -noisez, noisez );
        
        //set to particle
        p.target.set( pnt.x, pnt.y, pnt.z, 0 );
        p.targetColor.set( color );
        p.speed = 0.06;   //ofRandom( 0.01, 0.06 );
        
        if ( immediately ) {
            particlePos[i].set( pnt.x, pnt.y, pnt.z, 0 );
            particleColor[i].set( color );
        }
        
    }
    
    //upload to GPU
    particles.writeToDevice();
    
    if ( immediately ) {
        particlePos.writeToDevice();
    }
}


//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if ( key == '1' ) morphToImage( "image1.png", false );
    if ( key == '2' ) morphToImage( "image2.png", false );
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
