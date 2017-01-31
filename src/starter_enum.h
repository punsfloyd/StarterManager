#ifndef _DCMD_HBSRVSTARTER_H_
#define _DCMD_HBSRVSTARTER_H_

#if( __QNX__ )
#include <devctl.h>
#endif // __QNX__

typedef enum
{
   PACKAGE_ENTITY,
   PROCESS_ENTITY,
   INTERFACE_ENTITY,
   DOMAIN_ENTITY,
   INVALID_ENTITY   // termination of enumeration
} eStarterObjType;

typedef enum
{
   PACKAGE_STATE_IDLE, // Package not yet requested
   PACKAGE_STATE_PRE_STARTING, // Package has been triggered
   PACKAGE_STATE_RUN, // Package runs
   PACKAGE_STATE_STOP, // Package stopped
   PACKAGE_STATE_POST_STARTING, // Package starts up
   PACKAGE_STATE_STOPPING, // Package stops down
   PACKAGE_STATE_INVALID // termination of enumeration
} ePackageState;

#define PACKAGE_REQUEST_RUN     PACKAGE_STATE_RUN       // Package run request
#define PACKAGE_REQUEST_STOP    PACKAGE_STATE_STOP      // Package stop request
#define PACKAGE_REQUEST_INVALID PACKAGE_STATE_INVALID   // termination of enumeration

typedef enum
{
   INTERFACE_STATE_IDLE, // Interface Supplier not yet started
   INTERFACE_STATE_SOFT_AVAIL, // Timed interface available because of expired timeout
   INTERFACE_STATE_AVAIL, // Interface available
   INTERFACE_STATE_NOT_AVAIL, // Interface supplying Process started, Interface not avail yet
   INTERFACE_STATE_INVALID // termination of enumeration
} eInterfaceState;

#define INTERFACE_REQUEST_EXPECTED      INTERFACE_STATE_AVAIL       // Interface expected  (supplier process running)
#define INTERFACE_REQUEST_NOT_EXPECTED  INTERFACE_STATE_NOT_AVAIL   // Interface not expected (supplier process not running)
#define INTERFACE_REQUEST_INVALID       INTERFACE_STATE_INVALID     // termination of enumeration

typedef enum
{
   PROCESS_STATE_IDLE, // Process not yet started or stopped and joined
   PROCESS_STATE_PRE_STARTING, // Check required IFs, spawn process
   PROCESS_STATE_POST_STARTING, // Wait for provided IFs
   PROCESS_STATE_RUN, // Process runs, provided interfaces are up now
   PROCESS_STATE_STOPPING, // Process is stopping
   PROCESS_STATE_STOP, // Process stopped
   PROCESS_STATE_SHUTDOWN, // Process has caused shutdown of SrvStarter
   PROCESS_STATE_DONE, // Process has run once
   PROCESS_STATE_WAIT_VARIANT_INFO, // Process is waiting for variantinfo from variant controller
   PROCESS_STATE_DISCARDED, // Process is discarded because of variantinfo
   PROCESS_STATE_INVALID, // termination of enumeration
} eProcessState;

#define PROCESS_REQUEST_RUN     PROCESS_STATE_RUN       // Process run request
#define PROCESS_REQUEST_STOP    PROCESS_STATE_STOP      // Process stop request
#define PROCESS_REQUEST_INVALID PROCESS_STATE_INVALID   // termination of enumeration

typedef enum
{
   START_PARAM_BACKGROUND, // Start Process in Background, normal STDIO
   START_PARAM_BACKGROUND_SILENCE, // Start Process in Background, silent, only STDERR
   START_PARAM_FOREGROUND, // Start Process in Foreground, normal STDIO
   START_PARAM_FOREGROUND_SILENCE, // Start Process in Foreground, silent, only STDERR
   START_PARAM_DAEMON, // Start Daemon Process and block until in daemon mode, normal STDIO
   START_PARAM_DAEMON_SILENCE, // Start Daemon Process and block until in daemon mode, silent, only STDERR
   START_PARAM_DETACHED_DAEMON, // Start Daemon Process detached, normal STDIO
   START_PARAM_DETACHED_DAEMON_SILENCE, // Start Daemon Process detached, silent, only STDERR
   START_PARAM_INVALID // Termination of enumeration
} eStartParam;

typedef enum
{
   ON_TERMINATE_IGNORE_ALL, // In case of termination ignore this, do not restart, do not reset
   ON_TERMINATE_RESTART_NO_ERROR, // In case of termination without error, restart it
   ON_TERMINATE_RESTART_ON_ERROR, // In case of termination with error, restart it
   ON_TERMINATE_RESTART_ALWAYS, // In case of termination always restart it
   ON_TERMINATE_INVALID // Termination of enumeration
} eOnProcTerminate;

typedef enum
{
   FINAL_ACTION_IGNORE, // In case of termination after max. restarts ignore
   FINAL_ACTION_SHUTDOWN_SYSTEM,   // In case of termination after max. restarts reboot system !
   FINAL_ACTION_EXTERNAL_RECOVERY, // In case of termination after max. restarts signalling to recovery manager is done
   FINAL_ACTION_STARTER_RECOVERY,  // In case of termination after max. restarts starter starts emergency process 
   FINAL_ACTION_INVALID // Termination of enumeration
} eFinalAction;

typedef enum
{
   ON_SHUTDOWN_IGNORE, // In case of system shutdown ignore process, let it run
   ON_SHUTDOWN_SIGTERM, // In case of shutdown send SIGTERM to process
   ON_SHUTDOWN_SIGKILL, // In case of shutdown send SIGKILL to process
   ON_SHUTDOWN_START,   // In case of final action as shutdown process tagged with 
   ON_SHUTDOWN_INVALID // Termination of enumeration
} eOnProcShutdown;

typedef enum
{
   ACTION_NO_ACTION,          // No action necessary
   ACTION_CHANGE_WORKING_DIR, // Change current working directory
   ACTION_REDIRECT_STDIO,     // Redirect stdio global
   ACTION_GOTO_BACKGROUND,    // Run srv-starter as daemon
   ACTION_SYSTEM_CALL,        // Executes system call's like ln, ...
   ACTION_LINK,               // link api call
   ACTION_SYMLINK,            // symlink api call
   ACTION_MOUNT,              // mount api call
   ACTION_UNLINK,             // unlink api call
   ACTION_MKDIR,              // create a directory
   ACTION_IFCONFIG,           // configure network interface
   ACTION_PATHMGR_SYMLINK,    // pathmgr_symlink api call
   ACTION_TRIGGERSTAGE,       // trigger file loading by ziploader
   ACTION_WRITE,              // write data to the specified device
   ACTION_INVALID             // Termination of enumeration
} eActions;

typedef enum
{
  NET_PRESENT,
  NET_UP,
  NET_CONFIGURED,
  NET_INVALID
} eNetState;

typedef enum
{
  INTERFACE_TYPE_FILE,
  INTERFACE_TYPE_DEVICE,
  INTERFACE_TYPE_NETWORK,
  INTERFACE_TYPE_INVALID
} eInterfaceType;

#define EXIT_CODE_SPAWN_ERROR -2

#endif //_DCMD_HBSRVSTARTER_H_
