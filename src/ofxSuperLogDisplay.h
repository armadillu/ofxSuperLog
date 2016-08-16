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
 *				 
 *  ofxSuperLogDisplay.h, created by Marek Bereza on 02/09/2013.
 */

#pragma once
#include "ofMain.h"
#define DEFAULT_NUM_LOG_LINES 4096

#if defined(__has_include) /*llvm only - query about header files being available or not*/
	#if __has_include("ofxFontStash.h") && !defined(DISABLE_AUTO_FIND_FONSTASH_HEADERS)
		#define USE_OFX_FONTSTASH
	#endif
#endif

#ifdef USE_OFX_FONTSTASH
#include "ofxFontStash.h"
#endif

class ofxSuperLogDisplay: public ofBaseLoggerChannel {
public:
	
	ofxSuperLogDisplay();
	virtual ~ofxSuperLogDisplay();
	
	void setMaxNumLogLines(int maxNumLogLines);
	void setEnabled(bool enabled);
	bool getEnabled(){return enabled;}
	void setAutoDraw(bool d){ autoDraw = d;}
	bool isEnabled();
	
	void setMinimized(bool minimized);
	bool isMinimized();

	void setUseColors(bool useC){useColors = useC;};
	void setColorForLogLevel(ofLogLevel l, const ofColor &c){ logColors[l] = c;}

	///this defines how much space the on-screen logging will take when the log is visible
	///the panel is always on the right side. You must supply a % [0..1] of how much of the
	///whole screen the panel takes.
	void setPanelWidth(float pct){widthPct = pct;}
	
	#ifdef USE_OFX_FONTSTASH
	void setFont(ofxFontStash* f, float fontSize_ = 16.0f);
	#endif

	void draw(float w, float h); //for manual drawing
	
	void mousePressed(ofMouseEventArgs &e);
	void mouseMoved(ofMouseEventArgs &e);
	void mouseDragged(ofMouseEventArgs &e);
	void mouseReleased(ofMouseEventArgs &e);
	

	void log(ofLogLevel level, const string & module, const string & message);
	void log(ofLogLevel logLevel, const string & module, const char* format, ...);
	void log(ofLogLevel logLevel, const string & module, const char* format, va_list args);

	
protected:

	void draw(ofEventArgs &e);

	struct LogLine{
		string line;
		string module;
		ofLogLevel level;
		LogLine(string modName, string lin, ofLogLevel lev){ line = lin; module = modName, level = lev; }
	};

	bool enabled;
	bool autoDraw;
	deque<LogLine> logLines;

	float lastW; //manual drawing
	float lastH;

	ofColor& getColorForModule(const string& modName);

	int MAX_NUM_LOG_LINES;
	bool minimized;
	
	float widthPct; //how wide is the logging scrollist , pct of ofGetWidth()
	ofRectangle minimizedRect;
	bool draggingWidth;

	//scroll v
	bool scrolling;
	float scrollV;
	float prevY, prevY2;
	int oldestLineOnScreen;
	int newestLineOnScreen;
	float inertia;
	float dragSpeed;

	ofMutex mutex;

	bool useColors;
	ofColor logColors[6]; //6 being the # of ofLogLevels. This is not very future proof - TODO!
	map<string, ofColor> moduleColors;

	#ifdef USE_OFX_FONTSTASH
	ofxFontStash * font;
	float fontSize;
	#endif
	float lineH;
	size_t maxModuleLen = 8; //len of the longest OF log module
};
