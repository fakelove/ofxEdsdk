#include "testApp.h"

void testApp::setup() {
	ofSetVerticalSync(true);
	ofSetLogLevel(OF_LOG_VERBOSE);
	cameraOne.setup(0);
    index = 0;
}

void testApp::update() {
	cameraOne.update();
	if(cameraOne.isFrameNew()) {
		// process the live view with cameraOne.getLivePixels()
	}
	if(cameraOne.isPhotoNew()) {
		// process the photo with cameraOne.getPhotoPixels()
		// or just save the photo to disk (jpg only):
		cameraOne.savePhoto(ofToString(ofGetFrameNum()) + ".jpg");
	}
}

void testApp::draw() {
    //	cameraOne.draw(0, 0);
	// cameraOne.drawPhoto(0, 0, 432, 288);
	ofBackground(4, 5,6);
	if(cameraOne.isLiveReady()) {
		stringstream status;
        status << cameraOne.getWidth() << "x" << cameraOne.getHeight() << " @ " <<
        (int) ofGetFrameRate() << " app-fps " << " / " <<
        (int) cameraOne.getFrameRate() << " cam-fps";
		ofDrawBitmapString(status.str(), 10, 20);
	}
}

void testApp::keyPressed(int key) {
	if(key == ' ') {
		cameraOne.takePhoto();
	}
    if(key == OF_KEY_RIGHT){
        index++;
        if(index > 2){
            index = 0;
        }
        cameraOne.setCamera(index);
        
    }
}