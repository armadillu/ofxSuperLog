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
	draggingWidth = scrolling = false;

	//init some default colors for the log lines
	float gain = 0.99;
	logColors[OF_LOG_VERBOSE] = ofColor(99) * gain;
	logColors[OF_LOG_NOTICE] = ofColor(60,130,240) * gain;
	logColors[OF_LOG_WARNING] = ofColor(250,250,6) * gain;
	logColors[OF_LOG_ERROR] = ofColor(240,0,0) * gain;
	logColors[OF_LOG_FATAL_ERROR] = ofColor(140,32,170) * gain;
	logColors[OF_LOG_SILENT] = ofColor(90) * gain;

	useColors = true;
	scrollV = 0;
	lineH = 20;
	inertia = 0;
	font = NULL;
}

ofxSuperLogDisplay::~ofxSuperLogDisplay() {
}

#ifdef USE_OFX_FONTSTASH
void ofxSuperLogDisplay::setFont(ofxFontStash* f, float fontSize_){
	font = f;
	fontSize = fontSize_;
	lineH = font->getBBox("M", fontSize, 0, 0).height * 1.4; //find line height with "M" char
}
#endif

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

		dragSpeed *= 0.6;

		//clamp scrolling to lines we own
		if(!scrolling){
			float filter = 0.85f;
			float limit = lineH * logLines.size() - ofGetHeight();

			if(scrollV < -limit){
				scrollV = filter * scrollV + -limit * (1.0f - filter);
				inertia *= 0.6;
			}
			if(scrollV > 0){
				scrollV = scrollV * filter;
				inertia *= 0.6;
				//inertia = 0;
			}

			scrollV += inertia;
			inertia *= 0.94;
		}

		float x = ofGetWidth() - width;

		ofRect(x, 0, width, ofGetHeight());

		if(!useColors)ofSetColor(200);
		int pos = 0;

		#ifdef USE_OFX_FONTSTASH
		if(font) font->beginBatch();
		#endif

		deque<LogLine> linesCopy;

		mutex.lock();
		linesCopy = logLines;
		mutex.unlock();

		bool drawn = false;
		for(int i = logLines.size() - 1; i >=0; i--) {
			if(useColors) ofSetColor(logColors[linesCopy[i].level]);
			bool drawLine = true;
			#ifdef USE_OFX_FONTSTASH
			if(font){
				float yy = ofGetHeight() - pos * lineH - scrollV;
				if(yy < 0){
					newestLineOnScreen = i;
					break;
				}
				if(yy > ofGetHeight()) drawLine = false;
				if(font && drawLine){
					if(!drawn){
						oldestLineOnScreen = i;
						drawn = true;
					}
					font->drawBatch(linesCopy[i].line, fontSize, x + 22, yy - 5);
				}
			}else
			#endif
			{
				float yy = ofGetHeight() - pos * lineH - 5 - scrollV;
				if(yy < 0){
					newestLineOnScreen = i;
					break;
				}
				if(yy > ofGetHeight()) drawLine = false;
				if(drawLine){
					if(!drawn){
						oldestLineOnScreen = i;
						drawn = true;
					}
					ofDrawBitmapString(linesCopy[i].line, x + 20, yy);
				}
			}
			pos++;
		}

		#ifdef USE_OFX_FONTSTASH
		if(font)font->endBatch();
		#endif

		ofSetColor(44, 255);
		float xx = ofGetWidth() - width;
		ofRect(xx, 0, 20, ofGetHeight());
		ofSetColor(255);
		float yy = ofGetHeight()/2;
		ofLine(x+8, yy - 10, x+8, yy+10);
		ofLine(x+12, yy - 10, x+12, yy+10);
		ofDrawBitmapString("x", ofGetWidth() - width + 6, ofGetHeight() - 5);
		ofSetColor(0,0,0);
		float y1 = ofMap(oldestLineOnScreen, 0, logLines.size(), 0, ofGetHeight());
		float y2 = ofMap(newestLineOnScreen, 0, logLines.size(), 0, ofGetHeight());
		ofSetColor(255,64);
		ofRect(xx + 5, y1, 10, y2 - y1);
	}
	ofPopStyle();
}

void ofxSuperLogDisplay::mousePressed(ofMouseEventArgs &e) {
	if(!minimized && ABS(e.x - (ofGetWidth() - width))<20) {
		draggingWidth = true;
		mouseDragged(e);
	}
	if(!minimized && (e.x > (ofGetWidth() - width))) {
		scrolling = true;
		prevY = e.y;
		inertia = 0;
		dragSpeed = 0;
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

	if(scrolling && !draggingWidth){
		dragSpeed = (e.y - prevY) * lineH * 0.25;
		inertia = -dragSpeed;
		scrollV -= dragSpeed;
		prevY = e.y;
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
		if(scrolling){
			inertia = -dragSpeed;
		}
	}
	draggingWidth = false;
	scrolling = false;
}

void ofxSuperLogDisplay::setMinimized(bool minimized) {
	this->minimized  = minimized;
	inertia = 0;
	scrolling = false;
	scrollV = 0;
}

bool ofxSuperLogDisplay::isMinimized() {
	return minimized;
}
