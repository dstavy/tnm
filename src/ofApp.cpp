#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetFrameRate(30);
	//ofEnableAlphaBlending();
	fbo.allocate(fboW, fboH, GL_RGBA);
	fbo.begin();
	ofClear(255, 255, 255, 0);
	fbo.end();
	ofSetBackgroundColor(255, 255, 255, 0);
    // All examples share data files from example-data, so setting data path to this folder
    // This is only relevant for the example apps
    //ofSetDataPathRoot(ofFile(__BASE_FILE__).getEnclosingDirectory()+"../../model/");
   // remove smile
	learned_functions = vector<pfunct_type>(4);
    
    // Load SVM data model
    dlib::deserialize(ofToDataPath("data_ecstatic_smile.func")) >> learned_functions[0];
    dlib::deserialize(ofToDataPath("data_small_smile.func")) >> learned_functions[1];
    dlib::deserialize(ofToDataPath("data_o.func")) >> learned_functions[2];
    dlib::deserialize(ofToDataPath("data_neutral.func")) >> learned_functions[3];
    
    // Setup value filters for the classifier
    neutralValue.setFc(0.04);
    bigSmileValue.setFc(0.04);
    smallSmileValue.setFc(0.04);
    oValue.setFc(0.04);
    
    auto devs = grabber.listDevices();
    int width= 1920;
    int height = 1080;
    
    for(auto & d : devs) {
        std::cout << d.id << ": " << d.deviceName << " // " <<
        d.formats.size() << " formats" << std::endl;
        for(auto & f : d.formats) {
            std::cout << "  " << f.width << "x" << f.height << std::endl;
            width = f.width;
            height = f.height;
            for(auto & fps : f.framerates) {
                std::cout << "    " << fps << std::endl;
            }
        }
    }
    // Setup grabber
	grabber.setDeviceID(0);
   //grabber.setPixelFormat(OF_PIXELS_MONO);
    
    // TODO:: set device id
    grabber.setup(width,height);
    grabber.setDesiredFrameRate(30);
	//grabber.setUseTexture(true);
    
    // Setup tracker
    tracker.setup();
    //tracker.setFaceRotation(90);
    
    vidRecorder.setVideoCodec("h264");
    vidRecorder.setVideoBitrate("800k");
    fileName = "testMovie";
    fileExt = ".mp4";

	// Video  
	user1Movie.load("user1.mp4");
	user1Movie.play();
	user2Movie.load("user2.mp4");
	user2Movie.play();

    // ofSetWindowShape(grabber.getWidth(), grabber.getHeight()    );
    bRecording = false;
	frame.setStrokeColor(offWhite);
	frame.setStrokeWidth(lineWidth);
	frame.setFilled(false);

	centeredMsg.loadFont("Arial.ttf", 32);
}

//--------------------------------------------------------------
void ofApp::exit(){
    vidRecorder.close();
	user1Movie.close();
	user2Movie.close();
}

ofRectangle ofApp::getBoundingBox(ofRectangle rec1, ofRectangle rec2) {
    // get min max
    int xmin = min(rec1.getLeft(),rec2.getLeft());
    int ymin= min(rec1.getTop(), rec2.getTop());
    int xmax = max(rec1.getRight(), rec2.getRight());
    int ymax = max(rec1.getBottom(), rec2.getBottom());
    
    return ofRectangle(xmin, ymin, xmax - xmin, ymax - ymin);
}

//--------------------------------------------------------------
void ofApp::update(){
    grabber.update();
    
    // Update tracker when there are new frames
    if(grabber.isFrameNew()){
        tracker.update(grabber);

        if(tracker.size()){
            ofxFaceTracker2Instance camFace = tracker.getInstances()[0];
            ofxFaceTracker2Landmarks landmarks = camFace.getLandmarks();
            ofPolyline eyeL = landmarks.getImageFeature(ofxFaceTracker2Landmarks::LEFT_EYE);
            ofPolyline eyeR = landmarks.getImageFeature(ofxFaceTracker2Landmarks::RIGHT_EYE);
            ofPolyline nose = landmarks.getImageFeature(ofxFaceTracker2Landmarks::NOSE_BASE);
            ofPolyline noseB = landmarks.getImageFeature(ofxFaceTracker2Landmarks::NOSE_BRIDGE);
            ofPolyline mouth = landmarks.getImageFeature(ofxFaceTracker2Landmarks::OUTER_MOUTH);
            ofPolyline face = landmarks.getImageFeature(ofxFaceTracker2Landmarks::FACE_OUTLINE);
            ofPolyline leb = landmarks.getImageFeature(ofxFaceTracker2Landmarks::LEFT_EYEBROW);
            ofPolyline reb = landmarks.getImageFeature(ofxFaceTracker2Landmarks::RIGHT_EYEBROW);
            ofPolyline all = landmarks.getImageFeature(ofxFaceTracker2Landmarks::ALL_FEATURES);
            ofPolyline eyeLTop= landmarks.getImageFeature(ofxFaceTracker2Landmarks::LEFT_EYE_TOP);
            ofPolyline eyeRTop = landmarks.getImageFeature(ofxFaceTracker2Landmarks::RIGHT_EYE_TOP);
            
            eyeL.addVertices(eyeLTop.getVertices());
            boundBoxes[0] = eyeL.getBoundingBox();
            eyeR.addVertices(eyeRTop.getVertices());
            boundBoxes[1] = eyeR.getBoundingBox();
            boundBoxes[2] = mouth.getBoundingBox();
            boundBoxes[3] = leb.getBoundingBox();
            boundBoxes[4] = reb.getBoundingBox();
            nose.addVertices(noseB.getVertices());
            boundBoxes[5] = nose.getBoundingBox();
            //boundBoxes[5] = face.getBoundingBox();
            ofRectangle allRec = face.getBoundingBox();
            allRec.setY(allRec.getTop() - allRec.getHeight()/2);
            allRec.setHeight(allRec.getHeight() * 1.5);
            boundBoxes[6] = allRec;
            
            // Run the classifiers and update the filters
            bigSmileValue.update(learned_functions[0](makeSample()));
            smallSmileValue.update(learned_functions[1](makeSample()));
            oValue.update(learned_functions[2](makeSample()));
            neutralValue.update(learned_functions[3](makeSample()));
        }
        
        if(bRecording){
            bool success = vidRecorder.addFrame(grabber.getPixelsRef());
            if (!success) {
                ofLogWarning("This frame was not added!");
            }
        }
        
        // Check if the video recorder encountered any error while writing video frame or audio smaples.
        if (vidRecorder.hasVideoError()) {
            ofLogWarning("The video recorder failed to write some frames!");
        }
        
        if (vidRecorder.hasAudioError()) {
            ofLogWarning("The video recorder failed to write some audio samples!");
        }
    }

	user1Movie.update();
	user2Movie.update();
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofClear(ofColor::black);
	fbo.begin();
	ofPushMatrix();
	//rotate videos
	ofRotate(90);
	user1Movie.draw(0, -1080, 960, 540);
	frame.rectangle(0, -1080, 960, 540);
	frame.draw();
	user2Movie.draw(0, 540 - 1080, 960, 540);
	frame.rectangle(0, 540 - 1080, 960, 540);
	frame.draw();
	// rotate camera
	grabber.draw(ofRectangle(320, 420- 1080, 320, 240));
	frame.rectangle(320, 420- 1080, 320, 240);
	//frame.draw(); // bug ?? 
	ofPopMatrix();
	drawCenteredMessage("This is a message", 540, 1050);
	fbo.end();
    /*
    // Draw tracker landmarks
    tracker.drawDebug();
    
    // Draw estimated 3d pose
    tracker.drawDebugPose();
    
    ofSetHexColor(0x00ff00);
    ofNoFill();
    
    for (int i = 0; i < 7; i++)
    {
        ofRectangle rec  =  boundBoxes[i];
        ofDrawRectangle (rec);
    }
    // Draw text UI
    ofDrawBitmapStringHighlight("Framerate : "+ofToString(ofGetFrameRate()), 10, 20);
    ofDrawBitmapStringHighlight("Tracker thread framerate : "+ofToString(tracker.getThreadFps()), 10, 40);
   
#ifndef __OPTIMIZE__
    ofSetColor(ofColor::red);
    ofDrawBitmapString("Warning! Run this app in release mode to get proper performance!",10,60);
    ofSetColor(ofColor::white);
#endif
 */
    /*smile
    ofPushMatrix();
    ofTranslate(0, 100);
    ofFill();
    for (int i = 0; i < 4; i++) {
        ofSetColor(255);
        
        string str;
        float val;
        switch (i) {
            case 0:
                str = "BIG SMILE";
                val = bigSmileValue.value();
                break;
            case 1:
                str = "SMALL SMILE";
                val = smallSmileValue.value();
                break;
            case 2:
                str = "OOO MOUTH";
                val = oValue.value();
                break;
            case 3:
                str = "NEUTRAL MOUTH";
                val = neutralValue.value();
                break;
        }
        
        ofDrawBitmapStringHighlight(str, 20, 0);
        ofDrawRectangle(20, 20, 300*val, 30);
        
        ofNoFill();
        ofDrawRectangle(20, 20, 300, 30);
        ofFill();
        
        ofTranslate(0, 70);
    }
    
    ofPopMatrix();
	*/  
	drawSplitScreen(fbo, angle);
}

// Function that creates a sample for the classifier containing the mouth and eyes
sample_type ofApp::makeSample(){
    auto outer = tracker.getInstances()[0].getLandmarks().getImageFeature(ofxFaceTracker2Landmarks::OUTER_MOUTH);
    auto inner = tracker.getInstances()[0].getLandmarks().getImageFeature(ofxFaceTracker2Landmarks::INNER_MOUTH);
    
    auto lEye = tracker.getInstances()[0].getLandmarks().getImageFeature(ofxFaceTracker2Landmarks::LEFT_EYE);
    auto rEye = tracker.getInstances()[0].getLandmarks().getImageFeature(ofxFaceTracker2Landmarks::RIGHT_EYE);
    
    ofVec2f vec = rEye.getCentroid2D() - lEye.getCentroid2D();
    float rot = vec.angle(ofVec2f(1,0));
    
    vector<ofVec2f> relativeMouthPoints;
    
    ofVec2f centroid = outer.getCentroid2D();
    for(ofVec2f p : outer.getVertices()){
        p -= centroid;
        p.rotate(rot);
        p /= vec.length();
        
        relativeMouthPoints.push_back(p);
    }
    
    for(ofVec2f p : inner.getVertices()){
        p -= centroid;
        p.rotate(rot);
        p /= vec.length();
        
        relativeMouthPoints.push_back(p);
    }
    
    sample_type s;
    for(int i=0;i<20;i++){
        s(i*2+0) = relativeMouthPoints[i].x;
        s(i*2+1) = relativeMouthPoints[i].y;
    }
    return s;
}

void ofApp::drawSplitScreen(ofFbo& fbo, float angle) {
	ofPushMatrix();
	//float z = tan(angle)* fboW / 2;
	ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2 - fbo.getWidth() / 2);
	ofRotateYDeg(angle);
	fbo.getTextureReference().drawSubsection(-(fbo.getWidth() / 2), 0, 0, fbo.getWidth() / 2, fbo.getHeight(), 0, 0, fbo.getWidth() / 2, fbo.getHeight());
	ofRotateYDeg(-angle *2);
	fbo.getTextureReference().drawSubsection(0, 0, 0, fbo.getWidth() / 2, fbo.getHeight(), fbo.getWidth() / 2, 0, fbo.getWidth() / 2, fbo.getHeight());
	ofPopMatrix();
}

void ofApp::drawCenteredMessage(string msg, float centerW, float centerH) {
	float padding = 10;
	ofVec2f center(centerW, centerH);
	ofSetColor(offWhite);
	centeredMsg.drawCenteredBoundingBox(msg, center, padding);
	ofSetColor(ofColor::black);
	centeredMsg.drawStringCentered(msg, center);
}

void ofApp::keyReleased(int key){
    if (key == 'y'){
        rotation += 90;
        rotation = rotation % 360;
        tracker.setFaceRotation(rotation);
    } else if (key == 'c'){
		if (tracker.size()) {
			ofPixels crop = grabber.getPixelsRef();
			if (crop.size() > 0) {
				for (int i = 0; i < std::size(boundBoxes); i++)
				{
					ofRectangle rec = boundBoxes[i];
					ofImage img;
					int padding = 20;
					img.allocate(rec.width + padding, rec.height + padding, ofImageType::OF_IMAGE_COLOR);

					crop.cropTo(img.getPixelsRef(), rec.x - padding / 2, rec.y - padding / 2, rec.width + padding, rec.height + padding);
					std::string id = std::to_string(i);
					std::ostringstream oss;
					oss << "img_" << i << ".jpg";
					img.saveImage(oss.str());
					img.clear();
				}
			}
		}
    } else if(key=='r'){
        bRecording = !bRecording;
        if(bRecording && !vidRecorder.isInitialized()) {
			// bug video not playing in right fps with no audio
            //vidRecorder.setup(fileName+ofGetTimestampString()+fileExt, grabber.getWidth(), grabber.getHeight(), 30, sampleRate, channels);
                      vidRecorder.setup(fileName+ std::to_string(sessionId)+fileExt, grabber.getWidth(), grabber.getHeight(), 30, 44100, 1, true, true); // no audio
            //        vidRecorder.setup(fileName+ofGetTimestampString()+fileExt, 0,0,0, sampleRate, channels); // no video
            //        vidRecorder.setupCustomOutput(vidGrabber.getWidth(), vidGrabber.getHeight(), 30, sampleRate, channels, "-vcodec mpeg4 -b 1600k -acodec mp2 -ab 128k -f mpegts udp://localhost:1234"); // for custom ffmpeg output string (streaming, etc)
            
            // Start recording
            vidRecorder.start();
        }
        /*
        else if(!bRecording && vidRecorder.isInitialized()) {
            vidRecorder.setPaused(true);
        }
        else if(bRecording && vidRecorder.isInitialized()) {
            vidRecorder.setPaused(false);
        }
         */
    }
    else if(key=='s'){
        bRecording = false;
        vidRecorder.close();
    }
	else if (key == 'w') {
		user1Movie.close();
		user2Movie.close();
		user1Movie.load("user2.mp4");
		user2Movie.load("user1.mp4");
		user1Movie.play();
		user2Movie.play();
	}
}
