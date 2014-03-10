#pragma once

/*
 ofxEdsdk lets OF talk to Canon cameras using a simple interface.
 */

#include "ofMain.h"
#include "EdsWrapper.h"
#include "RateTimer.h"
#include "FixedQueue.h"

namespace ofxEdsdk {
	class Camera {
        
	public:

        static Camera* getInstance();
        
		bool setup(int deviceId = 0);
		
		void update();
		bool isFrameNew();
		unsigned int getWidth() const;
		unsigned int getHeight() const;
		bool isLiveReady() const;
		void draw(float x, float y);
		void draw(float x, float y, float width, float height);
		ofPixels& getLivePixels();
		ofTexture& getLiveTexture();
		float getFrameRate();
		
		void takePhoto(bool blocking = false);
        void takePhoto(string blocking);
		bool isPhotoNew();
		void drawPhoto(float x, float y);
		void drawPhoto(float x, float y, float width, float height);
		bool savePhoto(string filename); // .jpg only

        void exit();
        vector<string> getSerialNumbers();
        map<int, string> getSerialMap();
		ofPixels& getPhotoPixels();
		ofTexture& getPhotoTexture();
        map<string, ofBuffer> getImages();
        ofBuffer getImage();
        int numCameras();
	protected:
//        class LiveView : public ofThread{
//            static LiveView* getLiveViews();
//            
//        };
        Camera():connected(false),
        liveReady(false),
        liveDataReady(false),
        frameNew(false),
        needToTakePhoto(false),
        photoNew(false),
        needToDecodePhoto(false),
        needToUpdatePhoto(false),
        photoDataReady(false),
        needToSendKeepAlive(false),
        needToDownloadImage(false),
#ifdef  TARGET_OSX
        bTryInitLiveView(false),
#endif
        resetIntervalMinutes(15){};
        Camera( Camera const& ){};
		~Camera();
		EdsCameraRef camera;
        vector<EdsCameraRef> cameras;
        map<string, ofBuffer> mBuffers;
        deque<ofBuffer> dBuff;
        map<int, string> mCamSerials;
        map<string, EdsCameraRef> mCamsSerial;
        vector<string> vCamSerials;
        map<int, string> camName;
		static Camera *mInstance;
		RateTimer fps;
		
		/*
		 Live view data is read from the camera into liveBufferBack when DownloadEvfData()
		 is called. Then the class is locked, and liveBufferBack is quickly pushed
		 onto the liveBufferMiddle queue. When update() is called, the class is
		 also locked to quickly pop from liveBufferMiddle into liveBufferFront.
		 At this point, the pixels are decoded into livePixels and uploaded to liveTexture.
		 */
		ofBuffer liveBufferBack, liveBufferFront;
		FixedQueue<ofBuffer*> liveBufferMiddle;
		ofPixels livePixels;
		ofTexture liveTexture;
		
		/*
		 Photo data is read from the camera into photoBuffer when downloadImage() is
		 called after being triggered by takePhoto(). photoBuffer is only decoded
		 into photoPixels when getPhotoPixels() is called. drawPhoto() will call
		 getPhotoPixels(), and also upload photoPixels to photoTexture. savePhoto()
		 does not decode photoBuffer.
		 */
		ofBuffer photoBuffer;
		ofPixels photoPixels;
		ofTexture photoTexture;
		
		/*
		 There are a few important state variables used for keeping track of what
		 is and isn't ready, and syncing data between the main thread and the
		 capture thread.
		 */
		bool connected; // camera is valid, OpenSession was successful, you can use Eds(camera) now.
		bool liveReady; // Live view is initialized and connected, ready for downloading.
		bool liveDataReady; // Live view data has been downloaded at least once by threadedFunction().
		bool frameNew; // There has been a new frame since the user last checked isFrameNew().
		bool needToTakePhoto; // threadedFunction() should take a picture next chance it gets.
		bool photoNew; // There is a new photo since the user last checked isPhotoNew().
		bool needToDecodePhoto; // The photo pixels needs to be decoded from photo buffer.
		bool needToUpdatePhoto; // The photo texture needs to be updated from photo pixels.
		bool photoDataReady; // Photo data has been downloaded at least once.
		bool needToSendKeepAlive; // Send keepalive next chance we get.
		bool needToDownloadImage; // Download image next chance we get.

		
		// the liveview needs to be reset every so often to avoid the camera turning off
		float resetIntervalMinutes;
		float lastResetTime;
		void resetLiveView();
		
		static EdsError EDSCALLBACK handleObjectEvent(EdsObjectEvent event, EdsBaseRef object, EdsVoid* context);
		static EdsError EDSCALLBACK handlePropertyEvent(EdsPropertyEvent event, EdsPropertyID propertyId, EdsUInt32 param, EdsVoid* context);
		static EdsError EDSCALLBACK handleCameraStateEvent(EdsStateEvent event, EdsUInt32 param, EdsVoid* context);
		
		void setLiveReady(bool liveViewReady);
		void setDownloadImage(EdsDirectoryItemRef directoryItem, string serial);
		void setSendKeepAlive();
		string mCurrentSerial;
		EdsDirectoryItemRef directoryItem;
        

        
#ifdef TARGET_OSX        
        int initTime;
        bool bTryInitLiveView;
#endif
	};
    
    class CameraCallBack{
    public:
        CameraCallBack(){ cam = ofxEdsdk::Camera::getInstance();};
        ~CameraCallBack(){};
        Camera* cam;
        int camIndex;
    };
}