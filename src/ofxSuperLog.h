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

#ifdef TARGET_OSX
#include <cxxabi.h>
#endif

#ifdef USE_OFX_FONTSTASH
#include "ofxFontStash.h"
#endif

#pragma once

#define LOG_TIMESTAMP				string(ofToString(ofGetHours(), 2, '0')			\
									+ ":" + ofToString(ofGetMinutes(), 2, '0')		\
									+ ":" + ofToString(ofGetSeconds(), 2, '0') )

//#define LOG_CONTEXTUAL_INFO		"[" << typeid(this).name() << "::" << __func__ << "() @ " << __LINE__ << " ]"

#ifdef TARGET_OSX
#define LOG_CONTEXTUAL_INFO				__func__ << "() @ " << LOG_TIMESTAMP
#define LOG_CONTEXTUAL_INFO_STATIC		__func__ << "() @ " << LOG_TIMESTAMP
#else
#define LOG_CONTEXTUAL_INFO				string(demangled_type_info_name(typeid(this))) << " " << LOG_TIMESTAMP
#define LOG_CONTEXTUAL_INFO_STATIC		__FUNCTION__ << "() @ "<< LOG_TIMESTAMP
#endif


#ifdef TARGET_OSX
	#define WARN_EMOJI		"⚠️"
	#define ERR_EMOJI		"‼️"
	#define F_ERR_EMOJI		"💣"
	#define NOTICE_EMOJI	"💬"
#else
	#define WARN_EMOJI		"#"
	#define ERR_EMOJI		"#"
	#define F_ERR_EMOJI		"#"
	#define NOTICE_EMOJI	"#"
#endif

//normal
#define LOG							ofLogNotice(demangled_type_info_name(typeid(this))) << LOG_CONTEXTUAL_INFO << " " << NOTICE_EMOJI << " "
#define LOG_VERBOSE					ofLogVerbose(demangled_type_info_name(typeid(this))) << LOG_CONTEXTUAL_INFO << " "
#define LOG_NOTICE					ofLogNotice(demangled_type_info_name(typeid(this))) << LOG_CONTEXTUAL_INFO << " " << NOTICE_EMOJI << " "
#define LOG_WARNING					ofLogWarning(demangled_type_info_name(typeid(this))) << LOG_CONTEXTUAL_INFO << " " << WARN_EMOJI << " "
#define LOG_ERROR					ofLogError(demangled_type_info_name(typeid(this))) << LOG_CONTEXTUAL_INFO << " " << ERR_EMOJI << " "
#define LOG_FATAL_ERROR				ofLogFatalError(demangled_type_info_name(typeid(this))) << LOG_CONTEXTUAL_INFO << " " << F_ERR_EMOJI << " "

//for static methods
#define LOG_STATIC					ofLog() << "[" << LOG_CONTEXTUAL_INFO_STATIC << "] " << NOTICE_EMOJI << " "
#define LOG_STATIC_ERROR			ofLogError() << "[" << LOG_CONTEXTUAL_INFO_STATIC << "] " << ERR_EMOJI << " "
#define LOG_STATIC_FATAL_ERROR		ofLogFatalError() << "[" << LOG_CONTEXTUAL_INFO_STATIC << "] " << F_ERR_EMOJI << " "

//same as normal, but specifying a module
//this way you can shut individual modules with ofSetLogLevel(moduleName, level);
#define MLOG(module)				ofLogNotice(module) << LOG_CONTEXTUAL_INFO << " " << NOTICE_EMOJI << " "
#define MLOG_VERBOSE(module)		ofLogVerbose(module) << LOG_CONTEXTUAL_INFO << " "
#define MLOG_WARNING(module)		ofLogWarning(module) << LOG_CONTEXTUAL_INFO << " " << WARN_EMOJI << " "
#define MLOG_ERROR(module)			ofLogError(module) << LOG_CONTEXTUAL_INFO << " " << ERR_EMOJI << " "
#define MLOG_FATAL_ERROR(module)	ofLogFatalError(module) << LOG_CONTEXTUAL_INFO << " " << F_ERR_EMOJI << " "
#define MLOG_STATIC(module)			ofLogNotice(module) << LOG_CONTEXTUAL_INFO_STATIC << " " << NOTICE_EMOJI << " "

//short log - less info
#define SLOG						ofLog() << LOG_TIMESTAMP << " " << NOTICE_EMOJI << " "
#define SLOG_VERBOSE				ofLogVerbose() << LOG_TIMESTAMP << " "
#define SLOG_NOTICE					ofLogNotice() << LOG_TIMESTAMP << " " << NOTICE_EMOJI << " "
#define SLOG_WARNING				ofLogWarning() << LOG_TIMESTAMP << " " << WARN_EMOJI << " "
#define SLOG_ERROR					ofLogError() << LOG_TIMESTAMP << " " << ERR_EMOJI << " "
#define SLOG_FATAL_ERROR			ofLogFatalError() << LOG_TIMESTAMP << " " << F_ERR_EMOJI << " "


static char demangleSpace[4096];
static ofMutex logMutex;

inline std::string demangled_type_info_name(const std::type_info&ti){

	ofScopedLock lock(logMutex);

	#ifdef TARGET_WIN32
	static std::vector<std::string> keywords;
	if ( 0 == keywords.size() ) {
		keywords.push_back("class ");
		keywords.push_back("struct ");
		keywords.push_back("enum ");
		keywords.push_back("union ");
		keywords.push_back("__cdecl");
	}
	std::string r = ti.name();
	for ( size_t i = 0; i < keywords.size(); i ++ ) {
		while (r.find(keywords[i]) != std::string::npos) 
			r = r.replace(r.find(keywords[i]), keywords[i].size(), "");
		while (r.find(" *") != std::string::npos) 
			r = r.replace(r.find(" *"), 2, "*");
		while (r.find(" &") != std::string::npos) 
			r = r.replace(r.find(" &"), 2, "&");
	}
	if(r.size() > 0){
		r = r.substr(0, r.size() - 1);
	}

	return r;
	#else
	int status = 0;
	size_t len = 4096;
	char * ret = abi::__cxa_demangle(ti.name(),(char*)&demangleSpace, &len, &status);
	string finalS = string(demangleSpace);
	if(finalS.size() > 0){
		finalS = finalS.substr(0, finalS.size() - 1);
	}
	return finalS;
	#endif
}


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

	#ifdef USE_OFX_FONTSTASH
	void setFont(ofxFontStash * font, float fontSiz);
	#endif

	void setUseScreenColors(bool u){ displayLogger.setUseColors(u); }
	void setColorForLogLevel(ofLogLevel l, const ofColor &c){ displayLogger.setColorForLogLevel(l, c); }
	void setAutoDraw(bool autoDraw){displayLogger.setAutoDraw(autoDraw);}

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
