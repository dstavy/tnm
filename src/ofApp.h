#pragma once

#include "ofMain.h"
#include "ofxFaceTracker2.h"
#include "ofxBiquadFilter.h"
#include "ofxVideoRecorder.h"
#include "ofxCenteredTrueTypeFont.h"

typedef dlib::matrix<double, 40, 1> sample_type;
typedef dlib::radial_basis_kernel<sample_type> kernel_type;

typedef dlib::decision_function<kernel_type> dec_funct_type;
typedef dlib::normalized_function<dec_funct_type> funct_type;

typedef dlib::probabilistic_decision_function<kernel_type> probabilistic_funct_type;
typedef dlib::normalized_function<probabilistic_funct_type> pfunct_type;


class ofApp : public ofBaseApp {
public:
	void setup();
	void exit();
	void update();
	void draw();
	void keyReleased(int key);
	ofRectangle getBoundingBox(ofRectangle rec1, ofRectangle rec2);
	sample_type makeSample();
	void ofApp::drawSplitScreen(ofFbo& fbo, float angle);
	void ofApp::drawCenteredMessage(string msg, float centerW, float centerH);
	;

	ofxFaceTracker2 tracker;
	ofVideoGrabber grabber;
	ofRectangle boundBoxes[7];
	int rotation;

	ofxBiquadFilter1f neutralValue;
	ofxBiquadFilter1f smallSmileValue;
	ofxBiquadFilter1f bigSmileValue;
	ofxBiquadFilter1f oValue;

	vector<pfunct_type> learned_functions;

	ofxVideoRecorder    vidRecorder;
	ofSoundStream       soundStream;
	bool bRecording;
	int sampleRate;
	int channels;
	string fileName;
	string fileExt;
	int sessionId;
	ofFbo recordFbo;
	ofPixels recordPixels;
	ofVideoPlayer user1Movie;
	ofVideoPlayer user2Movie;
	ofPath frame;
	int lineWidth = 6;
	ofFbo fbo;
	ofxCenteredTrueTypeFont centeredMsg;
	ofColor offWhite = ofColor(0xf9f9f9);
	int fboW = 1080;
	int fboH = 1100;
	float angle = 20.;
};
