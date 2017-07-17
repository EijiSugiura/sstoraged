/** 
 * @file  logger.h
 * @brief logger definitions
 * @author Eiji Sugiura <sugiura@isp.co.jp> Research Institute of Systems Planning, Inc
 * @version $Id: logger.h 313 2007-09-28 01:35:55Z sugiura $
 */
#ifndef __LOGGER_H__
#define __LOGGER_H__

#include "common.h"
#include <string>

#ifdef HAVE_SYSLOG_H
#include <syslog.h>
#else	// !HAVE_SYSLOG_H
#define LOG_EMERG       0       /* system is unusable */
#define LOG_ALERT       1       /* action must be taken immediately */
#define LOG_CRIT        2       /* critical conditions */
#define LOG_ERR         3       /* error conditions */
#define LOG_WARNING     4       /* warning conditions */
#define LOG_NOTICE      5       /* normal but significant condition */
#define LOG_INFO        6       /* informational */
#define LOG_DEBUG       7       /* debug-level messages */
#endif	// HAVE_SYSLOG_H

#ifdef HAVE_LIBLOG4CXX

#include <log4cxx/logger.h>
#include <log4cxx/ndc.h>
using namespace log4cxx;
/** The logger */
extern LoggerPtr logger;
#else

#include <fstream>
/** The logger */
extern std::ofstream logger;

#define LOG4CXX_DEBUG(logger,log)	\
	if(log_level == LOG_DEBUG) {	\
		logger << log << endl;	\
	}
#define LOG4CXX_INFO(logger,log)	\
	if(log_level >= LOG_INFO) {	\
		logger << log << endl;	\
	}
#define LOG4CXX_NOTICE(logger,log)	\
	if(log_level >= LOG_NOTICE) {	\
		logger << log << endl;	\
	}
#define LOG4CXX_WARN(logger,log)	\
	if(log_level >= LOG_WARNING) {	\
		logger << log << endl;	\
	}
#define LOG4CXX_ERROR(logger,log)	\
	if(log_level >= LOG_ERR) {	\
		logger << log << endl;	\
	}

#endif

using namespace std;

/** Logger initializer
    @param logfile : path to the log file
    @param loglevel : log level string
    @param debug : debug mode flag
 */
extern bool InitLogger(const bool debug);
extern void FinalizeLogger();
extern int log_level;

#endif /* __LOGGER_H__ */
