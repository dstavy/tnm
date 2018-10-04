#include "ofApp.h"

int previewWidth = 1920;
int previewHeight = 1080;

//--------------------------------------------------------------
void ofApp::setup(){
	kinect.open();
	kinect.initDepthSource();
	kinect.initColorSource();
	kinect.initInfraredSource();
	//kinect.initFaceSource();
	kinect.initBodySource();
	kinect.initBodyIndexSource();
	kinect.initFaceSource();

	ofSetWindowShape(previewWidth * 2, previewHeight * 2);
}

//--------------------------------------------------------------
void ofApp::update(){
	kinect.update();

	//--
	//Getting joint positions (skeleton tracking)
	//--
	//
	{
		auto bodies = kinect.getBodySource()->getBodies();
		for (auto body : bodies) {
			for (auto joint : body.joints) {
				//now do something with the joints
			}
		}
	}
	//
	//--



	//--
	//Getting bones (connected joints)
	//--
	//
	{
		// Note that for this we need a reference of which joints are connected to each other.
		// We call this the 'boneAtlas', and you can ask for a reference to this atlas whenever you like
		auto bodies = kinect.getBodySource()->getBodies();
		auto boneAtlas = ofxKinectForWindows2::Data::Body::getBonesAtlas();

		for (auto body : bodies) {
			for (auto bone : boneAtlas) {
				auto firstJointInBone = body.joints[bone.first];
				auto secondJointInBone = body.joints[bone.second];

				//now do something with the joints
			}
		}
	}
	//
	//--
}

//--------------------------------------------------------------
void ofApp::draw(){
	//kinect.getDepthSource()->draw(0, 0, previewWidth, previewHeight);  // note that the depth texture is RAW so may appear dark
	
	// Color is at 1920x1080 instead of 512x424 so we should fix aspect ratio
	float colorHeight = previewWidth * (kinect.getColorSource()->getHeight() / kinect.getColorSource()->getWidth());
	float colorTop = (previewHeight - colorHeight);

	kinect.getColorSource()->draw(0, 0 , previewWidth, previewHeight);
	kinect.getBodySource()->drawProjected(0, 0 , previewWidth, previewHeight);
	kinect.getFaceSource()->drawProjected(0, 0, previewWidth, previewHeight);
	
	//kinect.getInfraredSource()->draw(0, previewHeight, previewWidth, previewHeight);
	
	//kinect.getBodyIndexSource()->draw(previewWidth, previewHeight, previewWidth, previewHeight);
	//kinect.getBodySource()->drawProjected(previewWidth, previewHeight, previewWidth, previewHeight, ofxKFW2::ProjectionCoordinates::DepthCamera);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if (key == 'x') {
		ofImage  img;
		img.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
		img.save("screenshot.png");
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){

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

#ifdef TARGET_WIN32  

static WNDPROC currentWndProc;
static HWND handle = NULL;

static bool CALLBACK EnumWindowsProc(HWND hWnd, LPARAM processID) {

	//get the process id for each window  
	DWORD tempID = 0;
	GetWindowThreadProcessId(hWnd, &tempID);

	if (tempID == (DWORD)processID) {
		//three windows are associated with our id  
		//a console window, the glut window and an invisible gl context  
		//we get the name of each window to determin which is the one we want  
		wchar_t name[256];
		GetClassName(hWnd, name, 255);

		//if you want to see the process names  
		//wprintf(L"class name is %s \n", name);   

		//we check the process name to see if it GLUT  
		//if so we store the handle and then stop the search  
		if (wcsncmp(name, L"GLUT", 4) == 0) {
			handle = hWnd;
			return false;
		}

	}
	return true;
}

static LRESULT CALLBACK winProc(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam) {

	//we catch close and destroy messages  
	//and send them to OF  
	switch (Msg) {
	case WM_CLOSE:
		OF_EXIT_APP(0);
		break;
	case WM_DESTROY:
		OF_EXIT_APP(0);
		break;
	default:
		//all other messages our handled by glut  
		return currentWndProc(hwnd, Msg, wParam, lParam);
		break;
	}

	return 0;
}

//--------------------------------------  
static void fixCloseWindow() {

	//we get our current process id  
	DWORD ourProcessID = GetCurrentProcessId();

	//we run through all windows and look for windows matching  
	//our process id  
	EnumWindows((WNDENUMPROC)EnumWindowsProc, ourProcessID);

	//a fallback incase no window handle is found  
	//we use the top most.  
	if (handle == NULL) {
		handle = WindowFromDC(wglGetCurrentDC());
	}

	//store the current message event handler for the window  
	currentWndProc = (WNDPROC)::GetWindowLong(handle, GWL_WNDPROC);

	//tell the window to now use our event handler!  
	SetWindowLong(handle, GWL_WNDPROC, (long)winProc);
}

#endif  

