/**
   @file src/main.cpp
   @brief sstoraged main rouine
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: main.cpp 313 2007-09-28 01:35:55Z sugiura $
 */

#include <stdlib.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <functional>
#include <boost/ref.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include "common.h"

#ifdef HAVE_BOOST_IOSTREAMS
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
using namespace boost::iostreams;
#endif

#include "logger.h"
#include "configurator.h"
#include "configbinder.h"
#include "targetportconfigurator.h"
#include "inetutils.h"
#include "port.h"
#include "portlistener.h"
#include "signallistener.h"
#include "manager.h"
#include "acceptor.h"

extern status_t parseArgs(int argc, char *argv[]);

static bool readConfig()
{
	Configurator &config = Configurator::getInstance();
	try {
		string configfile_path = "";
		if(!config.getAttr("ConfigFile", configfile_path)){
			cerr << "ConfigFile attribute not found!" << endl;
			return false;
		}
		ConfigBinder *binder = new ConfigBinder();
		binder->append(configfile_path);
		if(!binder->validate()){
			cerr << "Invalid configuration!" << endl;
			return false;
		}
		delete binder;
	} catch (const std::exception &e) {
		cerr << e.what() << endl;
		cerr << "Invalid configuration!" << endl;
		return false;
	}
	return true;
}

static bool openPorts(iSCSIAcceptor &iscsi_acceptor)
{
	PortListenerPtr iscsi_listener = PortListenerPtr(new PortListener);
	/** @todo : support multiple TargetPort */
	TargetPortConfigurator portconfs;
	if(portconfs.empty()){
		LOG4CXX_ERROR(logger, "TargetPort array is empty!");
		return false;
	}
	iSCSIManager &manager = iSCSIManager::getInstance();
	TargetPortConfigurator::iterator itr = portconfs.begin();
	try {
		for(; itr != portconfs.end(); ++itr){
			sockaddr_storage target_addr = {};
			InetUtil::getSockAddr(itr->get(), target_addr);
			/** Initialize iSCSI listener */
			PortPtr iscsi_port = PortPtr(new ListenPort(target_addr));
			iscsi_port->setReceiver(boost::bind(&iSCSIAcceptor::accept,&iscsi_acceptor,_1));
			iscsi_listener->addPort(iscsi_port);
			manager.setListener(iscsi_listener);
			LOG4CXX_INFO(logger, "Listening iSCSI port " 
				     + InetUtil::getAddrPortStr(target_addr)
				     + " : " + itr->get());
		}
	} catch (const std::exception &e) {
		LOG4CXX_ERROR(logger, "Failed to open iSCSI port : " + itr->get()
			      + " " + e.what());
		return false;
	}
	return true;
}

/**
   sstoraged main routine
   @param argc : argument counter
   @param argv : argument value array
   @return EXIT_SUCCESS
   @return EXIT_FAILURE
 */
int main(int argc, char *argv[])
{
	/** Setup configuration defaults
	    Parse command line arguments */
	try {
		parseArgs(argc, argv);
	} catch (const std::exception &e) {
		cerr << e.what() << endl;
		return EXIT_FAILURE;
	}

	/** Parse configuration file */
	if(!readConfig())
		return EXIT_FAILURE;
	Configurator &config = Configurator::getInstance();
	bool debug = true;
	config.getAttr("Debug", debug);

#ifdef HAVE_LOG4CXX_LOGGER_H
	NDC::push("main");
#endif

	/** Initialize Logger */
	if(!InitLogger(debug))
		return EXIT_FAILURE;

	/** Open sockets */
	iSCSIAcceptor iscsi_acceptor;
	if(!openPorts(iscsi_acceptor))
		return EXIT_FAILURE;

	LOG4CXX_INFO(logger, "Entering application.");

	/** Daemonize */
#ifdef HAVE_DAEMON
	if(!debug && daemon(1,0) < 0){
		LOG4CXX_ERROR(logger, "Failed to daemonize");
		return EXIT_FAILURE;
	}
#else
#warning TODO support service mode for WIN32
#endif

	/** Start Signal Listener */
	boost::thread_group threads;
	SignalListener &sig_listener = SignalListener::getInstance();
 	threads.create_thread(boost::ref(sig_listener));

#ifdef HAVE_DAEMON
	/** Create PID file */
	string pidfile = "";
	try {
		config.getAttr("PIDfile", pidfile);
		filehandle_t pidfd = open(pidfile.c_str(), O_WRONLY|O_CREAT|O_EXCL,				 
					  S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		if(pidfd == INVALID_HANDLE_VALUE)
			throw runtime_error(strerror(errno));
		{
#ifdef HAVE_BOOST_IOSTREAMS
		stream<file_descriptor_sink> ofs(pidfd);
		ofs << getpid();
		ofs.flush();
		ofs.close();
#else
		char buf[8];
		snprintf(buf, sizeof(buf), "%d", getpid());
		if(write(pidfd, buf, strlen(buf)) != strlen(buf)) {
			LOG4CXX_ERROR(logger, "Failed to write PID file");
		}
		close(pidfd);
#endif
		}
	} catch (const std::exception &e) {
		LOG4CXX_ERROR(logger, "Failed to open PID file : " + pidfile);
		LOG4CXX_ERROR(logger, e.what());
		return EXIT_FAILURE;
	}
#else
#warning TODO support service mode for WIN32
#endif

	/** @todo : Start Coordinator */

	/** Enter to iSCSI Listener Loop */
	iSCSIManager &manager = iSCSIManager::getInstance();
 	try {
 		(*manager.getListener())();
 	} catch(const std::exception &e) {
		LOG4CXX_ERROR(logger, e.what());
 		return EXIT_FAILURE;
 	}

	/** Close iSCSI listener */
	manager.clearListener();

	/** Stop Signal Listener / Cordinator */
	sig_listener.exit();
	threads.join_all();

	sig_listener.clearInstance();

	config.clear();

#ifdef HAVE_DAEMON
	/** Delete PID file */
	unlink(pidfile.c_str());
#else
#warning TODO support service mode for WIN32
#endif

	LOG4CXX_INFO(logger, "Exiting application.");
#ifdef HAVE_LOG4CXX_LOGGER_H
	NDC::pop();
 	NDC::remove();
#endif
	FinalizeLogger();

	return EXIT_SUCCESS;
}
