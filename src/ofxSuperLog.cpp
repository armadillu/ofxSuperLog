/**
 *  ofxSuperLog.cpp
 *
 *  Created by Marek Bereza on 02/09/2013.
 */

#include "ofxSuperLog.h"
#ifdef TARGET_WIN32
#include <VersionHelpers.h>
#endif

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

	#ifdef TARGET_WIN32

	colorTerm = true;  IsWindowsVersionOrGreater(10, 0, 0);
	if (colorTerm) {
		HANDLE hStdout;
		DWORD handleMode;
		hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
		#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
		#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
		#endif
		handleMode = ENABLE_VIRTUAL_TERMINAL_PROCESSING | ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT;
		SetConsoleMode(hStdout, handleMode);
	}
	#else
	if(const char* env_p = std::getenv("TERM")){ //see if term supports color!
		ofLogNotice("ofxSuperLog") << "Your $TERM is: '" << env_p << "'";
		if(ofIsStringInString(string(env_p), "xterm")){
			colorTerm = true;
		}
	}
	#endif

	if(colorTerm)ofLogNotice("ofxSuperLog") << "Enabling colored console output";

	this->loggingToFile = logDirectory!="";
	this->loggingToScreen = drawToScreen;
	this->loggingToConsole = writeToConsole;
	this->logDirectory = logDirectory;
	if(loggingToFile) {
		if(!ofFile(logDirectory).exists()) {
			ofDirectory dir(logDirectory);
			dir.create();
		}

		fileLogger.setFile(logDirectory + "/" + ofGetTimestampString("%Y-%m-%d | %H-%M-%S | %A") + ".log", true);
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
		ofLogError("ofxSuperLog") << "can't clearOldLogs; supplied path not found: " << path;
		return;
	}

	ofDirectory dir;
	dir.listDir(ppath);

	if(!dir.exists()){
		dir.create();
	}else{
		std::time_t now = std::time( nullptr ) ;

		for(int i = 0; i < dir.size() ; i++){
			string logFilePath = dir.getPath(i);
			string logFileAbsolutePath = ofToDataPath(logFilePath, true);
			std::time_t lastMod = std::filesystem::last_write_time(logFileAbsolutePath);

			int secondsOld = std::difftime(now, lastMod);
			int daysOld = secondsOld / (60 * 60 * 24);
			if(daysOld > numDays){
				ofLogNotice("ofxSuperLog") << "removing old log at \"" << logFilePath << "\" bc it's more than " << numDays << " days old.";
				bool didRemove = ofFile::removeFile(logFileAbsolutePath, false);
				if(!didRemove){
					ofLogError("ofxSuperLog") << "couldn't delete log at " << logFileAbsolutePath;
				}
			}
		}
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

#ifdef USE_OFX_FONTSTASH
void ofxSuperLog::setFont(ofxFontStash * font, float fontSiz){
	displayLogger.setFont(font, fontSiz);
}
#endif

string ofxSuperLog::filterModuleName(const string & module){

	if(module.size() > maxModuleLen) maxModuleLen = module.size();
	string pad; pad.append(maxModuleLen - module.size(), ' ');
	return pad + module;
}

void ofxSuperLog::log(ofLogLevel level, const string & module, const string & message) {

	string filteredModName = filterModuleName(module);
	if(loggingToFile) fileLogger.log(level, filteredModName, message);
	if(loggingToScreen) displayLogger.log(level, filteredModName, message);
	if(loggingToConsole){
		if(colorTerm){ //colorize term output
			string colorMsg;
			switch (level) {
				case OF_LOG_VERBOSE: colorMsg = "\033[0;37m"; break; //gray
				case OF_LOG_NOTICE: colorMsg = "\033[0;34m"; break; //blue
				case OF_LOG_WARNING: colorMsg = "\033[0;33m"; break; //yellow
				case OF_LOG_ERROR: colorMsg = "\033[0;31m"; break; //red
				case OF_LOG_FATAL_ERROR: colorMsg = "\033[0;35m"; break; //purple
				default : break;
			}
			colorMsg += message + "\033[0;0m";
			consoleLogger.log(level, filteredModName, colorMsg);
		}else{
			consoleLogger.log(level, filteredModName, message);
		}
	}
}

void ofxSuperLog::log(ofLogLevel logLevel, const string & module, const char* format, ...) {

	va_list args;
	va_start(args, format);
	log(logLevel, filterModuleName(module), format, args);
	va_end(args);
}

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

	string filteredModName = filterModuleName(module);

	if(loggingToFile) fileLogger.log(logLevel, filteredModName, aux64);
	if(loggingToScreen) displayLogger.log(logLevel, filteredModName, aux64);
	if(loggingToConsole) consoleLogger.log(logLevel, filteredModName, aux64);
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
