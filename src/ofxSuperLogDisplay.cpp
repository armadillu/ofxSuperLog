/**
 *  ofxSuperLogDisplayLoggerChannel.cpp
 *
 *  Created by Marek Bereza on 02/09/2013.
 */

#include "ofxSuperLogDisplay.h"

ofxSuperLogDisplay::ofxSuperLogDisplay() {
	enabled = false;
	minimized = true;

	MAX_NUM_LOG_LINES = DEFAULT_NUM_LOG_LINES;
	width = ofGetWidth() * 0.5f;
	draggingWidth = false;

	//init some default colors for the log lines
	float gain = 0.99;
	logColors[OF_LOG_VERBOSE] = ofColor(99) * gain;
	logColors[OF_LOG_NOTICE] = ofColor(60,130,240) * gain;
	logColors[OF_LOG_WARNING] = ofColor(250,250,6) * gain;
	logColors[OF_LOG_ERROR] = ofColor(240,0,0) * gain;
	logColors[OF_LOG_FATAL_ERROR] = ofColor(140,32,170) * gain;
	logColors[OF_LOG_SILENT] = ofColor(90) * gain;

	useColors = true;
}

ofxSuperLogDisplay::~ofxSuperLogDisplay() {
}

void ofxSuperLogDisplay::setMaxNumLogLines(int maxNumLogLines) {
	MAX_NUM_LOG_LINES = maxNumLogLines;
}

void draw(ofEventArgs &e);

void mousePressed(ofMouseEventArgs &e);
void mouseMoved(ofMouseEventArgs &e);
void mouseDragged(ofMouseEventArgs &e);
void mouseReleased(ofMouseEventArgs &e);


void ofxSuperLogDisplay::setEnabled(bool enabled) {
	if(enabled==this->enabled) return;
	this->enabled = enabled;
	if(enabled) {
		ofAddListener(ofEvents().mousePressed, this, &ofxSuperLogDisplay::mousePressed);
		ofAddListener(ofEvents().mouseMoved, this, &ofxSuperLogDisplay::mouseMoved);
		ofAddListener(ofEvents().mouseDragged, this, &ofxSuperLogDisplay::mouseDragged);
		ofAddListener(ofEvents().mouseReleased, this, &ofxSuperLogDisplay::mouseReleased);
		ofAddListener(ofEvents().draw, this, &ofxSuperLogDisplay::draw);
	} else {
		ofRemoveListener(ofEvents().mousePressed, this, &ofxSuperLogDisplay::mousePressed);
		ofRemoveListener(ofEvents().mouseMoved, this, &ofxSuperLogDisplay::mouseMoved);
		ofRemoveListener(ofEvents().mouseDragged, this, &ofxSuperLogDisplay::mouseDragged);
		ofRemoveListener(ofEvents().mouseReleased, this, &ofxSuperLogDisplay::mouseReleased);
		ofRemoveListener(ofEvents().draw, this, &ofxSuperLogDisplay::draw);
	}
}

bool ofxSuperLogDisplay::isEnabled() {
	return enabled;
}

void ofxSuperLogDisplay::log(ofLogLevel level, const string & module, const string & message) {

	mutex.lock();
	if(message.find('\n')==-1) {

		logLines.push_back(LogLine(string(module + ": " + ofGetLogLevelName(level) + ": " + message),
								   level)
						   );
	} else {
		vector<string> lines = ofSplitString(message,"\n");
		for(int i = 0; i < lines.size(); i++) {
			if(i==0) {
				logLines.push_back(LogLine(module + ": " + ofGetLogLevelName(level) + ": " + lines[0], level));
			} else {
				logLines.push_back(LogLine("\t" + lines[i], level));
			}
		}
	}
	while(logLines.size()>MAX_NUM_LOG_LINES) {
		logLines.pop_front();
	}
	mutex.unlock();
}


void ofxSuperLogDisplay::log(ofLogLevel logLevel, const string & module, const char* format, ...) {
	va_list args;
	va_start(args, format);
	log(logLevel, module, format, args);
	va_end(args);
}

void ofxSuperLogDisplay::log(ofLogLevel logLevel, const string & module, const char* format, va_list args) {
	log(logLevel, module, ofVAArgsToString(format,args));
}



void ofxSuperLogDisplay::draw(ofEventArgs &e) {

	ofPushStyle();
	ofEnableAlphaBlending();
	ofSetColor(0, 240);

	if(minimized) {
		minimizedRect.set(ofGetWidth() -150, ofGetHeight() - 20, 150, 20);
		ofRect(minimizedRect);
		ofSetColor(255);
		ofDrawBitmapString("+ [ Log ] ", minimizedRect.x + 10, minimizedRect.getBottom() - 4);
	} else {
		float x = ofGetWidth() - width;

		ofRect(x, 0, width, ofGetHeight());

		if(!useColors)ofSetColor(200);
		int pos = 0;
		#ifdef USE_OFX_FONTSTASH
		int lineH = 16;
		if(font){
			lineH = font->getBBox("M", fontSize, 0, 0).height; //find line height with "M" char
		}
		font->beginBatch();
		#endif

		deque<LogLine> linesCopy;
		mutex.lock();
		linesCopy = logLines;
		mutex.unlock();

		for(int i = logLines.size() - 1; i >=0; i--) {
			if(useColors) ofSetColor(logColors[linesCopy[i].level]);
			#ifdef USE_OFX_FONTSTASH
			float yy = ofGetHeight() - pos * lineH * 1.33;
			if(yy<0) break;
			if(font) font->drawBatch(linesCopy[i].line, fontSize, x + 22, yy - 5);
			#else
			float yy = ofGetHeight() - pos * 20 - 5;
			if(yy<0) break;
			ofDrawBitmapString(linesCopy[i].line, x + 20, yy);
			#endif
			pos++;
		}
		mutex.unlock();

		#ifdef USE_OFX_FONTSTASH
		font->endBatch();
		#endif
		ofSetColor(44, 255);
		ofRect(ofGetWidth() - width, 0, 20, ofGetHeight());
		ofSetColor(255);
		float yy = ofGetHeight()/2;
		ofLine(x+8, yy - 10, x+8, yy+10);
		ofLine(x+12, yy - 10, x+12, yy+10);
		ofDrawBitmapString("x", ofGetWidth() - width + 5, ofGetHeight() - 5);
	}
	ofPopStyle();
}

void ofxSuperLogDisplay::mousePressed(ofMouseEventArgs &e) {
	if(!minimized && ABS(e.x - (ofGetWidth() - width))<20) {
		draggingWidth = true;
		mouseDragged(e);
	}
}
void ofxSuperLogDisplay::mouseMoved(ofMouseEventArgs &e) {
}
void ofxSuperLogDisplay::mouseDragged(ofMouseEventArgs &e) {
	if(draggingWidth) {
		width = 10 + ofGetWidth() - e.x;
		width = MAX(10, width);
		width = MIN(ofGetWidth() - 10, width);
	}
}
	   
	   
void ofxSuperLogDisplay::mouseReleased(ofMouseEventArgs &e) {
	if(minimized) {
		if(minimizedRect.inside(e.x, e.y)) {
			minimized = false;
		}
	} else {
		if(e.y>ofGetHeight() - 20) {
			if(ABS(e.x - (ofGetWidth() - width))<20) {
				minimized = true;
			}
		}
	}
	draggingWidth = false;
}

void ofxSuperLogDisplay::setMinimized(bool minimized) {
	this->minimized  = minimized;
}

bool ofxSuperLogDisplay::isMinimized() {
	return minimized;
}
