#include "ofxEdsdk.h"

/*
 This controls the size of liveBufferMiddle. If you are running at a low fps
 (lower than 30 fps), then it will effectively correspond to the latency of the
 camera. If you're running higher than 30 fps, it will determine how many frames
 you can miss without dropping one. For example, if you are running at 60 fps
 but one frame happens to last 200 ms, and your buffer size is 4, you will drop
 2 frames.
 */
#define OFX_EDSDK_BUFFER_SIZE 4
#define OFX_EDSDK_LIVE_DELAY 10
#ifdef TARGET_WIN32
#define _WIN32_DCOM
#include <objbase.h>
#endif
namespace ofxEdsdk {
    Camera* Camera::mInstance = NULL;
    
    Camera* Camera::getInstance(){
        if( mInstance == NULL ){
            mInstance = new Camera();
        }
        return mInstance;
    }
    
	EdsError EDSCALLBACK Camera::handleObjectEvent(EdsObjectEvent event, EdsBaseRef object, EdsVoid* context) {
		//ofLogVerbose() << "object event " << Eds::getObjectEventString(event);
		if(object) {
			if(event == kEdsObjectEvent_DirItemCreated) {
				CameraCallBack* fooCam = ((CameraCallBack*) context);
                fooCam->cam->setDownloadImage(object, fooCam->cam->mCamSerials[fooCam->camIndex]);
			} else if(event == kEdsObjectEvent_DirItemRemoved) {
				// no need to release a removed item
			} else {
				try {
					Eds::SafeRelease(object);
				} catch (Eds::Exception& e) {
					ofLogError() << "Error while releasing EdsBaseRef inside handleObjectEvent()";
				}
			}
		}
		return EDS_ERR_OK;
	}
	
	EdsError EDSCALLBACK Camera::handlePropertyEvent(EdsPropertyEvent event, EdsPropertyID propertyId, EdsUInt32 param, EdsVoid* context) {
		//ofLogVerbose() << "property event " << Eds::getPropertyEventString(event) << ": " << Eds::getPropertyIDString(propertyId) << " / " << param;
		if(propertyId == kEdsPropID_Evf_OutputDevice) {
			((Camera*) context)->setLiveReady(true);
		}
		return EDS_ERR_OK;
	}
	
	EdsError EDSCALLBACK Camera::handleCameraStateEvent(EdsStateEvent event, EdsUInt32 param, EdsVoid* context) {
		//ofLogVerbose() << "camera state event " << Eds::getStateEventString(event) << ": " << param;
		if(event == kEdsStateEvent_WillSoonShutDown) {
			((Camera*) context)->setSendKeepAlive();
		}
		return EDS_ERR_OK;
	}
	
	Camera::~Camera() {
		mInstance = NULL;
	}
    void Camera::exit(){
        if(connected) {

			if(liveReady) {
				Eds::EndLiveview(camera);
			}
            for(int i = 0; i < cameras.size(); i++){
                
                try {
                    Eds::CloseSession(cameras[i]);
                } catch (Eds::Exception& e) {
                    ofLogError() << "There was an error destroying ofxEds::Camera: " << e.what();
                }
                
                try {
                    Eds::SafeRelease(cameras[i]);
                } catch (Eds::Exception& e) {
                    ofLogError() << "There was an error destroying ofxEds::Camera: " << e.what();
                }
            }
            
            cameras.clear();
            mBuffers.clear();
            mCamSerials.clear();
            vCamSerials.clear();
            camName.clear();
            dBuff.clear();
            try {
                Eds::TerminateSDK();
            } catch (Eds::Exception& e) {
                ofLogError() << "There was an error destroying ofxEds::Camera: " << e.what();
            }
		}
		for(int i = 0; i < liveBufferMiddle.maxSize(); i++) {
			delete liveBufferMiddle[i];
		}
    }
    
	bool Camera::setup(int deviceId) {
		try {
			Eds::InitializeSDK();
			
			EdsCameraListRef cameraList;
			Eds::GetCameraList(&cameraList);
			
			EdsUInt32 cameraCount;
			Eds::GetChildCount(cameraList, &cameraCount);
			
			if(cameraCount > 0) {
				for(int i = 0; i < cameraCount; i++){
                    
                    EdsCameraRef fooRef;
                    
                    EdsInt32 cameraIndex = i;
                    Eds::GetChildAtIndex(cameraList, cameraIndex, &fooRef);
                    EdsDeviceInfo info;
                    Eds::GetDeviceInfo(fooRef, &info);
                    ofLogVerbose("ofxEdsdk::setup") << "connected camera model: " <<  info.szDeviceDescription << " " << info.szPortName <<" of "<<cameraCount<< endl;
                    cameras.push_back(fooRef);
                }
                Eds::SafeRelease(cameraList);
                
            } else {
                ofLogError() << "No cameras are connected for ofxEds::Camera::setup().";
                Eds::TerminateSDK();
                return false;
            }
        } catch (Eds::Exception& e) {
            ofLogError() << "There was an error during Camera::setup(): " << e.what();
            Eds::TerminateSDK();
            return false;
        }
        
        
        
        for(int i = 0; i < cameras.size(); i++){
            if(cameras[i] != NULL){
                try {
                    Eds::OpenSession(cameras[i]);
                    try {
                        CameraCallBack* fooCallback = new CameraCallBack();
                        fooCallback->camIndex = i;
                        fooCallback->cam = this;
                        
                        Eds::SetObjectEventHandler(cameras[i], kEdsObjectEvent_All, handleObjectEvent, fooCallback);
                        Eds::SetPropertyEventHandler(cameras[i], kEdsPropertyEvent_All, handlePropertyEvent, fooCallback);
                        Eds::SetCameraStateEventHandler(cameras[i], kEdsStateEvent_All, handleCameraStateEvent, fooCallback);
                        
                        EdsDataType dataType;
                        EdsUInt32 dataSize;
                        Eds::GetPropertySize(cameras[i], kEdsPropID_BodyIDEx, 0, &dataType, &dataSize);
                        EdsChar serial[dataSize];
                        Eds::GetPropertyData(cameras[i], kEdsPropID_BodyIDEx, 0, dataSize, &serial);
                        string & fooSerial = mCamSerials[i];
                        fooSerial = serial;
                        string foo = serial;
                        mCamsSerial[serial] = cameras[i];
                        vCamSerials.push_back(foo);
                   
  
                    } catch (Eds::Exception& e) {
                        ofLogError() << "There was an error opening the camera, or starting live view: " << e.what();
                        Eds::TerminateSDK();
                        return false;
                    }
                    
                    connected = true;
                } catch (Eds::Exception& e) {
                    ofLogError() << "There was an error opening the camera, or starting live view: " << e.what();
                    Eds::TerminateSDK();
                    return false;
                }
            }
        }
        
        return connected;
        
    }
    
    void Camera::update() {
        if(connected){
            if(needToSendKeepAlive) {
                try {
                    // always causes EDS_ERR_DEVICE_BUSY, even with live view disabled or a delay
                    // but if it's not here, then the camera shuts down after 5 minutes.
                    for(vector<EdsCameraRef>::iterator iter = cameras.begin(); iter != cameras.end(); ++iter){
                        Eds::SendCommand(*iter, kEdsCameraCommand_ExtendShutDownTimer, 0);
                    }
                } catch (Eds::Exception& e) {
                    ofLogError() << "Error while sending kEdsCameraCommand_ExtendShutDownTimer with Eds::SendStatusCommand: " << e.what();
                }
                
                needToSendKeepAlive = false;
            }
        }
    }
    
    
    bool Camera::isFrameNew() {
        if(frameNew) {
            frameNew = false;
            return true;
        } else {
            return false;
        }
    }
    
    bool Camera::isPhotoNew() {
        if(photoNew) {
            photoNew = false;
            return true;
        } else {
            return false;
        }
    }
    
    float Camera::getFrameRate() {
        float frameRate;
        
        frameRate = fps.getFrameRate();
        
        return frameRate;
    }
    
    void Camera::takePhoto(bool blocking) {
        
        try {
            for(vector<EdsCameraRef>::iterator iter = cameras.begin(); iter != cameras.end(); ++iter){
                Eds::SendCommand(*iter, kEdsCameraCommand_TakePicture, 0);
            }
        } catch (Eds::Exception& e) {
            ofLogError() << "Error while taking a picture: " << e.what();
        }
    }
    
    void Camera::takePhoto(string serial) {
        
        try {
            
            Eds::SendCommand(mCamsSerial[serial], kEdsCameraCommand_TakePicture, 0);
            
        } catch (Eds::Exception& e) {
            ofLogError() << "Error while taking a picture: " << e.what();
        }
    }
    
    ofPixels& Camera::getLivePixels() {
        return livePixels;
    }
    
    ofPixels& Camera::getPhotoPixels() {
        if(needToDecodePhoto) {
            ofLoadImage(photoPixels, photoBuffer);
            needToDecodePhoto = false;
        }
        return photoPixels;
    }
    
    unsigned int Camera::getWidth() const {
        return livePixels.getWidth();
    }
    
    unsigned int Camera::getHeight() const {
        return livePixels.getHeight();
    }
    
    void Camera::draw(float x, float y) {
        draw(x, y, getWidth(), getHeight());
    }
    
    void Camera::draw(float x, float y, float width, float height) {
        //		if(liveDataReady) {
        //			liveTexture.draw(x, y, width, height);
        //		}
    }
    
    ofTexture& Camera::getLiveTexture() {
        return liveTexture;
    }
    
    void Camera::drawPhoto(float x, float y) {
        //		if(photoDataReady) {
        //			ofPixels& photoPixels = getPhotoPixels();
        //			draw(x, y, getWidth(), getHeight());
        //		}
    }
    
    void Camera::drawPhoto(float x, float y, float width, float height) {
        //		if(photoDataReady) {
        //			photoTexture.draw(x, y, width, height);
        //		}
    }
    
    ofTexture& Camera::getPhotoTexture() {
        //		if(photoDataReady) {
        //			ofPixels& photoPixels = getPhotoPixels();
        //			if(needToUpdatePhoto) {
        //				if(photoTexture.getWidth() != photoPixels.getWidth() ||
        //                   photoTexture.getHeight() != photoPixels.getHeight()) {
        //					photoTexture.allocate(photoPixels.getWidth(), photoPixels.getHeight(), GL_RGB8);
        //				}
        //				photoTexture.loadData(photoPixels);
        //				needToUpdatePhoto = false;
        //			}
        //		}
        //		return photoTexture;
    }
    
    bool Camera::isLiveReady() const {
        return liveDataReady;
    }
    
    void Camera::setLiveReady(bool liveReady) {
        this->liveReady = liveReady;
    }
    
    vector<string> Camera::getSerialNumbers(){
        return vCamSerials;
    }
    
    map<int, string> Camera::getSerialMap(){
        return mCamSerials;
    }
    
    int Camera::numCameras(){
        int num;
        num = cameras.size();
        return num;
    }
    
    void Camera::setDownloadImage(EdsDirectoryItemRef directoryItem, string serial) {
        this->directoryItem = directoryItem;
        try {
            ofBuffer buff;
            Eds::DownloadImage(directoryItem, buff);
            ofLogVerbose() << "Downloaded image: " << (int) (buff.size() / 1024) << " KB";
            ofLogVerbose() << "Downloaded from: " << serial<<endl;
            mCurrentSerial = serial;
            needToDownloadImage = false;
            dBuff.push_back(buff);
            mBuffers[mCurrentSerial] = buff;
            if(dBuff.size() == cameras.size()){
                photoDataReady = true;
                photoNew = true;
                needToDecodePhoto = true;
                needToUpdatePhoto = true;
            }
        } catch (Eds::Exception& e) {
            ofLogError() << "Error while downloading image: " << e.what();
        }
    }
    
    void Camera::setSendKeepAlive() {
        needToSendKeepAlive = true;
    }
    
    ofBuffer Camera::getImage(){
        if(dBuff.size() > 0){
            ofBuffer front = dBuff.front();
            dBuff.pop_front();
            return front;
        }else{
            return ofBuffer();
        }
    }
    map<string, ofBuffer> Camera::getImages(){
        dBuff.clear();
        map<string, ofBuffer> buf = mBuffers;
        return buf;
    }
    
    bool Camera::savePhoto(string filename) {
        return ofBufferToFile(filename+"_"+mCurrentSerial+".png", mBuffers[mCurrentSerial], true);
    }
    
    void Camera::resetLiveView() {
        //		lock();
        ////		if(connected) {
        ////			Eds::StartLiveview(camera);
        ////			lastResetTime = ofGetElapsedTimef();
        ////		}
        //		unlock();
    }
}
