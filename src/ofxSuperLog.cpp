/**
 *  ofxSuperLog.cpp
 *
 *  Created by Marek Bereza on 02/09/2013.
 */

#include "ofxSuperLog.h"
#include "Poco/File.h"

ofPtr<ofxSuperLog> ofxSuperLog::logger;
ofxSuperLog *ofxSuperLog::logPtr = NULL;

ofPtr<ofxSuperLog> &ofxSuperLog::getLogger(bool writeToConsole, bool drawToScreen, string logDirectory) {
	if(logPtr==NULL) {
		logPtr = new ofxSuperLog(writeToConsole, drawToScreen, logDirectory);
		logger = ofPtr<ofxSuperLog>(logPtr);

	}
	return logger;
}


ofxSuperLog::ofxSuperLog(bool writeToConsole, bool drawToScreen, string logDirectory) {

	this->loggingToFile = logDirectory!="";
	this->loggingToScreen = drawToScreen;
	this->loggingToConsole = writeToConsole;
	this->logDirectory = logDirectory;
	if(loggingToFile) {
		if(!ofFile(logDirectory).exists()) {
			ofDirectory dir(logDirectory);
			dir.create();
		}

		fileLogger.setFile(logDirectory + "/" + ofGetTimestampString() + ".log", true);
	}
	if(drawToScreen) {
		displayLogger.setEnabled(true);
	}
}

ofxSuperLog::~ofxSuperLog() {
}


void ofxSuperLog::clearOldLogs(string path, int numDays){

	string ppath = ofFilePath::getPathForDirectory(path);
	if(ppath.empty()){
		return;
	}

	string originalDirectory = ppath;
	Poco::File myDir = Poco::File::File(ofToDataPath(ppath));

	if(myDir.exists()){
		vector<Poco::File>files;
		myDir.list(files);
		for(int i=0;i<(int)files.size();i++){
			Poco::Timestamp mod = files[i].getLastModified();
			Poco::Timestamp now;
			Poco::Timestamp::TimeDiff diffInMicroSecs = now - mod;
			int secondsOld = diffInMicroSecs / 1000000;
			int minutesOld = secondsOld/60;
			int hoursOld = minutesOld/60;
			if(hoursOld > (24 * numDays)){
				files[i].remove(false);
				ofLogNotice("ofxSuperLog") << "removing old log at \"" << files[i].path() << "\" bc it's more than " << numDays << " days old.";
			}
		}
	}else{
		myDir.createDirectory();
	}
}

void ofxSuperLog::archiveOldLogs(int numUncompressedToKeep, int numCompressedToKeep) {
    if(numUncompressedToKeep==-1) return;
    if(logDirectory=="") {
        ofLogError() << "Must specify a log directory - can't be \"\"";
        return;
    }
    ofDirectory dir;
    dir.allowExt("log");
    dir.listDir(logDirectory);
    dir.sort();
   // while(numUncompressedToKeep<dir.size())
   ofLogError() << "This isn't implemented yet!";

   /*
   // MUST use binary!
std::ofstream out("test.zip", std::ios::binary);
Compress c(out, true);
Poco::Path aFile("c:\\data\\hello.txt");
c.addFile(theFile, "hello.txt");
c.close(); // MUST be done to finalize the Zip file

*/
}


void ofxSuperLog::log(ofLogLevel level, const string & module, const string & message) {

	if(loggingToFile) fileLogger.log(level, module, message);
	if(loggingToConsole) consoleLogger.log(level, module, message);
	if(loggingToScreen) displayLogger.log(level, module, message);

}

void ofxSuperLog::log(ofLogLevel logLevel, const string & module, const char* format, ...) {
	va_list args;
	va_start(args, format);
	log(logLevel, module, format, args);
	va_end(args);
}

#ifdef USE_OFX_FONTSTASH
void ofxSuperLog::setFont(ofxFontStash * font, float fontSiz){
	displayLogger.setFont(font, fontSiz);
}
#endif



char aux64_2[6000];//hopefully that's enough!
char aux64[6000];

void ofxSuperLog::log(ofLogLevel logLevel, const string & module, const char* format, va_list args) {

	//we can only use args once! bc we have different outputs,
	//we pre-process args into a string, then we sedn all string to all channels
	vsprintf(aux64_2, format, args);
	if(module != ""){
		sprintf(aux64, "[%s] %s %s", ofGetLogLevelName(logLevel, true).c_str(), module.c_str(), aux64_2);
	}else{
		sprintf(aux64, "[%s] %s", ofGetLogLevelName(logLevel, true).c_str(), aux64_2);
	}

	if(loggingToFile) fileLogger.log(logLevel, module, aux64);
	if(loggingToConsole) consoleLogger.log(logLevel, module, aux64);
	if(loggingToScreen) displayLogger.log(logLevel, module, aux64);
}

void ofxSuperLog::draw(float w, float h){
	displayLogger.draw(w, h);
}

void ofxSuperLog::setMaxNumLogLines(int maxNumLogLines) {
	displayLogger.setMaxNumLogLines(maxNumLogLines);
}

bool ofxSuperLog::isScreenLoggingEnabled() {
	return displayLogger.isEnabled();
}
void ofxSuperLog::setScreenLoggingEnabled(bool enabled) {
	displayLogger.setEnabled(enabled);
}


void ofxSuperLog::setMaximized(bool maximized){
	displayLogger.setMinimized(!maximized);
}
