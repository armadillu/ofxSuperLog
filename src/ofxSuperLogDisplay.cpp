/**
 *  ofxSuperLogDisplayLoggerChannel.cpp
 *
 *  Created by Marek Bereza on 02/09/2013.
 */

#include "ofxSuperLogDisplay.h"

ofxSuperLogDisplay::ofxSuperLogDisplay() {
	enabled = false;
	minimized = true;
	autoDraw = true;

	MAX_NUM_LOG_LINES = DEFAULT_NUM_LOG_LINES;
	lastW = 1024;
	lastH = 768;

	widthPct = 0.85f;
	draggingWidth = scrolling = false;

	//init some default colors for the log lines
	float gain = 0.99;
	logColors[OF_LOG_VERBOSE] = ofColor(128) * gain;
	logColors[OF_LOG_NOTICE] = ofColor(60,130,240) * gain;
	logColors[OF_LOG_WARNING] = ofColor(250,250,6) * gain;
	logColors[OF_LOG_ERROR] = ofColor(240,0,0) * gain;
	logColors[OF_LOG_FATAL_ERROR] = ofColor(250,0,222) * gain;
	logColors[OF_LOG_SILENT] = ofColor(90) * gain;

	useColors = true;
	scrollY = 0;
	lineH = 15;
	#ifdef USE_OFX_FONTSTASH
	font = NULL;
	#endif
	logLines.push_back(LogLine("", "################## ofxSuperLog Start ##################", OF_LOG_WARNING));
	
	ofAddListener(ofEvents().keyPressed, this, &ofxSuperLogDisplay::onKeyPressed);
}

ofxSuperLogDisplay::~ofxSuperLogDisplay() {}


void ofxSuperLogDisplay::onKeyPressed(ofKeyEventArgs & k){
	#ifdef USE_OFX_FONTSTASH
	if(k.key == '+' || k.key == '-'){
		if(!isMinimized() && font){
			setFont(font, fontSize + float(k.key == '+' ? 1.0 : -1.0));
		}
	}
	#endif
	if (k.key == OF_KEY_DOWN || k.key == OF_KEY_UP) {
		targetScrollY += k.key == OF_KEY_DOWN ? 2 * lineH : -2 * lineH;
		targetScrollY = ofClamp(targetScrollY, -maxScrollY, 0);
	}
	if (k.key == OF_KEY_END) {
		targetScrollY = 0;
	}
	if (k.key == OF_KEY_HOME) {
		targetScrollY = -maxScrollY;
	}
	if (k.key == OF_KEY_PAGE_DOWN) {
		targetScrollY = ofClamp(targetScrollY + 100 * lineH, -maxScrollY, 0);
	}
	if (k.key == OF_KEY_PAGE_UP) {
		targetScrollY = ofClamp(targetScrollY - 100 * lineH, -maxScrollY, 0);
	}
}


#ifdef USE_OFX_FONTSTASH
void ofxSuperLogDisplay::setFont(ofxFontStash* f, float fontSize_){
	font = f;
	fontSize = fontSize_;
	ofRectangle r = font->getBBox("M", fontSize, 0, 0);
	lineH = r.height * 1.4; //find line height with "M" char
	if(lineH <= 0){
		ofLogError("ofxSuperLogDisplay") << "FontStash lineH reported 0!";
		lineH = 20;
	}
	charW = r.width;
}
#endif

void ofxSuperLogDisplay::setMaxNumLogLines(int maxNumLogLines) {
	MAX_NUM_LOG_LINES = maxNumLogLines;
}

void ofxSuperLogDisplay::setEnabled(bool enabled) {
	if(enabled==this->enabled) return;
	this->enabled = enabled;
	if(enabled) {
		ofAddListener(ofEvents().mousePressed, this, &ofxSuperLogDisplay::mousePressed);
		ofAddListener(ofEvents().mouseDragged, this, &ofxSuperLogDisplay::mouseDragged);
		ofAddListener(ofEvents().mouseReleased, this, &ofxSuperLogDisplay::mouseReleased);
		if(autoDraw){
			ofAddListener(ofEvents().draw, this, &ofxSuperLogDisplay::draw, OF_EVENT_ORDER_AFTER_APP + 10);
		}
	} else {
		ofRemoveListener(ofEvents().mousePressed, this, &ofxSuperLogDisplay::mousePressed);
		ofRemoveListener(ofEvents().mouseDragged, this, &ofxSuperLogDisplay::mouseDragged);
		ofRemoveListener(ofEvents().mouseReleased, this, &ofxSuperLogDisplay::mouseReleased);
		if(autoDraw){
			ofRemoveListener(ofEvents().draw, this, &ofxSuperLogDisplay::draw, OF_EVENT_ORDER_AFTER_APP + 10);
		}
	}
}

bool ofxSuperLogDisplay::isEnabled() {
	return enabled;
}

void ofxSuperLogDisplay::log(ofLogLevel level, const string & module, const string & message) {

	if(module.size() > maxModuleLen){
		maxModuleLen = module.size();
	}
	mutex.lock();
	if(message.find('\n') == -1) {
		logLines.push_back(LogLine(module, message, level));
	} else {
		vector<string> lines = ofSplitString(message,"\n");
		string emptyModName;
		//emptyModName.append(maxModuleLen, ' ');

		for(int i = 0; i < lines.size(); i++) {
			if(i==0) {
				logLines.push_back(LogLine(module, lines[0], level));
			} else {
				logLines.push_back(LogLine(emptyModName, lines[i], level));
			}
		}
	}
	while(logLines.size() > MAX_NUM_LOG_LINES) {
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
	draw(ofGetWidth(), ofGetHeight());
}

void ofxSuperLogDisplay::draw(float screenW, float screenH) {

	lastW = screenW;
	lastH = screenH;

	deque<LogLine> linesCopy;

	mutex.lock();
	linesCopy = logLines;
	mutex.unlock();

	if(linesCopy.size() == 0) return;

	ofPushStyle();
	ofEnableAlphaBlending();
	ofSetColor(0, 240);

	if(minimized) {
		minimizedRect.set(screenW - 150, screenH - 20, 150, 20);
		ofDrawRectangle(minimizedRect);
		ofSetColor(255);
		ofDrawBitmapString("+ [ Log ] ", minimizedRect.x + 10, minimizedRect.getBottom() - 4);
	} else {

		dragSpeed *= 0.6;

		//clamp scrolling to lines we own
		maxScrollY = lineH * linesCopy.size() - screenH;
		if(!scrolling){
			float filter = 0.85f;
			if(targetScrollY < -maxScrollY){
				targetScrollY = filter * targetScrollY + -maxScrollY * (1.0f - filter);
				inertia *= 0.6;
			}
			if(targetScrollY > 0){
				targetScrollY = targetScrollY * filter;
				inertia *= 0.6;
			}

			targetScrollY += inertia;
			inertia *= 0.97;
		}

		scrollY = ofLerp(scrollY, targetScrollY, 0.33);

		int x = screenW * (1. - widthPct);

		ofDrawRectangle(x, 0, ceil(1 + screenW * widthPct), screenH);

		if(!useColors) ofSetColor(200);
		int pos = 0;

		#ifdef USE_OFX_FONTSTASH
		if(font) font->beginBatch();
		#endif

		float yy;
		bool drawn = false;
		int postModuleX = maxModuleLen * charW; //
		const string separator = " | ";

		for(int i = linesCopy.size() - 1; i >= 0; i--) {
			#ifdef USE_OFX_FONTSTASH
			if(font){
				yy = screenH - pos * lineH - scrollY;
				if(yy < 0){
					newestLineOnScreen = i;
					break;
				}
				if(yy < screenH + 20){
					if(!drawn){
						oldestLineOnScreen = i;
						drawn = true;
					}
					if(linesCopy[i].module.size()){
						if(useColors) ofSetColor(getColorForModule(linesCopy[i].moduleClean));
						int off = charW * (maxModuleLen - linesCopy[i].module.size());
						font->drawBatch(linesCopy[i].module, fontSize, x + off + 22, yy - 5);
					}
					if(useColors) ofSetColor(logColors[linesCopy[i].level]);
					font->drawBatch(separator + linesCopy[i].line, fontSize, x + 16 + postModuleX, yy - 5);
				}
			}else
			#endif
			{
				yy = screenH - pos * lineH - 5 - scrollY;
				if(yy < 0){
					newestLineOnScreen = i;
					break;
				}
				if(yy < screenH + 20 ){
					if(!drawn){
						oldestLineOnScreen = i;
						drawn = true;
					}
					if(linesCopy[i].module.size()){
						if(useColors) ofSetColor(getColorForModule(linesCopy[i].moduleClean));
						int off = charW * (maxModuleLen - linesCopy[i].module.size());
						ofDrawBitmapString(linesCopy[i].module, x + off + 20, yy );
					}
					if(useColors) ofSetColor(logColors[linesCopy[i].level]);
					ofDrawBitmapString(separator + linesCopy[i].line, x + 20 + postModuleX, yy);
				}
			}
			pos++;
		}

		#ifdef USE_OFX_FONTSTASH
		if(font)font->endBatch();
		#endif

		ofSetColor(44, 255);
		ofDrawRectangle(x, 0, 20, screenH);
		ofSetColor(255);
		yy = ofGetHeight()/2;
		ofDrawLine(x + 8, yy - 10, x+8, yy+10);
		ofDrawLine(x+12, yy - 10, x+12, yy+10);
		ofDrawBitmapString("x", screenW - screenW * widthPct + 6, screenH - 5);
		ofSetColor(0,0,0);
		float y1 = ofMap(oldestLineOnScreen, 0, linesCopy.size(), 0, screenH);
		float y2 = ofMap(newestLineOnScreen, 0, linesCopy.size(), 0, screenH);
		ofSetColor(255,64);
		ofDrawRectangle(x + 5, y1, 10, y2 - y1);
	}
	ofPopStyle();
}


const ofColor& ofxSuperLogDisplay::getColorForModule(const string & modName){
	auto search = moduleColors.find(modName);
	if(search == moduleColors.end()){
		ofColor c; c.setHsb(( 30 + moduleColors.size() * 36)%255, 255, 255);
		moduleColors[modName] = c;
		return moduleColors[modName];
	}
	return search->second;
}


void ofxSuperLogDisplay::mousePressed(ofMouseEventArgs &e) {
	if(!minimized && ABS(e.x - (lastW * (1.0f - widthPct)))<20) {
		draggingWidth = true;
		mouseDragged(e);
	}
	if(!minimized && (e.x > (lastW * (1.0f - widthPct)))) {
		scrolling = true;
		prevY = e.y;
		inertia = 0;
		dragSpeed = 0;
	}
}


void ofxSuperLogDisplay::mouseDragged(ofMouseEventArgs &e) {
	int width;
	if(draggingWidth) {
		width = 10 + lastW - e.x;
		width = MAX(10, width);
		width = MIN(ofGetWidth() - 10, width);
		widthPct = width / lastW;
	}

	if(scrolling && !draggingWidth){
		dragSpeed = (e.y - prevY) * lineH * 0.25;
		inertia = -dragSpeed;
		targetScrollY -= dragSpeed;
		prevY = e.y;
	}
}
	   
	   
void ofxSuperLogDisplay::mouseReleased(ofMouseEventArgs &e) {
	if(minimized) {
		if(minimizedRect.inside(e.x, e.y)) {
			minimized = false;
		}
	} else {
		if(e.y > lastH - 20) {
			if(ABS(e.x - (lastW * (1.0f - widthPct))) < 20) {
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
	scrollY = targetScrollY = 0;
}

bool ofxSuperLogDisplay::isMinimized() {
	return minimized;
}
