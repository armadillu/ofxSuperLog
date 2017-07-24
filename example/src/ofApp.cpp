#include "ofApp.h"
#include "ofxSuperLog.h"

//--------------------------------------------------------------
void ofApp::setup(){
	
	string fileLoggingDirectory = "logs";
	bool logToConsole = true;
	bool logToScreen = true;
	
	ofSetLoggerChannel(ofxSuperLog::getLogger(logToConsole, logToScreen, "logs"));

	ofLogNotice() << "This is logging to your screen and ";
	ofLogNotice() << "to a file with a timestamped file ";
	ofLogNotice() << "in data/logs, and to your console.";
	ofLogNotice() << "";
	ofLogNotice() << "you can choose where to log to when";
	ofLogNotice() << "you set the logger channel.";
	ofLogNotice() << "";
	ofLogNotice() << "You can also minimize the log window ";
	ofLogNotice() << "by pressing the little x, or make it";
	ofLogNotice() << "wider by dragging the left bar";
	ofxSuperLog::getLogger()->setMaximized(true);
	
	ofBackground(20, 20, 20);
	ofSetFrameRate(60);
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
	ofNoFill();
	ofSetColor(50);
	// some pointless graphics
	float angle = ofGetElapsedTimeMillis()%8000;
	angle = 360 * angle / 8000.f;
	glPushMatrix();
	glTranslatef(ofGetWidth()/2, ofGetHeight()/2, 0);
	glRotatef(angle, 0, 0, 1);
	for(int j = 0; j < 360; j+=20) {
		glPushMatrix();
		glRotatef(j, 0, 0, 1);
		ofBeginShape();
		for(float i = 0; i <= 1; i+=0.01) {
			ofVertex(i*400, sin(ofGetElapsedTimef()+i)*20*i*100.f);
		}
		ofEndShape(false);
		glPopMatrix();
	}
//	for(float f = 0; f < PI*2; f *= PI*0.1) {

	glPopMatrix();
	ofFill();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
	
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
	ofLogNotice() << "Mouse dragged: " << x << ", " << y;
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