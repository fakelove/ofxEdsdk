#include "testApp.h"

void testApp::setup() {
	ofEnableAlphaBlending();
    ofSetVerticalSync( ( true ) );
    ofSetFrameRate(30);
	ofBackground(4, 5, 6);
    ofSetLogLevel(OF_LOG_VERBOSE);
    cameraOne = ofxEdsdk::Camera::getInstance();
    setGUI1();
    bSetup = false;
    
    //mClient = new ofxClientOSCManager();
    //mClient->init();
    
    //mTime = mClient->getCommonTimeOscObj();
}

void testApp::update() {
    //mCurrentTime = mTime->getTimeSecs();
    if(bSetup){
        cameraOne->update();
        if(mNumCameras == 0){
            mNumCameras = cameraOne->numCameras();
            mCameraMap = cameraOne->getSerialMap();
            mCaptureSize.set((ofGetWidth())/mNumCameras, 0);
            gui->addSpacer(255-2*OFX_UI_GLOBAL_WIDGET_SPACING, 2);
            gui->addWidgetDown(new ofxUILabel("CAMERAS", OFX_UI_FONT_MEDIUM));
            for(map<int, string>::iterator itr = mCameraMap.begin(); itr!=mCameraMap.end(); ++itr){
                CameraMapping[itr->second] = "";
                
                gui->addWidgetDown(new ofxUIButton("Cameria:"+ofToString(itr->first)+" serial: "+itr->second, false, 16, 16));
                //gui->addWidget(new ofxUITextInput(itr->second, ofToString(itr->first), OFX_UI_FONT_MEDIUM));
            }
            gui->autoSizeToFitWidgets();
        }
        if(cameraOne->isPhotoNew()) {
            mNewPhoto = true;
            map<string, ofBuffer> images = cameraOne->getImages();
            string dir = ofGetTimestampString();
            ofDirectory::createDirectory(ofGetTimestampString(), true);
            for(map<string, ofBuffer>::iterator itr = images.begin(); itr!=images.end(); ++itr){
                string filename = itr->first;
                if(itr->first == "292074032994"){
                    filename = "1";
                }else if(itr->first == "292074032993"){
                    filename = "2";
                }
                else if(itr->first == "292074032992"){
                    filename = "3";
                }
                
                else if(itr->first == "292074032989"){
                    filename = "4";
                }
                
                else if(itr->first == "342074067873"){
                    filename = "5";
                }
                
                else if(itr->first == "292074036327"){
                    filename = "6";
                }
                
                else if(itr->first == "342074067877"){
                    filename = "7";
                }
                
                else if(itr->first == "292074032991"){
                    filename = "8";
                }
                
                else if(itr->first == "322074063646"){
                    filename = "9";
                }
                
                else if(itr->first == "322074067491"){
                    filename = "10";
                }
                
                else if(itr->first == "322074062352"){
                    filename = "11";
                }
                
                else if(itr->first == "322074067417"){
                    filename = "12";
                }
                
                ofBufferToFile(dir+"/"+filename+".png", itr->second, true);
                ofImage foo;
                foo.loadImage(itr->second);
                captures.push_back(foo);
            }
        }else{
            mNewPhoto = false;
        }
        if(captures.size() > mNumCameras){
            captures.pop_front();
        }
        if(captures.size() > 0 && ofGetFrameNum()%16==0){
            mCurrentIndex++;
            if(mCurrentIndex>=mNumCameras){
                mCurrentIndex = 0;
            }
        }
    }
}

void testApp::draw() {
    ofPushMatrix();
    
    if(captures.size() > mCurrentIndex){
        captures[mCurrentIndex].draw(ofGetWidth()-captures[mCurrentIndex].getWidth()/2, 0, captures[mCurrentIndex].getWidth()/2, captures[mCurrentIndex].getHeight()/2);
    }
    int count = 0;
    for(deque<ofImage>::iterator iter = captures.begin(); iter != captures.end(); ++iter){
        (*iter).draw(ofPoint(count*mCaptureSize.x, ofGetHeight()-mCaptureSize.x*(*iter).height/(*iter).width), mCaptureSize.x*(*iter).width/(*iter).height, mCaptureSize.x*(*iter).height/(*iter).width);
        count++;
    }
    
    
    ofPopMatrix();
    
}

void testApp::setGUI1()
{
	
	float dim = 16;
	float xInit = OFX_UI_GLOBAL_WIDGET_SPACING;
    float length = 255-xInit;
	
	
	gui = new ofxUICanvas(0, 0, length+xInit, ofGetHeight());
    gui->setFont(OF_TTF_MONO);
	gui->addWidgetDown(new ofxUILabel("CANON DOWNLOADER", OFX_UI_FONT_LARGE));
    gui->setDrawPadding(true);
    //gui->setColorFill(ofxUIColor(255, 255, 0));
    gui->addSpacer(length-xInit, 2);
	gui->addWidgetDown(new ofxUILabel("CONTROLS", OFX_UI_FONT_MEDIUM));
	gui->addButton("Capture Image", false, dim, dim);
	gui->addToggle( "Initialize Cameras", false, dim, dim);
    ofAddListener(gui->newGUIEvent, this, &testApp::guiEvent);
}


void testApp::guiEvent(ofxUIEventArgs &e)
{
	string name = e.widget->getName();
	int kind = e.widget->getKind();
    cout<<name<<endl;
    cout<<kind<<endl;
    if(name == "Capture Image"){
        if(bSetup){
            ofxUIButton *button = (ofxUIButton *) e.widget;
            if(button->getValue()){
                capture_count++;
                cameraOne->takePhoto();
            }
        }
    }
    if(name == "Initialize Cameras"){
        if(bSetup){
            cameraOne->exit();
            bSetup = false;
            mNumCameras = 0;
        }
        
        if(!bSetup){
            mNumCameras = 0;
            bSetup = cameraOne->setup();
        }
    }
}

void testApp::keyReleased(int key){
    
}
void testApp::mouseMoved(int x, int y ){
    
}
void testApp::mouseDragged(int x, int y, int button){
    
}
void testApp::mousePressed(int x, int y, int button){
    
}
void testApp::mouseReleased(int x, int y, int button){
    
}
void testApp::windowResized(int w, int h){
    
}

void testApp::keyPressed(int key) {
    
}

void testApp::exit(){
    ofxXmlSettings settings;
    settings.addTag("canon");
    settings.pushTag("canon");
    settings.addValue("booth", 0);
    for(map<int, string>::iterator itr = mCameraMap.begin(); itr!=mCameraMap.end(); ++itr){
        ofxUITextInput* textinput = (ofxUITextInput *) gui->getWidget(itr->second);
        if(textinput != NULL){
            settings.addValue(textinput->getTextString(), ofToString(itr->second));
        }
    }
    settings.popTag();
    
    settings.save("settings.xml");
    if(bSetup){
        cameraOne->exit();
    }
}