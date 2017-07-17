/** 
 * @file  include/common.h
 * @brief sstoraged common definitions
 * @author Eiji Sugiura <sugiura@isp.co.jp> Research Institute of Systems Planning, Inc
 * @version $Id: common.h 312 2007-09-28 00:56:17Z sugiura $
 */
#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdint.h>
#include <sys/types.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** Status type */
typedef int status_t;

/** Socket type */
/** FileHandle type */
#ifndef HAVE_WINSOCK2_H
typedef int socket_t;
typedef int filehandle_t;
/** Invalid file descriptor */
extern const socket_t INVALID_SOCKET;
extern const filehandle_t INVALID_HANDLE_VALUE;

#else

#define WINVER (0x0501)
#include <winsock2.h>
typedef SOCKET socket_t;
typedef HANDLE filehandle_t;
#endif

/** Default configuration file path*/
#define DEFAULT_CONFIGFILE	"/etc/sstorage/sstoraged.conf"

/** Default PID file path*/
#define DEFAULT_PIDFILE		"/var/run/sstoraged"

/** Default PID file path*/
#define DEFAULT_TARGETPORT	"[0.0.0.0]:3260"

/** MAX. chars per line */
#define MAX_CHAR_PER_LINE	(128)

#ifdef __cplusplus
}
#endif

#include <string>
class SCSI {
public:
	const static size_t LBAUNIT = (1 << 9);
	static uint64_t LBA2OFFSET(uint64_t lba) { return (lba << 9); }
	static uint64_t OFFSET2LBA(uint64_t offset) { return (offset >> 9); }
	static std::string DUMPOCTETS(const uint8_t *ptr);
	static std::string DUMPOCTETS(const uint8_t *ptr, size_t size);
	static std::string DUMPBHS(const uint8_t *ptr);
};

#endif /* __COMMON_H__ */
