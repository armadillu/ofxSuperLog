/**
 *  ofxSuperLog.cpp
 *
 *  Created by Marek Bereza on 02/09/2013.
 */

#include "ofxSuperLog.h"
#ifdef TARGET_WIN32
#include <VersionHelpers.h>
#endif

#ifdef USE_OFX_FONTSTASH
	#include "ofxFontStash.h"
#endif

#include <time.h>
#include <sys/stat.h>
#include <stdio.h>
<<<<<<< Updated upstream
#include <cstdarg>
#include <cstdint>
#include <chrono>
#include <utility>
=======
>>>>>>> Stashed changes

ofPtr<ofxSuperLog> ofxSuperLog::logger;
ofxSuperLog *ofxSuperLog::logPtr = NULL;

ofPtr<ofxSuperLog> &ofxSuperLog::getLogger(bool writeToConsole, bool drawToScreen, string logDirectory) {
	if(logPtr == NULL) {
		logPtr = new ofxSuperLog(writeToConsole, drawToScreen, logDirectory);
		logger = ofPtr<ofxSuperLog>(logPtr);

	}
	return logger;
}

#ifdef TARGET_WIN32
//http://stackoverflow.com/questions/36543301/detecting-windows-10-version
typedef LONG NTSTATUS, *PNTSTATUS;
#define STATUS_SUCCESS (0x00000000)
typedef NTSTATUS(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

RTL_OSVERSIONINFOW GetRealOSVersion() {
	HMODULE hMod = ::GetModuleHandleW(L"ntdll.dll");
	if (hMod) {
		RtlGetVersionPtr fxPtr = (RtlGetVersionPtr)::GetProcAddress(hMod, "RtlGetVersion");
		if (fxPtr != nullptr) {
			RTL_OSVERSIONINFOW rovi = { 0 };
			rovi.dwOSVersionInfoSize = sizeof(rovi);
			if (STATUS_SUCCESS == fxPtr(&rovi)) {
				return rovi;
			}
		}
	}
	RTL_OSVERSIONINFOW rovi = { 0 };
	return rovi;
}
#endif

ofxSuperLog::ofxSuperLog(bool writeToConsole, bool drawToScreen, string logDirectory)
: queue(new QueueSlot[queueCapacity]) {
	for (std::size_t i = 0; i < queueCapacity; ++i) queue[i].sequence.store(i, std::memory_order_relaxed);

	#ifdef TARGET_WIN32
	
	colorTerm = GetRealOSVersion().dwMajorVersion >= 10;
	//colorTerm = IsWindows10OrGreater();
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

	if(colorTerm) ofLogNotice("ofxSuperLog") << "Enabling colored console output";

	this->loggingToFile = logDirectory!="";
	this->loggingToScreen = drawToScreen;
	this->loggingToConsole = writeToConsole;
	this->logDirectory = logDirectory;
	if(loggingToFile) {
		if(!ofFile(logDirectory).exists()) {
			ofDirectory dir(logDirectory);
			dir.create();
		}
		#ifdef TARGET_WIN32
		string fileName = ofGetTimestampString("%Y-%m-%d + %H-%M-%S + %A");
		#else
		string fileName = ofGetTimestampString("%Y-%m-%d | %H-%M-%S | %A");
		#endif
		
		currentLogFile = logDirectory + "/" + fileName + ".log";
		fileLogger.setFile(currentLogFile, true);
	}
	if(drawToScreen) {
		displayLogger.setEnabled(true);
	}
	worker = std::thread(&ofxSuperLog::consume, this);
}

ofxSuperLog::~ofxSuperLog() {
	running.store(false, std::memory_order_release);
	if (worker.joinable()) worker.join();
}


string ofxSuperLog::getEmojiForLogLevel(ofLogLevel level){
	switch (level) {
		case OF_LOG_WARNING: return "⚠️";
		case OF_LOG_ERROR: return "‼️";
		case OF_LOG_FATAL_ERROR: return "💣";
  		default: return "";
	}
}

//from https://stackoverflow.com/questions/61030383/how-to-convert-stdfilesystemfile-time-type-to-time-t
template <typename TP>
std::time_t to_time_t(TP tp){
	using namespace std::chrono;
	auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now() + system_clock::now());
	return system_clock::to_time_t(sctp);
}

void ofxSuperLog::clearOldLogs(string path, int numDays){

	string ppath = ofFilePath::getPathForDirectory(path);
	if(ppath.empty()){
		ofLogError("ofxSuperLog") << "can't clearOldLogs; supplied path not found: " << path;
		return;
	}

	if(!ofDirectory::doesDirectoryExist(ppath)){
		ofDirectory::createDirectory(ppath);
	}

	ofDirectory dir;
	dir.listDir(ppath);
	std::time_t now = std::time( nullptr ) ;

	for(int i = 0; i < dir.size() ; i++){
		string logFilePath = dir.getPath(i);
		string logFileAbsolutePath = ofToDataPath(logFilePath, true);

		struct stat file_details;
		stat(logFileAbsolutePath.c_str(), &file_details);
		time_t modify_time = file_details.st_mtime;

		int secondsOld = std::difftime(now, modify_time);
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

// Per-slot sequence MPMC ring based on Dmitry Vyukov's bounded queue:
// https://www.1024cores.net/home/lock-free-algorithms/queues/bounded-mpmc-queue
bool ofxSuperLog::tryEnqueue(LogEntry &entry) {
	std::size_t pos = enqueuePos.load(std::memory_order_relaxed);
	for (;;) {
		QueueSlot &slot = queue[pos % queueCapacity];
		const std::size_t seq = slot.sequence.load(std::memory_order_acquire);
		const std::intptr_t diff = static_cast<std::intptr_t>(seq - pos);
		if (diff == 0) {
			if (enqueuePos.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) {
				slot.entry = std::move(entry);
				slot.sequence.store(pos + 1, std::memory_order_release);
				return true;
			}
		} else if (diff < 0) return false;
		else pos = enqueuePos.load(std::memory_order_relaxed);
	}
}

bool ofxSuperLog::tryDequeue(LogEntry &entry) {
	std::size_t pos = dequeuePos.load(std::memory_order_relaxed);
	for (;;) {
		QueueSlot &slot = queue[pos % queueCapacity];
		const std::size_t seq = slot.sequence.load(std::memory_order_acquire);
		const std::intptr_t diff = static_cast<std::intptr_t>(seq - (pos + 1));
		if (diff == 0) {
			if (dequeuePos.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) {
				entry = std::move(slot.entry);
				slot.sequence.store(pos + queueCapacity, std::memory_order_release);
				return true;
			}
		} else if (diff < 0) return false;
		else pos = dequeuePos.load(std::memory_order_relaxed);
	}
}

void ofxSuperLog::enqueue(ofLogLevel level, string module, string message) {
	LogEntry entry;
	entry.level = level;
	entry.module = std::move(module);
	entry.message = std::move(message);
	entry.fileTimestamp = fileLogShowsTimestamps.load(std::memory_order_relaxed);
	entry.consoleTimestamp = consoleShowTimestamps.load(std::memory_order_relaxed);
	entry.color = colorTerm.load(std::memory_order_relaxed);
	if (entry.fileTimestamp || entry.consoleTimestamp)
		entry.timestamp = std::time(nullptr);
	// Preserve messages under load; the consumer must remain alive while producers run.
	while (!tryEnqueue(entry)) std::this_thread::yield();
}

void ofxSuperLog::log(ofLogLevel level, const string &module, const string &message) {
	enqueue(level, module, message);
}

void ofxSuperLog::consume() {
	LogEntry entry;
	while (running.load(std::memory_order_acquire)) {
		if (tryDequeue(entry)) writeEntry(entry);
		else std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	while (tryDequeue(entry)) writeEntry(entry);
}

void ofxSuperLog::writeEntry(const LogEntry &entry) {
	const ofLogLevel level = entry.level;
	const string &module = entry.module;
	const string &message = entry.message;
	string timeOfLog;
	if (entry.fileTimestamp || entry.consoleTimestamp) {
		std::tm localTime;
#ifdef TARGET_WIN32
		localtime_s(&localTime, &entry.timestamp);
#else
		localtime_r(&entry.timestamp, &localTime);
#endif
		char buffer[32];
		if (std::strftime(buffer, sizeof(buffer), "%Y/%m/%d %H:%M:%S", &localTime)) timeOfLog = buffer;
	}
	string filteredModName = filterModuleName(module);

	if(loggingToFile){
		if(entry.fileTimestamp){
			fileLogger.log(level, filteredModName, timeOfLog + " - " + message);
		}else{
			fileLogger.log(level, filteredModName, message);
		}
	}
	if(loggingToScreen) displayLogger.log(level, filteredModName, message);
	if(loggingToConsole){
		string emojiIcon = "";
		#if defined(TARGET_OSX) //sadly Xcode doesn't allow for colored console, but its really helpful to get warnings and errs to stand out
								//so we use emoji for that (which work on OSX but not so much on win)
		emojiIcon = getEmojiForLogLevel(level) + " ";
		#endif
		if(entry.color){ //colorize term output
			string colorMsg;
			switch (level) {
				case OF_LOG_VERBOSE: colorMsg = "\033[0;37m"; break; //gray
				case OF_LOG_NOTICE: colorMsg = "\033[0;32m"; break; //green
				case OF_LOG_WARNING: colorMsg = "\033[30;43m"; break; //yellow
				case OF_LOG_ERROR: colorMsg = "\033[30;41m"; break; //red bg
				case OF_LOG_FATAL_ERROR: colorMsg = "\033[30;45m"; break; //purple
				default : break;
			}
			if(entry.consoleTimestamp){
				consoleLogger.log(level, filteredModName, emojiIcon + colorMsg + timeOfLog + " - " + message + "\033[0;0m");
			}else{
				consoleLogger.log(level, filteredModName, emojiIcon + colorMsg + message + "\033[0;0m");
			}

		}else{
			if(entry.consoleTimestamp){
				consoleLogger.log(level, filteredModName, emojiIcon + timeOfLog + " - " + message);
			}else{
				consoleLogger.log(level, filteredModName, emojiIcon + message);
			}
		}
	}
	/*
	if(logToNotification){
		if (level >= OF_LOG_ERROR){
			string removeCrap  = message;
			ofStringReplace(removeCrap, "\"", "'");
			ofSystem("osascript -e 'display notification \"" + removeCrap + "\" with title \"" + module + "\"'");
		}
	}
	*/
#if defined(_WIN32) || defined(_WIN64)
	if (bWindowsEventLoggingEnabled) {

		// Tutorial:
		// https://stackoverflow.com/questions/37035958/log-to-event-viewer-on-windows-with-c
		// Reference:
		// https://docs.microsoft.com/en-us/windows/desktop/api/Winbase/nf-winbase-reporteventa
		// https://docs.microsoft.com/en-us/windows/desktop/EventLog/reporting-an-event

		// Register the source of these logs.
		// The source should be this application
		HANDLE windows_event_log;
		windows_event_log = RegisterEventSourceA(NULL, windowsEventLoggingName.c_str());

		// Parameters of the event (* = relevant)
		WORD wType = EVENTLOG_SUCCESS;		// * Success, Error, Information, Warning
		switch (level) {
		case OF_LOG_VERBOSE: wType = EVENTLOG_SUCCESS; break;
		case OF_LOG_NOTICE: wType = EVENTLOG_INFORMATION_TYPE; break;
		case OF_LOG_WARNING: wType = EVENTLOG_WARNING_TYPE; break;
		case OF_LOG_ERROR: wType = EVENTLOG_ERROR_TYPE; break;
		case OF_LOG_FATAL_ERROR: wType = EVENTLOG_ERROR_TYPE; break; // any other differentiation?
		default: break;
		}
		WORD wCategory = 0;					// * Can have any value
		switch (level) {
		case OF_LOG_VERBOSE: wCategory = 4; break;
		case OF_LOG_NOTICE: wCategory = 3; break;
		case OF_LOG_WARNING: wCategory = 2; break;
		case OF_LOG_ERROR: wCategory = 1; break;
		case OF_LOG_FATAL_ERROR: wCategory = 0; break; // any other differentiation?
		default: break;
		}

		DWORD dwEventID = 0;				// Specifies entry in message file
		PSID lpUserSid = NULL;				// Security Identifier
		WORD wNumStrings = 1;				// Number of strings in message array
		DWORD dwDataSize = 0;				// # bytes of event-specific binary data to write to log

		//string message;					// * Message
		string thisMsg = "";
		thisMsg += "App:\t" + windowsEventLoggingName + "\n";
		thisMsg += "Module:\t" + module + "\n";
		thisMsg += "Level:\t";
		switch (level) {
		case OF_LOG_VERBOSE: thisMsg += "Verbose"; break;
		case OF_LOG_NOTICE: thisMsg += "Notice"; break;
		case OF_LOG_WARNING: thisMsg += "Warning"; break;
		case OF_LOG_ERROR: thisMsg += "Error"; break;
		case OF_LOG_FATAL_ERROR: thisMsg += "Fatal Error"; break; // any other differentiation?
		default: thisMsg += "Unknown";  break;
		}
		thisMsg += "\n";
		thisMsg += "\n";
		thisMsg += "Message:\n" + message;

		LPVOID lpRawData = NULL;			// Binary data to write
		const char* charMsg = thisMsg.c_str();


		// TODO: Should each module have its own event log source?


		// Alt: if binary data is to be sent
		//CONST LPWSTR lpRawData = L"The command that was not valid";
		//DWORD dwDataSize = ((DWORD)wcslen(lpRawData) + 1) * sizeof(WCHAR);

		// Report the event
		bool bSuccess = ReportEventA(windows_event_log, wType, wCategory, dwEventID, lpUserSid, wNumStrings, dwDataSize, &charMsg, lpRawData);

	}
#endif

}

void ofxSuperLog::log(ofLogLevel logLevel, const string & module, const char* format, ...) {
	va_list args;
	va_start(args, format);
	log(logLevel, module, format, args);
	va_end(args);
}

void ofxSuperLog::setSyncronizedLogging(bool){
	// All output is serialized by the queue consumer.
}


void ofxSuperLog::log(ofLogLevel logLevel, const string & module, const char* format, va_list args) {

	enqueue(logLevel, module, ofVAArgsToString(format, args));
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

void ofxSuperLog::setWindowsEventLogging(bool _bEnabled, string _logName) {
	
#if defined(_WIN32) || defined(_WIN64)

	bWindowsEventLoggingEnabled = _bEnabled;
	if (!_logName.empty()) windowsEventLoggingName = _logName;

#else
	ofLogNotice("ofxSuperLog") << "Windows Event Logging is not enabled for Mac, Linux, etc.";
#endif
}


std::string demangled_type_info_name(const std::type_info&ti) {

#ifdef TARGET_WIN32
	static std::vector<std::string> keywords = { "class ", "struct ", "enum ", "union ", "__cdecl", "__ptr64" };
	std::string r = ti.name();
	for (size_t i = 0; i < keywords.size(); i++) {
		while (r.find(keywords[i]) != std::string::npos) r = r.replace(r.find(keywords[i]), keywords[i].size(), "");
		while (r.find("*") != std::string::npos) r = r.replace(r.find("*"), 1, "");
		while (r.find("&") != std::string::npos) r = r.replace(r.find("&"), 1, "");
		while (r.find(" ") != std::string::npos) r = r.replace(r.find(" "), 1, "");
	}
	return r;
#else
	char demangleBuffer[512];
	int status = 0;
	size_t len = 4096;
	abi::__cxa_demangle(ti.name(), (char*)&demangleBuffer, &len, &status);
	//note theres no need to free the returned char* as we are providing our own buffer
	string finalS = string(demangleBuffer);
	if (finalS.size() > 0) {
		finalS = finalS.substr(0, finalS.size() - 1);
	}
	return finalS;
#endif
}


