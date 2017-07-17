/**
   @file connection.h
   @brief Connection handling classes
   @author Eiji Sugiura <eiji.sugiura@gmail.com> Research Institute of Systems Planning, Inc
   @version $Id: connection.h 140 2007-06-14 06:15:02Z sugiura $
 */
#ifndef __ISCSI_CONNECTION_H__
#define __ISCSI_CONNECTION_H__

#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include "common.h"
#include "logger.h"
#include "port.h"
#include "counter.h"
#include "ioveccontainer.h"

class Listener;

/** Connection class */
class Connection {
public:
 	Connection() : port() {}
	Connection(const PortPtr accepted) : port(accepted) {}
 	virtual ~Connection() {}

	socket_t getSocket() const { return port->getSocket(); }
	PortPtr getPort() const { return port; }

protected:
	PortPtr port;
};

class PortListener;
class iSCSIReactor;
class iSCSIConnection : public Connection {
public:
	iSCSIConnection(const PortPtr accepted,
			boost::shared_ptr<PortListener> actor);
	~iSCSIConnection();

	void changeListener(boost::shared_ptr<iSCSIReactor> actor);
	void clearListener();

	uint16_t getCID() const { return cid; }
	void setCID(uint16_t aCID) { cid = aCID; }

	/** Status SN updater/getter */
	uint32_t getSTATSN() { return statsn_gen.cur(); }
	void advanceSTATSN() { statsn_gen(); }
	void setSTATSN(const uint32_t sn) { statsn_gen.set(sn); }

	void waitWritable();
	void unwaitWritable();

	IovecContainer & getRecvBuf() { return recvbuf; }
private:
	/** Connection ID */
	uint16_t cid;

	/** Status SN */
	SequenceCounter statsn_gen;

	/** Listener */
	boost::shared_ptr<PortListener> listener;

	/** Receive buffer */
	IovecContainer recvbuf;
};

typedef boost::shared_ptr<iSCSIConnection> iSCSIConnectionPtr;

#endif /* __ISCSI_CONNECTION_H__ */
