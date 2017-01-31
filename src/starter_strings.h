#ifndef _HBSRVSTARTER_STRINGS_H_
#define _HBSRVSTARTER_STRINGS_H_

// NOTE: This header can only be included once per application
//       or you will get compile errors

char* sStarterObjType[] = { "PACKAGE_ENTITY",
                            "PROCESS_ENTITY",
                            "INTERFACE_ENTITY"};

// Strings to match states defined by eStartParam
char *sStartParam[] = { "BACKGROUND",
                        "BACKGROUND (silent)",
                        "FOREGROUND",
                        "FOREGROUND (silent)",
                        "DAEMON MODE",
                        "DAEMON MODE (silent)",
                        "DETACHED DAEMON MODE",
                        "DETACHED DAEMON MODE (silent)"};

// Strings to match states defined by eOnProcTerminate
char *sOnTerminate[] = {"IGNORE ALL",
                        "RESTART WHEN NO ERROR",
                        "RESTART ON ERROR",
                        "RESTART ALWAYS"};

// Strings to match states defined by eOnProcTerminate
char *sFinalAction[] = {"FINAL ACTION IGNORE",
                        "FINAL ACTION SHUTDOWN SYSTEM",
                        "FINAL ACTION EXTERNAL RECOVERY",
                        "FINAL ACTION STARTER RECOVERY"};

// Strings to match states defined by eOnProcShutdown
char *sOnShutdown[] = {"IGNORE (let run)",
                       "send SIGTERM",
                       "send SIGKILL",
                       "start emergency process"};

// Strings to match states defined by eActions
char *sActions[] = {"NONE",
                    "CHANGE WORKING DIRECTORY",
                    "REDIRECT STDIO",
                    "GOTO BACKGROUND",
                    "SYSTEM CALL",
                    "LINK",
                    "SYMLINK",
                    "MOUNT",
                    "UNLINK",
                    "MKDIR",
                    "IFCONFIG",
                    "PATHMGR_SYMLINK",
                    "TRIGGERSTAGE",      
                    "WRITE",         
                    "INVALID"};

// Strings to match states defined by ePackageState
char *sPackageState[] = {"IDLE",          // Package not yet requested
                         "PRE_STARTING",  // Package has been triggered
                         "RUN",           // Package is running, interfaces valid
                         "STOP",          // Package stopped
                         "POST_STARTING", // Package starts up
                         "STOPPING",      // Package shuts down
                         "INVALID"};      // termination of enumeration


// Strings to match states defined by eInterfaceState
char *sInterfaceState[] = {"IDLE",                     // Interface Supplier not yet started
                        	"SOFT_AVAIL",               // Pretend interface is available after availability timeout has expired !
                           "AVAIL",                    // Interface available !
                           "NOT_AVAIL",                // Interface supplying Process started, Interface not avail yet
                           "INVALID"};                 // termination of enumeration

// Strings to match states defined by eProcessState
char *sProcessState[] = {  "IDLE",                       // Process not yet started or stopped and joined
                           "PRE_STARTING",               // Check required IFs, spawn process
                           "POST_STARTING",              // Wait for provided IFs
                           "RUN",                        // Process runs, provided interfaces are up now
                           "STOPPING",                   // Process is stopping
                           "STOP",                       // Process stopped
                           "SHUTDOWN",                   // Process has caused shutdown of SrvStarter
                           "DONE",                       // Process has run once
                           "WAIT_VARIANT_INFO",          // Process is waiting for variantinfo from variant controller
                           "DISCARDED",                  // Process is discarded because of variantinfo
                           "INVALID"};                   // termination of enumeration
                           
// Strings to match states defined by eInterfaceType                          
 char  *sInterfaceType[] = { "FILE",                     //File Type 
                             "DEVICE",                   //Device Type
                             "NETWORK",                  //Network Type
                             "INVALID"};                 //Invalid Type

// Strings to match states defined by eNetState                
char  *sNetState[] = {  "PRESENT",                      //when Network Manager reports Network Interface Present  Under Qnx, the if_up utility uses the same condition if_up -p <ip_addr> 
                        "UP",                           //when Network Manager reports physical link up .Under Qnx, the if_up utility uses the same condition if_up -l <ip_addr> 
                        "CONFIGURED",                   //when Network Manager reports Network interface configured .   
                        "INVALID"};                     //Invalid State     

#endif //_HBSRVSTARTER_STRINGS_H_
