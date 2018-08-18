/**     ___           ___           ___                         ___           ___
 *     /__/\         /  /\         /  /\         _____         /  /\         /__/|
 *    |  |::\       /  /::\       /  /::|       /  /::\       /  /::\       |  |:|
 *    |  |:|:\     /  /:/\:\     /  /:/:|      /  /:/\:\     /  /:/\:\      |  |:|
 *  __|__|:|\:\   /  /:/~/::\   /  /:/|:|__   /  /:/~/::\   /  /:/  \:\   __|__|:|
 * /__/::::| \:\ /__/:/ /:/\:\ /__/:/ |:| /\ /__/:/ /:/\:| /__/:/ \__\:\ /__/::::\____
 * \  \:\~~\__\/ \  \:\/:/__\/ \__\/  |:|/:/ \  \:\/:/~/:/ \  \:\ /  /:/    ~\~~\::::/
 *  \  \:\        \  \::/          |  |:/:/   \  \::/ /:/   \  \:\  /:/      |~~|:|~~
 *   \  \:\        \  \:\          |  |::/     \  \:\/:/     \  \:\/:/       |  |:|
 *    \  \:\        \  \:\         |  |:/       \  \::/       \  \::/        |  |:|
 *     \__\/         \__\/         |__|/         \__\/         \__\/         |__|/
 *
 *  Description:
 *				 make your ofLog display on the screen and optionally to a file and the console.
 *
 *  Usage:
 *				To enable: in ofApp::setup()
 *				Do this:
 *					ofSetLoggerChannel(ofxSuperLog::getLogger(true, true, "logs"));
 *
 *				And it will log your logs to a file with a timestamp in logs/ and log to the screen.
 *
 *				To turn off screen logging temporarily:
 *				ofxSuperLog::getLogger().setScreenLoggingEnabled(false);
 *
 *  ofxSuperLog.h, created by Marek Bereza on 02/09/2013.
 */



#include "ofMain.h"

#if defined(__has_include) /*llvm only - query about header files being available or not*/
	#if __has_include("ofxFontStash.h") && !defined(DISABLE_AUTO_FIND_FONSTASH_HEADERS)
		#ifndef USE_OFX_FONTSTASH
			#define USE_OFX_FONTSTASH
		#endif
	#endif
#endif

#include "ofxSuperLogDisplay.h"

#if defined(TARGET_OSX) || defined(TARGET_LINUX)
#include <cxxabi.h>
#endif

#ifdef USE_OFX_FONTSTASH
	class ofxFontStash;
#endif

#define SUPERLOG_TYPE_NAME							demangled_type_info_name(typeid(this))
#define SUPERLOG_TYPE_NAME_WITH_FUNC				(demangled_type_info_name(typeid(this)) + "::" + string(__func__) + "()")
#define SUPERLOG_TYPE_NAME_WITH_FUNC_AND_LINE		(demangled_type_info_name(typeid(this)) + "::" + string(__func__) + "():" + ofToString(__LINE__))

#pragma once


class ofxSuperLog: public ofBaseLoggerChannel {

public:

	static ofPtr<ofxSuperLog> &getLogger(bool writeToConsole = true, bool drawToScreen = true, string logDirectory = "");

	// this affects the display only,  sets how many lines the scroll will have.
	void setMaxNumLogLines(int maxNumLogLines);

	void setScreenLoggingEnabled(bool enabled);
	bool isScreenLoggingEnabled();

	void setMaximized(bool maximized);

	//enabling this will lock/unlock a mutex for every ofLog() command to avoid mixed-up lines bc of de-synced logging
	//probably big performance hit though!
	void setSyncronizedLogging(bool useMutex);

	#ifdef USE_OFX_FONTSTASH
	void setFont(ofxFontStash * font, float fontSiz);
	#endif

	void setUseScreenColors(bool u){ displayLogger.setUseColors(u); }
	void setColorForLogLevel(ofLogLevel l, const ofColor &c){ displayLogger.setColorForLogLevel(l, c); }
	void setAutoDraw(bool autoDraw){displayLogger.setAutoDraw(autoDraw);}
	void setColorTerm(bool color) { colorTerm = color;}

	///this defines how much space the on-screen logging will take when the log is visible
	///the panel is always on the right side. You must supply a % [0..1] of how much of the
	///whole screen the panel takes.
	void setDisplayWidth( float widthPercent){displayLogger.setPanelWidth(widthPercent);}

	void log(ofLogLevel level, const string & module, const string & message);

	void log(ofLogLevel logLevel, const string & module, const char* format, ...);

	void log(ofLogLevel logLevel, const string & module, const char* format, va_list args);

	virtual ~ofxSuperLog();

	void draw(float w, float h);

	static void clearOldLogs(string path, int numDays);
    
    ofxSuperLogDisplay& getDisplayLogger(){return displayLogger;}
	
	void setFileLogShowsTimestamps(bool t){fileLogShowsTimestamps = t;}
	
	string getCurrentLogFile(){return currentLogFile;}

	void setConsoleShouldShowTimestamps(bool c){consoleShowTimestamps = c;}
private:

	static ofPtr<ofxSuperLog> logger;
	static ofxSuperLog *logPtr;
	ofxSuperLog(bool writeToConsole, bool drawToScreen, string logDirectory);

    string logDirectory;

	bool loggingToFile;
	bool loggingToScreen;
	bool loggingToConsole;
	ofConsoleLoggerChannel consoleLogger;
	ofFileLoggerChannel fileLogger;
	ofxSuperLogDisplay displayLogger;

	bool fileLogShowsTimestamps = true;
	bool consoleShowTimestamps = false;
	
	string currentLogFile;
	
	string filterModuleName(const string &);
	size_t maxModuleLen = 8; //len of the longest OF log module
	bool colorTerm = false;
	
	bool useMutex = false;
	ofMutex syncLogMutex;

	string getEmojiForLogLevel(ofLogLevel level);
};

std::string demangled_type_info_name(const std::type_info&ti);
