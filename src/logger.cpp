/**
   @file logger.cpp
   @brief Logger with Log4CXX 
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: logger.cpp 313 2007-09-28 01:35:55Z sugiura $
 */

#include <stdexcept>
#include <iostream>
#include <log4cxx/logger.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/fileappender.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/logmanager.h>
#include "logger.h"
#include "configurator.h"

#ifdef HAVE_LIBLOG4CXX
/** logger instance */
LoggerPtr logger(Logger::getLogger("sstoraged"));
int log_level;

static void _initLogger(const string &logfile, const string &loglevel, const bool debug)
{
	try {
		LayoutPtr layout(new PatternLayout
 				 ("%d{%a, %d %b %Y %H:%M:%S %z} %p %c %x - %m%n"));
		helpers::Pool pool;

		if(debug){
			ConsoleAppenderPtr console(new ConsoleAppender());
			console->setTarget(LOG4CXX_STR("System.out"));
			console->setLayout(layout);
			console->activateOptions(pool);
			Logger::getRootLogger()->addAppender(console);
		}

		FileAppenderPtr file(new FileAppender());
		file->setFile(logfile, true, true, 1024, pool);
		file->setLayout(layout);
		Logger::getRootLogger()->addAppender(file);

		LogManager::getLoggerRepository()->setConfigured(true);
		logger->setLevel(Level::toLevel(loglevel));
	} catch (const helpers::Exception &e) {
		throw std::runtime_error(e.what());
	} catch (...) {
		throw std::runtime_error("Unknown exception");
	}
}
void FinalizeLogger() {}

#else // !HAVE_LOG4CXXLIB

ofstream logger;
static void _initLogger(const string &logfile, const string &loglevel, const bool debug)
{
	if(debug){
		logger = cout;
	} else {
		logger.open(logfile.c_str(), ios::out|ios::app);
	}
}
void FinalizeLogger() { logger.close(); }

#endif // HAVE_LIBLOG4CXX

bool InitLogger(const bool debug)
{
	string logfile = "";
	try {
		Configurator &config = Configurator::getInstance();
		config.getAttr("LogFile", logfile);
		string loglevel;
		config.getAttr("LogLevel", loglevel);
		_initLogger(logfile, loglevel, debug);
		LOG4CXX_INFO(logger, "LogLevel : " + loglevel);
		if(loglevel == "debug")
			log_level = LOG_DEBUG;
		else if(loglevel == "info")
			log_level = LOG_INFO;
		else if(loglevel == "notice")
			log_level = LOG_NOTICE;
		else if(loglevel == "warning")
			log_level = LOG_WARNING;
		else if(loglevel == "error")
			log_level = LOG_ERR;
		else {
			cerr << "Invalid log level : " << loglevel << endl;
			return false;
		}

	} catch (const std::exception &e) {
 		cerr << "Failed to open log file : " << logfile << endl;
		cerr << e.what() << endl;
		return false;
	}
	return true;
}
