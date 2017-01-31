#ifndef _SRVSTARTEREXPORT_HPP_
#define _SRVSTARTEREXPORT_HPP_
/**********************************************************************
 *  Project      Harman Car Multimedia System
 *  (c) copyright 2006
 *  Company      Harman/Becker Automotive Systems GmbH
 *  All rights reserved
 **********************************************************************/
/**
 * @file         starter_dcmd.hpp
 * @ingroup      OSCAR
 * @author       Thomas Fiedler
 * @date         2009-04-24
 *
 * @brief        POSIX client interface of starter
 */

//--------------------------------------------------------------------
//                              Includes
//--------------------------------------------------------------------


//#if( __QNX__ )
   #include <inttypes.h>
   #include <devctl.h>
   #define Int8                 int8_t
   #define UInt8               uint8_t
   #define Int16               int16_t
   #define UInt16             uint16_t
   #define Int32               int32_t
   #define UInt32             uint32_t
   #define UInt64             uint64_t
//#else
//   #include "api/sys/colibry/pf/base/src/HBTypes.h"
//#endif // #if( __QNX__ )

#include "starter_enum.h"

//--------------------------------------------------------------------
//                         Namespace Declaration
//--------------------------------------------------------------------
#ifdef __cplusplus
namespace GStartupConfig
{
#endif // #ifdef __cplusplus

// srv-starter version
#define CORE_VERSION_MAJOR      5   // Major changes of IF - not compatible to former version
#define CORE_VERSION_MINOR      17  // Minor changes of IF - still compatible to former version

#define MAX_STARTEROBJ_NAME     64

#ifdef __cplusplus
   const Int32 core_version_major = CORE_VERSION_MAJOR;
   const Int32 core_version_minor = CORE_VERSION_MINOR;
#endif // #ifdef __cplusplus

/**
 * @brief   Mountpoint definitions
 */
#define MOUNTPOINT_START         "/dev/starter/start"
#define MOUNTPOINT_STATE         "/dev/starter/status"
#define MOUNTPOINT_VERSION       "/dev/starter/version"
#define MOUNTPOINT_VARIANT       "/dev/starter/variant"
#define MOUNTPOINT_CONFIG        "/dev/starter/config"
#define MOUNTPOINT_WATCHDOG      "/dev/starter/watchdog"
#define MOUNTPOINT_PROBLEMREPORT_PROCESSES  "/dev/starter/processproblems"
#define MOUNTPOINT_PROBLEMREPORT_INTERFACES "/dev/starter/pendingifs"

#ifdef __cplusplus
   const char * const mountpoint_start    = MOUNTPOINT_START;
   const char * const mountpoint_state    = MOUNTPOINT_STATE;
   const char * const mountpoint_version  = MOUNTPOINT_VERSION;
   const char * const mountpoint_variant  = MOUNTPOINT_VARIANT;
   const char * const mountpoint_conifg   = MOUNTPOINT_CONFIG; // TODO: delete with next incomp. interface change
   const char * const mountpoint_config   = MOUNTPOINT_CONFIG;
   const char * const mountpoint_watchdog = MOUNTPOINT_WATCHDOG;
   const char * const mountpoint_problemreport_processes = MOUNTPOINT_PROBLEMREPORT_PROCESSES;
   const char * const mountpoint_problemreport_interfaces = MOUNTPOINT_PROBLEMREPORT_INTERFACES;
#endif //#ifdef __cplusplus

typedef struct
{
   Int32 coreMajorVersion;         // min. major version allowed by SrvStarter parser -> dynamic config version in XML.cfg
   Int32 coreMinorVersion;         // min. minor version allowed by SrvStarter parser -> dynamic config version in XML.cfg
   Int32 configMajorVersion;       // dynamic config major version in XML.cfg (header) -> TaSC output XML version
   Int32 configMinorVersion;       // dynamic config minor version in XML.cfg (header) -> TaSC output XML version
   Int32 coreBuildVersion;         // SrvStarter binary version
}  tVersionInfo;

// Version Info Data Definition, for use with DEVCTL_STARTER_QUERY_VERSION2
// This struct is used for POSIX interface /dev/starter/version and must be stable
// Client should be first query this info from SrvStarter.
typedef struct
{
   Int32 cfgVerFormat;   // Format version of cfg file
   Int32 cfgVerMajor;    // Major version of cfg file
   Int32 cfgVerMinor;    // Minor version of cfg file
}  tCfgVersion;

// Version Info Data Definition, for use with DEVCTL_STARTER_QUERY_VERSION2
// This struct is used for POSIX interface /dev/starter/version and must be stable
// Client should be first query this info from SrvStarter.
typedef struct
{
   Int32 itfVerMajor;   // Core major version of srv-starter
   Int32 itfVerMinor;   // Core minor version of srv-starter
   Int32 itfVerBuild;   // Build no. of srv-starter
}  tItfVersion;

// Run time variant code must be provided by the variant controller as
// a bitvector. Variant code is configurable and provided ty cfg file.
typedef struct
{
   UInt32 variantCode;  // The runtime variant code
}  tVariantInfo;

// -----------------------------------------------------------------------------
// Query starter object state at runtime (devctl)
// -----------------------------------------------------------------------------
typedef struct
{
   Int32 domainId;
   Int32 objId;
}  tStarterObjStateRequest;

typedef struct
{
   Int32 requestedState;
   Int32 currentState;
}  tStarterObjStateResponse;

typedef union
{
   tStarterObjStateRequest  stateRequest;
   tStarterObjStateResponse stateResponse;
}  tStarterObjStateQuery;

// -----------------------------------------------------------------------------
// Interface to query application domains (devctl)
// -----------------------------------------------------------------------------
typedef struct
{
   Int32 domainIndex;                     // index of requested starter object
}  tDomainRequest;

typedef struct
{
   Int32 domainId;                        // key value of app domain
   char  domainName[MAX_STARTEROBJ_NAME]; // name of the requested starter object
}  tDomainResponse;

typedef union
{
   tDomainRequest  domainRequest;   // tx: starter object request
   tDomainResponse domainResponse;  // rx: starter object response
}  tDomainQuery;

// -----------------------------------------------------------------------------
// Interface to query starter object names (devctl)
// -----------------------------------------------------------------------------
typedef struct
{
   Int32 domainId;                     // filter for app domain
   Int32 objIndex;                     // index of requested starter object
}  tStarterObjRequest;

typedef struct
{
   Int32 domainId;                     // key value of app domain
   Int32 objId;                        // key value of the requested starter object
   char  objName[MAX_STARTEROBJ_NAME]; // name of the requested starter object
}  tStarterObjResponse;

typedef union
{
   tStarterObjRequest  request;   // tx: starter object request
   tStarterObjResponse response;  // rx: starter object response
}  tStarterObjQuery;

// -----------------------------------------------------------------------------
// Observer interface for starter obj state changes (read)
// -----------------------------------------------------------------------------
typedef struct
{
   eStarterObjType objType;       // starter object type
   Int32 domainId;                // key value of the app domain
   Int32 objId;                   // key value of the starter object
   Int32 objState;                // state of the starter object
}  tStarterObjState;

// -----------------------------------------------------------------------------
// Interface to query starterobjects blockwise
// -----------------------------------------------------------------------------
typedef struct
{
   eStarterObjType type;              // starter object type
   Int32 domainId;                    // key value of the app domain, empty if domaintype
   Int32 objId;                       // key value of the starter object
   char objName[MAX_STARTEROBJ_NAME]; // name of the object
}  tStarterObjKeyInfo;

// -----------------------------------------------------------------------------
// Package command interface (devctl)
// -----------------------------------------------------------------------------
typedef struct
{
   Int32 domainId;
   Int32 packageId;
   Int32 requestedState;
}  tPackageCmdRequest;

// -----------------------------------------------------------------------------
// Watchdog error message (devctl)
// -----------------------------------------------------------------------------
typedef struct
{
   Int32 pid;
   Int32 tid;
   Int32 severity;
   Int32 code;
}  tWatchdogError;

// -----------------------------------------------------------------------------
// Process info (devctl)
// -----------------------------------------------------------------------------
typedef struct
{
   Int32 pid;
}  tProcessInfoRequest;

typedef struct
{
   Int32 oid;             // HIWORD domainId, LOWWORD objId
   Int32 requestedState;
   Int32 currentState;
}  tProcessInfoResponse;

typedef union
{
   tProcessInfoRequest  request;   // tx: starter object request
   tProcessInfoResponse response;  // rx: starter object response
}  tProcessInfoQuery;

// -----------------------------------------------------------------------------
// APS to background partition (devctl)
// -----------------------------------------------------------------------------
#define TO_BACKGROUND_PARTITION 1
#define TO_HOME_PARTITION       2

typedef struct
{
   Int32 tid;
   Int32 direction;
   Int32 timeout;
}  tThread2BgRequest;

typedef struct
{
   Int32 state;             
}  tThread2BgResponse;

typedef union
{
   tThread2BgRequest  request;   // tx: starter object request
   tThread2BgResponse response;  // rx: starter object response
}  tThread2BgQuery;


#ifndef __cplusplus
   #define hbsrvstr_version_t                   tVersionInfo
   #define hbsrvstr_itf_version_t               tItfVersion
   #define hbsrvstr_cfg_version_t               tCfgVersion
   #define hbsrvstr_variant_info_t              tVariantInfo
   #define hbsrvstr_domain_query_t              tDomainQuery
   #define hbsrvstr_starterobj_query_t          tStarterObjQuery
   #define hbsrvstr_starterobj_state_query_t    tStarterObjStateQuery
   #define hbsrvstr_package_cmd_request_t       tPackageCmdRequest
   #define hbsrvstr_process_info_query_t        tProcessInfoQuery
   #define hbsrvstr_thread2bg_query_t           tThread2BgQuery
   #define hbsrvstr_watchdog_error_t            tWatchdogError
#else
   #define hbsrvstr_version_t                   GStartupConfig::tVersionInfo
   #define hbsrvstr_itf_version_t               GStartupConfig::tItfVersion
   #define hbsrvstr_cfg_version_t               GStartupConfig::tCfgVersion
   #define hbsrvstr_variant_info_t              GStartupConfig::tVariantInfo
   #define hbsrvstr_domain_query_t              GStartupConfig::tDomainQuery
   #define hbsrvstr_starterobj_query_t          GStartupConfig::tStarterObjQuery
   #define hbsrvstr_starterobj_state_query_t    GStartupConfig::tStarterObjStateQuery
   #define hbsrvstr_package_cmd_request_t       GStartupConfig::tPackageCmdRequest
   #define hbsrvstr_process_info_query_t        GStartupConfig::tProcessInfoQuery
   #define hbsrvstr_thread2bg_query_t           GStartupConfig::tThread2BgQuery
   #define hbsrvstr_watchdog_error_t            GStartupConfig::tWatchdogError
#endif // #ifndef __cplusplus

#if( __QNX__ )
   #define DCMD_HBSRVSTR_ITF_VERSION        __DIOF(_DCMD_MISC,  1,  hbsrvstr_itf_version_t)
   #define DCMD_HBSRVSTR_CFG_VERSION        __DIOF(_DCMD_MISC,  2,  hbsrvstr_cfg_version_t)
   #define DCMD_HBSRVSTR_VARIANT_INFO       __DIOT(_DCMD_MISC,  3,  hbsrvstr_variant_info_t)
   #define DCMD_HBSRVSTR_DOMAIN_COUNT       __DIOF(_DCMD_MISC,  4,  UInt32)
   #define DCMD_HBSRVSTR_DOMAIN_ENTITY      __DIOTF(_DCMD_MISC, 5,  hbsrvstr_domain_query_t)
   #define DCMD_HBSRVSTR_PACKAGE_COUNT      __DIOF(_DCMD_MISC,  6,  UInt32)
   #define DCMD_HBSRVSTR_PACKAGE_ENTITY     __DIOTF(_DCMD_MISC, 7,  hbsrvstr_starterobj_query_t)
   #define DCMD_HBSRVSTR_PACKAGE_STATE      __DIOTF(_DCMD_MISC, 8,  hbsrvstr_starterobj_state_query_t)
   #define DCMD_HBSRVSTR_PROCESS_COUNT      __DIOF(_DCMD_MISC,  9,  UInt32)
   #define DCMD_HBSRVSTR_PROCESS_ENTITY     __DIOTF(_DCMD_MISC, 10, hbsrvstr_starterobj_query_t)
   #define DCMD_HBSRVSTR_PROCESS_STATE      __DIOTF(_DCMD_MISC, 11, hbsrvstr_starterobj_state_query_t)
   #define DCMD_HBSRVSTR_INTERFACE_COUNT    __DIOF(_DCMD_MISC,  12, UInt32)
   #define DCMD_HBSRVSTR_INTERFACE_ENTITY   __DIOTF(_DCMD_MISC, 13, hbsrvstr_starterobj_query_t)
   #define DCMD_HBSRVSTR_INTERFACE_STATE    __DIOTF(_DCMD_MISC, 14, hbsrvstr_starterobj_state_query_t)
   #define DCMD_HBSRVSTR_PACKAGE_COMMAND    __DIOT(_DCMD_MISC,  15, hbsrvstr_package_cmd_request_t)
   #define DCMD_HBSRVSTR_WATCHDOG_ERROR     __DIOT(_DCMD_MISC,  16, hbsrvstr_watchdog_error_t)
   #define DCMD_HBSRVSTR_PROCESS_INFO       __DIOTF(_DCMD_MISC, 17, hbsrvstr_process_info_query_t)
   #define DCMD_HBSRVSTR_STARTER_THREAD2BG  __DIOTF(_DCMD_MISC, 18, hbsrvstr_thread2bg_query_t)
#else // #if( __QNX__ )
   #define DCMD_HBSRVSTR_ITF_VERSION        1
   #define DCMD_HBSRVSTR_CFG_VERSION        2
   #define DCMD_HBSRVSTR_VARIANT_INFO       3
   #define DCMD_HBSRVSTR_DOMAIN_COUNT       4
   #define DCMD_HBSRVSTR_DOMAIN_ENTITY      5
   #define DCMD_HBSRVSTR_PACKAGE_COUNT      6
   #define DCMD_HBSRVSTR_PACKAGE_ENTITY     7
   #define DCMD_HBSRVSTR_PACKAGE_STATE      8
   #define DCMD_HBSRVSTR_PROCESS_COUNT      9
   #define DCMD_HBSRVSTR_PROCESS_ENTITY     10
   #define DCMD_HBSRVSTR_PROCESS_STATE      11
   #define DCMD_HBSRVSTR_INTERFACE_COUNT    12
   #define DCMD_HBSRVSTR_INTERFACE_ENTITY   13
   #define DCMD_HBSRVSTR_INTERFACE_STATE    14
   #define DCMD_HBSRVSTR_PACKAGE_COMMAND    15
   #define DCMD_HBSRVSTR_WATCHDOG_ERROR     16
   #define DCMD_HBSRVSTR_PROCESS_INFO       17
   #define DCMD_HBSRVSTR_STARTER_THREAD2BG  18
#endif // #if( __QNX__ )

#ifdef __cplusplus
}
#endif // #ifdef __cplusplus

#endif // #ifndef _SRVSTARTEREXPORT_HPP_
