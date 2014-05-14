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
 *				To enable: in testApp::setup()
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
#include "ofxSuperLogDisplay.h"

#pragma once

#define LOG_TIMESTAMP				string(ofToString(ofGetHours(), 2, '0')			\
									+ ":" + ofToString(ofGetMinutes(), 2, '0')		\
									+ ":" + ofToString(ofGetSeconds(), 2, '0') )

//#define LOG_CONTEXTUAL_INFO		"[" << typeid(this).name() << "::" << __func__ << "() @ " << __LINE__ << " ]"
#define LOG_CONTEXTUAL_INFO			LOG_TIMESTAMP << " " << typeid(this).name() << " : " << __func__ << "() "
#define LOG_CONTEXTUAL_INFO_STATIC	LOG_TIMESTAMP << " " <<  __func__ << "() "

//normal
#define LOG_VERBOSE					ofLogVerbose() << LOG_CONTEXTUAL_INFO
#define LOG_NOTICE					ofLogNotice() << LOG_CONTEXTUAL_INFO
#define LOG_WARNING					ofLogWarning() << LOG_CONTEXTUAL_INFO
#define LOG_ERROR					ofLogError() << LOG_CONTEXTUAL_INFO
#define LOG_FATAL_ERROR				ofLogFatalError() << LOG_CONTEXTUAL_INFO
#define LOG							ofLog() << LOG_CONTEXTUAL_INFO
#define LOG_STATIC					ofLog() << LOG_CONTEXTUAL_INFO_STATIC

//short
#define SLOG_VERBOSE				ofLogVerbose() << LOG_TIMESTAMP << " "
#define SLOG_NOTICE					ofLogNotice() << LOG_TIMESTAMP << " "
#define SLOG_WARNING				ofLogWarning() << LOG_TIMESTAMP << " "
#define SLOG_ERROR					ofLogError() << LOG_TIMESTAMP << " "
#define SLOG_FATAL_ERROR			ofLogFatalError() << LOG_TIMESTAMP << " "
#define SLOG						ofLog() << LOG_TIMESTAMP << " "


class ofxSuperLog: public ofBaseLoggerChannel {

public:

	static ofPtr<ofxSuperLog> &getLogger(bool writeToConsole = true, bool drawToScreen = true, string logDirectory = "");


	// this compresses all the old logs
	// - the first parameter, is how many of the most recent logs to keep as plain text,
	// the second is how many after that you want compressed. If you set either value
	// to -1 it means infinite.
	// so if you set numUncompressedToKeep==-1 then all logs are kept forever and in plain text
    // but if you call with default args (0, -1) then all logs are kept but in compressed format.
    void archiveOldLogs(int numUncompressedToKeep = 0, int numCompressedToKeep = -1);

	// in the display, this sets how many lines to remember.
	// not that useful unless there is scrollback.
	void setMaxNumLogLines(int maxNumLogLines);

	void setScreenLoggingEnabled(bool enabled);

	bool isScreenLoggingEnabled();

	void setMaximized(bool maximized);


	void log(ofLogLevel level, const string & module, const string & message);

	void log(ofLogLevel logLevel, const string & module, const char* format, ...);

	void log(ofLogLevel logLevel, const string & module, const char* format, va_list args);

	virtual ~ofxSuperLog();



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

};
