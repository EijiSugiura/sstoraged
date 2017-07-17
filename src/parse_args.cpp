/**
   @file parse_args.cpp
   @brief Command line arguments parser
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: parse_args.cpp 29 2007-02-27 05:29:56Z sugiura $
 */

#include <getopt.h>
#include <iostream>
#include "config.h"
#include "common.h"
#include "configurator.h"

static void printVersion()
{
	cout << PACKAGE_STRING << endl;
}

static void printUsage()
{
	cout << "Usage: " << PACKAGE_NAME <<
		" [-dhv] [-c conf_file] [-p pid_file]" << endl;
}

/**
   Command Line Arguments parser
   @param argc : argument counter
   @param argv : argument value list
   @return 0 : success
   @return -EINVAL : failure
*/
status_t parseArgs(int argc, char *argv[])
{
	int c;
	struct option long_options[] = {
		{"config", 	1, 0, 'c'},
		{"debug",	0, 0, 'd'},
		{"help",	0, 0, 'h'},
		{"pid",		1, 0, 'p'},
		{"version",	0, 0, 'v'},
		{0, 0, 0, 0}
	};
	Configurator &config = Configurator::getInstance();
	while(1) {
		c = getopt_long (argc, argv, "c:dhp:v", long_options, NULL);
		if(c < 0)
			break;
		switch(c) {
		case 'c':
			config.setAttr("ConfigFile", static_cast<string>(optarg));
			break;
		case 'd':
			config.setAttr("Debug", true);
			break;
		case 'h':
			printUsage();
			exit(EXIT_SUCCESS);
		case 'p':
			config.setAttr("PIDfile", static_cast<string>(optarg));
			break;
		case 'v':
			printVersion();
			exit(EXIT_SUCCESS);
		default: 
			return -EINVAL;
		}
	}
	return 0;
}
