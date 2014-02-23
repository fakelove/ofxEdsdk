#include "testApp.h"

void testApp::setup() {
	ofSetVerticalSync(true);
	ofSetLogLevel(OF_LOG_VERBOSE);
	cameraOne.setup();
    mNumCameras = cameraOne.numCameras();
}

void testApp::update() {
	cameraOne.update();
	if(cameraOne.isPhotoNew()) {
		// process the photo with cameraOne.getPhotoPixels()
		// or just save the photo to disk (jpg only):
        mNewPhoto = true;
		cameraOne.savePhoto(ofToString(ofGetFrameNum()) + ".jpg");
	}else if(ofGetFrameNum()%15 == 0){
        mNewPhoto = false;
    }
}

void testApp::draw() {
	ofBackground(4, 5,6);
    
    for(int i = 0 ; i < mNumCameras; i++){
        ofSetColor((255*i)%255, (100*i)%255, (150*i)%255);
        ofRect(i*ofGetWidth()/mNumCameras, 0, ofGetWidth()/mNumCameras, ofGetHeight());
    }
    
    if(mNewPhoto){
        ofSetColor(255, 0, 255);
        ofRect(0, 0, ofGetWidth(), ofGetHeight());
    }
}

void testApp::keyPressed(int key) {
	if(key == ' ') {
		cameraOne.takePhoto();
	}
}