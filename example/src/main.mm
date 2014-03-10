#include "testApp.h"
#include "ofxCocoaWindow.h"
#include "ofxWatchdog.h"
int main( )
{
//    ofxWatchdog::watch(5000, true, true, true);
    ofxCocoaWindow cocoaWindow;
    ofSetupOpenGL(&cocoaWindow, 1680, 1050, OF_WINDOW);
	ofRunApp(new testApp());
}
 