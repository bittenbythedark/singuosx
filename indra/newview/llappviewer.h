/** 
 * @mainpage
 *
 * This is the sources for the Second Life Viewer;
 * information on the open source project is at 
 * https://wiki.secondlife.com/wiki/Open_Source_Portal
 *
 * The Mercurial repository for the trunk version is at
 * https://bitbucket.org/lindenlab/viewer-release
 * 
 * @section source-license Source License
 * @verbinclude LICENSE-source.txt
 * 
 * @section artwork-license Artwork License
 * @verbinclude LICENSE-logos.txt
 *
 * $LicenseInfo:firstyear=2007&license=viewerlgpl$
 * Second Life Viewer Source Code
 * Copyright (C) 2010, Linden Research, Inc.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License only.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 * Linden Research, Inc., 945 Battery Street, San Francisco, CA  94111  USA
 * $/LicenseInfo$
 *
 * @file llappviewer.h
 * @brief The LLAppViewer class declaration
 */

#ifndef LL_LLAPPVIEWER_H
#define LL_LLAPPVIEWER_H

#include "llallocator.h"
#include "llapp.h"
#include "llapr.h"
#include "llcontrol.h"
#include "llsys.h"			// for LLOSInfo
#include "lltimer.h"
#include "llappcorehttp.h"
#include "lltrace.h"

class LLCommandLineParser;
class LLFrameTimer;
class LLPumpIO;
class LLTextureCache;
class LLImageDecodeThread;
class LLTextureFetch;
class LLWatchdogTimeout;

extern LLTrace::BlockTimerStatHandle FTM_FRAME;

class LLAppViewer : public LLApp
{
public:
	LLAppViewer();
	virtual ~LLAppViewer();

    /**
     * @brief Access to the LLAppViewer singleton.
     * 
     * The LLAppViewer singleton is created in main()/WinMain().
     * So don't use it in pre-entry (static initialization) code.
     */
    static LLAppViewer* instance() {return sInstance; } 

	//
	// Main application logic
	//
	bool init() override;			// Override to do application initialization
	bool cleanup() override;			// Override to do application cleanup
	bool frame() override; // Override for application body logic

	// Application control
	void flushVFSIO(); // waits for vfs transfers to complete
	void forceQuit(); // Puts the viewer into 'shutting down without error' mode.
	void fastQuit(S32 error_code = 0); // Shuts down the viewer immediately after sending a logout message
	void requestQuit(); // Request a quit. A kinder, gentler quit.
	void userQuit(); // The users asks to quit. Confirm, then requestQuit()
    void earlyExit(const std::string& name, 
				   const LLSD& substitutions = LLSD()); // Display an error dialog and forcibly quit.
    void abortQuit();  // Called to abort a quit request.

    bool quitRequested() { return mQuitRequested; }
    bool logoutRequestSent() { return mLogoutRequestSent; }

	void writeDebugInfo(bool isStatic=true);

	const LLOSInfo& getOSInfo() const { return LLOSInfo::instance(); }

	// Report true if under the control of a debugger. A null-op default.
	virtual bool beingDebugged() { return false; } 

	virtual bool restoreErrorTrap() = 0; // Require platform specific override to reset error handling mechanism.
	                                     // return false if the error trap needed restoration.
	virtual void initCrashReporting(bool reportFreeze = false) = 0; // What to do with crash report?
	static void handleViewerCrash(); // Hey! The viewer crashed. Do this, soon.
    
	// Thread accessors
	static LLTextureCache* getTextureCache() { return sTextureCache; }
	static LLImageDecodeThread* getImageDecodeThread() { return sImageDecodeThread; }
	static LLTextureFetch* getTextureFetch() { return sTextureFetch; }

	static U32 getTextureCacheVersion() ;
	static U32 getObjectCacheVersion() ;

	const std::string& getSerialNumber() { return mSerialNumber; }
	
	bool getPurgeCache() const { return mPurgeCache; }
	
	const std::string& getSecondLifeTitle() const; // The Second Life title.
	const std::string& getWindowTitle() const; // The window display name.

    void forceDisconnect(const std::string& msg); // Force disconnection, with a message to the user.
    void badNetworkHandler(); // Cause a crash state due to bad network packet.

	bool hasSavedFinalSnapshot() { return mSavedFinalSnapshot; }
	void saveFinalSnapshot(); 

    void loadNameCache();
    void saveNameCache();

	void removeMarkerFiles();
	
	void removeDumpDir();
    // LLAppViewer testing helpers.
    // *NOTE: These will potentially crash the viewer. Only for debugging.
    virtual void forceErrorLLError();
    virtual void forceErrorBreakpoint();
    virtual void forceErrorBadMemoryAccess();
    virtual void forceErrorInfiniteLoop();
    virtual void forceErrorSoftwareException();
    virtual void forceErrorDriverCrash();

	// *NOTE: There are currently 3 settings files: 
	// "Global", "PerAccount" and "CrashSettings"
	// The list is found in app_settings/settings_files.xml
	// but since they are used explicitly in code,
	// the follow consts should also do the trick.
	static const std::string sGlobalSettingsName; 
	static const std::string sPerAccountSettingsName; 
	static const std::string sCrashSettingsName; 

	// Load settings from the location specified by loction_key.
	// Key availale and rules for loading, are specified in 
	// 'app_settings/settings_files.xml'
	bool loadSettingsFromDirectory(const std::string& location_key,
	                               bool set_defaults = false);

	std::string getSettingsFilename(const std::string& location_key,
	                                const std::string& file);

	// For thread debugging. 
	// llstartup needs to control init.
	// llworld, send_agent_pause() also controls pause/resume.
	void initMainloopTimeout(const std::string& state, F32 secs = -1.0f);
	void destroyMainloopTimeout();
	void pauseMainloopTimeout();
	void resumeMainloopTimeout(const std::string& state = "", F32 secs = -1.0f);
	void pingMainloopTimeout(const std::string& state, F32 secs = -1.0f);

	// Handle the 'login completed' event.
	// *NOTE:Mani Fix this for login abstraction!!
	void handleLoginComplete();

    LLAllocator & getAllocator() { return mAlloc; }

	// On LoginCompleted callback
	typedef boost::signals2::signal<void (void)> login_completed_signal_t;
	login_completed_signal_t mOnLoginCompleted;
	boost::signals2::connection setOnLoginCompletedCallback( const login_completed_signal_t::slot_type& cb ) { return mOnLoginCompleted.connect(cb); } 

	void addOnIdleCallback(const std::function<void()>& cb); // add a callback to fire (once) when idle

	void purgeCache(); // Clear the local cache. 

	// Metrics policy helper statics.
	static void metricsUpdateRegion(U64 region_handle);
	static void metricsSend(bool enable_reporting);

	// llcorehttp init/shutdown/config information.
	LLAppCoreHttp & getAppCoreHttp()			{ return mAppCoreHttp; }
	
protected:
	virtual bool initWindow(); // Initialize the viewer's window.
	virtual void initLoggingAndGetLastDuration(); // Initialize log files, logging system
	virtual void initLoggingPortable(); // <singu/> Portability mode
	virtual void initConsole() {}; // Initialize OS level debugging console.
	virtual bool initHardwareTest() { return true; } // A false result indicates the app should quit.
	virtual bool initSLURLHandler();
	virtual bool sendURLToOtherInstance(const std::string& url);

	virtual bool initParseCommandLine(LLCommandLineParser& clp) 
        { return true; } // Allow platforms to specify the command line args.

	virtual std::string generateSerialNumber() = 0; // Platforms specific classes generate this.


private:

	void initMaxHeapSize();
	bool initThreads(); // Initialize viewer threads, return false on failure.
	bool initConfiguration(); // Initialize settings from the command line/config file.

	bool initCache(); // Initialize local client cache.
	void checkMemory() ;

	// We have switched locations of both Mac and Windows cache, make sure
	// files migrate and old cache is cleared out.
	void migrateCacheDirectory();

	void cleanupSavedSettings(); // Sets some config data to current or default values during cleanup.
	void removeCacheFiles(const std::string& filemask); // Deletes cached files the match the given wildcard.

	void writeSystemInfo(); // Write system info to "debug_info.log"

	void processMarkerFiles(); 
	static void recordMarkerVersion(LLAPRFile& marker_file);
	bool markerIsSameVersion(const std::string& marker_name) const;
    
    void idle(); 
    void idleShutdown();
	// update avatar SLID and display name caches
	void idleNameCache();
    void idleNetwork();
	void idleAudio();
	void shutdownAudio();

    void sendLogoutRequest();
    void disconnectViewer();

	// *FIX: the app viewer class should be some sort of singleton, no?
	// Perhaps its child class is the singleton and this should be an abstract base.
	static LLAppViewer* sInstance; 

    bool mSecondInstance; // Is this a second instance of the app?

	std::string mMarkerFileName;
	LLAPRFile mMarkerFile; // A file created to indicate the app is running.

	std::string mLogoutMarkerFileName;
	LLAPRFile mLogoutMarkerFile; // A file created to indicate the app is running.

	bool mReportedCrash;

	// Thread objects.
	static LLTextureCache* sTextureCache; 
	static LLImageDecodeThread* sImageDecodeThread; 
	static LLTextureFetch* sTextureFetch;

	S32 mNumSessions;

	std::string mSerialNumber;
	bool mPurgeCache;
    bool mPurgeOnExit;

	bool mSavedFinalSnapshot;

    bool mQuitRequested;				// User wants to quit, may have modified documents open.
    bool mLogoutRequestSent;			// Disconnect message sent to simulator, no longer safe to send messages to the sim.
	LLSD mSettingsLocationList;

	LLWatchdogTimeout* mMainloopTimeout;

	// For performance and metric gathering
	class LLThread*	mFastTimerLogThread;

	// for tracking viewer<->region circuit death
	bool mAgentRegionLastAlive;
	LLUUID mAgentRegionLastID;

    LLAllocator mAlloc;

	LLFrameTimer mMemCheckTimer;
	
	// llcorehttp library init/shutdown helper
	LLAppCoreHttp mAppCoreHttp;

	//---------------------------------------------
	//*NOTE: Mani - legacy updater stuff
	// Still useable?
public:

	//some information for updater
	typedef struct
	{
		std::string mUpdateExePath;
		std::ostringstream mParams;
	}LLUpdaterInfo ;
	static LLUpdaterInfo *sUpdaterInfo ;
};

// consts from viewer.h
const S32 AGENT_UPDATES_PER_SECOND  = 10;

// Globals with external linkage. From viewer.h
// *NOTE:Mani - These will be removed as the Viewer App Cleanup project continues.
//
// "// llstartup" indicates that llstartup is the only client for this global.

extern LLSD gDebugInfo;
extern BOOL	gShowObjectUpdates;

extern BOOL gAcceptTOS;
extern BOOL gAcceptCriticalMessage;

typedef enum 
{
	LAST_EXEC_NORMAL = 0,
	LAST_EXEC_FROZE,
	LAST_EXEC_LLERROR_CRASH,
	LAST_EXEC_OTHER_CRASH,
	LAST_EXEC_LOGOUT_FROZE,
	LAST_EXEC_LOGOUT_CRASH
} eLastExecEvent;

extern eLastExecEvent gLastExecEvent; // llstartup

extern U32 gFrameCount;
extern U32 gForegroundFrameCount;

extern LLPumpIO* gServicePump;

// Is the Pacific time zone (aka server time zone)
// currently in daylight savings time?
extern BOOL gPacificDaylightTime;

extern U64      gFrameTime;					// The timestamp of the most-recently-processed frame
extern F32		gFrameTimeSeconds;			// Loses msec precision after ~4.5 hours...
extern F32		gFrameIntervalSeconds;		// Elapsed time between current and previous gFrameTimeSeconds
extern F32		gFPSClamped;				// Frames per second, smoothed, weighted toward last frame
extern F32		gFrameDTClamped;
extern U64		gStartTime;
extern U32 		gFrameStalls;

extern LLTimer gRenderStartTime;
extern LLFrameTimer gForegroundTime;

extern F32 gLogoutMaxTime;
extern LLTimer gLogoutTimer;

extern F32 gSimLastTime; 
extern F32 gSimFrames;

extern BOOL		gDisconnected;

extern LLFrameTimer	gRestoreGLTimer;
extern BOOL			gRestoreGL;
extern BOOL		gUseWireframe;

// VFS globals - gVFS is for general use
// gStaticVFS is read-only and is shipped w/ the viewer
// it has pre-cache data like the UI .TGAs
extern LLVFS	*gStaticVFS;

extern LLMemoryInfo gSysMemory;
extern U64Bytes gMemoryAllocated;

extern std::string gLastVersionChannel;

extern LLVector3 gWindVec;
extern LLVector3 gRelativeWindVec;
extern U32	gPacketsIn;
extern BOOL gPrintMessagesThisFrame;

extern LLUUID gSunTextureID;
extern LLUUID gMoonTextureID;

extern BOOL gRandomizeFramerate;
extern BOOL gPeriodicSlowFrame;

#endif // LL_LLAPPVIEWER_H
