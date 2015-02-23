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
#define DEFAULT_NUM_LOG_LINES 50

#ifdef USE_OFX_FONTSTASH
#include "ofxFontStash.h"
#endif

class ofxSuperLogDisplay: public ofBaseLoggerChannel {
public:
	
	ofxSuperLogDisplay();
	virtual ~ofxSuperLogDisplay();
	
	void setMaxNumLogLines(int maxNumLogLines);
	void setEnabled(bool enabled);
	bool isEnabled();
	
	void setMinimized(bool minimized);
	bool isMinimized();
	
	
	#ifdef USE_OFX_FONTSTASH
	void setFont(ofxFontStash* f, float fontSize_ = 16.0f){font = f; fontSize = fontSize_;}
	#endif

	
	void draw(ofEventArgs &e);
	
	void mousePressed(ofMouseEventArgs &e);
	void mouseMoved(ofMouseEventArgs &e);
	void mouseDragged(ofMouseEventArgs &e);
	void mouseReleased(ofMouseEventArgs &e);
	
	
	
	
	void log(ofLogLevel level, const string & module, const string & message);
    
	void log(ofLogLevel logLevel, const string & module, const char* format, ...);
    
	void log(ofLogLevel logLevel, const string & module, const char* format, va_list args);
	
protected:
	bool enabled;
	deque<string> logLines;
	int MAX_NUM_LOG_LINES;
	bool minimized;
	
	float width;
	ofRectangle minimizedRect;
	bool draggingWidth;

	ofMutex mutex;
	#ifdef USE_OFX_FONTSTASH
	ofxFontStash * font;
	float fontSize;
	#endif
};
