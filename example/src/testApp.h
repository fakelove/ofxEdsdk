#pragma once

#include "ofMain.h"
#include "ofxEdsdk.h"
#include "ofxUI.h"
#include "ofxOSCSync.h"
class testApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void keyPressed  (int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	void exit();
    void setGUI1();
	ofxEdsdk::Camera* cameraOne;
    deque<ofImage> captures;
    map<int, string> mCameraMap;
    bool bSetup;
    int capture_count;
    int mNumCameras;
    int mCurrentIndex;
    string mLastFile;
    bool mNewPhoto;
    ofPoint mCaptureSize;
    ofxUICanvas *gui;
	void guiEvent(ofxUIEventArgs &e);
    ofxClientOSCManager* mClient;
    ofxCommonTimeOSC* mTime;
    float mCurrentTime;
    map<string, string> CameraMapping;
};
