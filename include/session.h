/**
   @file session.h
   @brief Session handling classes
   @author Eiji Sugiura <eiji.sugiura@gmail.com> Research Institute of Systems Planning, Inc
   @version $Id: session.h 312 2007-09-28 00:56:17Z sugiura $
 */
#ifndef __ISCSI_SESSION_H__
#define __ISCSI_SESSION_H__

#include "common.h"

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif

#include <stdexcept>
#include <list>
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>
#include "logger.h"
#include "port.h"
#include "task.h"
#include "info.h"
#include "connection.h"
#include "initiator.h"
#include "iscsi/protocol.h"

using namespace std;

class WriteSegment;
class WriteCache;
class Limitter;
/** iSCSI Session class */
class iSCSISession {
public:
	/** Default constructor */
 	iSCSISession();
	/** Destructor */
 	virtual ~iSCSISession();

 	status_t recv(const socket_t fd);
 	status_t recv(boost::shared_ptr<iSCSISession> session,
		      const socket_t fd);
 	status_t send(const socket_t fd);

	/** belows are initialized in constructor, with configuration file */
	template<typename T>
	void setAttr(const string &attr, const T &value) { attrs[attr] = value;	}
	template<typename T>
	bool getAttr(const string &attr, T &value);

	/** Session Type class */
	class SessionType {
	public:
		SessionType() : type(iSCSI_LOGIN_SESSION) {}
		int get() const { return type; }
		string getStr() const;
		void set(const string& str);
	private:
		/** Session Type : Login/Discovery/Normal */
		int type;
	} type;
	/** status shift Login -> Discovery */
	bool shift2Discovery();
	/** status shift Login -> Normal */
	bool shift2Normal();

	/** Portal Groupt Tag class */
	class PortalGroupTag {
	public:
		/** Default Constructor
		    tag is always = 1
		 */
		PortalGroupTag() : tag(1) {}
		/** Target Portal Group Tag getter 
		    @see : RFC3720 Appendix D.  SendTargets Operation
		    @todo : support multiple TPGT */
		uint16_t get() const { return tag; }
		/** Target Portal Group Tag setter 
		    @todo : support multiple TPGT */
		void set(const uint16_t ttag) { tag = ttag; }
	private:
		/** Target Portal Tag */
		uint16_t tag;
	} tag;

	/** Session ID class */
	class SID {
	public:
		/** Default constructor */
		SID() : isid() {}
		/** SessionID getter */
		SessionID get() const { return isid; }
		/** SessionID setter
		    @param sid : session id to set
		 */
		void set(const SessionID &sid) { isid = sid; }
	private:
		/** Session ID */
		SessionID isid;
	} sid;

	/** Target Idnetifying Handle class */
	class IdentifyingHandle{
	public:
		/** Default Constructor */
		IdentifyingHandle() : tsih(0) {}
		/** TSIH getter */
		uint16_t get() const { return tsih; }
		/** TSIH setter
		    @param aTSIG : tsih to set
		 */
		void set(uint16_t aTSIH) { tsih = aTSIH; }
	private:
		/** Target Session Identifying Handle */
		uint16_t tsih;
	} tsih;

	/** Expected Command SN getter */
	uint32_t getEXPCMDSN() const { return expcmdsn_gen.cur(); }
	/** MAX Command SN getter */
	uint32_t getMAXCMDSN() const { return maxcmdsn_gen.cur(); }
	/** Increment Expected/Max Command SN */
	void advanceCMDSNs();

	/** get front iSCSI header token */
 	iSCSIHeader *frontToken() { return tokens.frontHeader(); }
	/** get front Data Segment */
	boost::shared_ptr<WriteSegment> frontDataSegment(const uint64_t lba,
							 const uint32_t transfer_length)
	{ return tokens.frontDataSegment(lba, transfer_length); }
	/** pop front token */
	bool popToken() { return tokens.pop(); }

	/** Send Task container class */
	class SessionSendTask {
	public:
		/** push new task
		    @param task : task to push
		*/
		void push(TaskPtr task) { tasks.push_back(task); }
		/** push new task list
		    @param tasklist : task list to push
		*/
		void push(list<TaskPtr> &tasklist) { tasks.merge(tasklist); }
		/** invoke task
		    @return success of failure
		 */
		bool invoke();
	private:
		/** pop/get front task
		    @return front task to exec
		 */
		TaskPtr pop();
		/** Task list */
		list<TaskPtr> tasks;
	} sendtask;

	/** Receive information container */
	class SessionRecvInfo {
	public:
		/** put new info for ITT
		    @param itt : key for info
		    @param info : the info
		    @return success or failure
		 */
		bool put(const uint32_t itt, const DataOutInfoPtr info)
		{
			map<uint32_t,DataOutInfoPtr>::iterator itr= infos.find(itt);
			if(itr != infos.end())
				return false;
			infos[itt] = info;
			return true;
		}
		/** get info for ITT
		    @param itt : search key
		    @return info : success
		    @return null : failed
		 */
		DataOutInfoPtr get(const uint32_t itt)
		{
			map<uint32_t,DataOutInfoPtr>::iterator itr= infos.find(itt);
			if(itr != infos.end())
				return itr->second;
			DataOutInfoPtr null;
			return null;
		}
		/** pop info for ITT
		    @param itt : search key
		    @return success or failure
		 */
		bool pop(const uint32_t itt)
		{
			map<uint32_t,DataOutInfoPtr>::iterator itr= infos.find(itt);
			if(itr == infos.end())
				return false;
			infos.erase(itr);
			return true;
		}
	private:
		map<uint32_t,DataOutInfoPtr> infos; 
	} recvinfo;

	/** Connection container class */
	class Connection {
	public:
		/** add new connection
		    @param conn : new connection to add
		*/
		bool add(const iSCSIConnectionPtr &conn) throw();
		/** delete exists connection
		    @param fd : connection's socket to delete
		*/
		bool del(const socket_t fd) throw();
		/** delete exists connection
		    @param fd : connection's socket to delete
		*/
		bool del(const iSCSIConnectionPtr &conn) throw();
		/** get exists connection
		    @param fd : File Descriptor
		    @return fd's connection
		    @return null : no such connection
		*/
		iSCSIConnectionPtr find(const socket_t fd) const;
		/** get initial connection
		    @return connection : only one initial connection
		    @return null : no connection or multi-connections found
		*/
		iSCSIConnectionPtr getInitial() const;
		/** clear listener for each ports */
		void clearListener();
		/** connetion counter */
		size_t count() const { return ports.size(); }
	private:
		/** accepted ports pool FD/Connection map */
		typedef map<socket_t,iSCSIConnectionPtr> conn_map_t;
		conn_map_t ports;
	} connection;

	/** Session's initiator class */
	class Initiator {
	public:
		/** initiator setter
		    @param ainitiator : initiator to set
		 */
		void set(iSCSIInitiatorPtr ainitiator) { initiator = ainitiator; }
		/** initiator getter */
		iSCSIInitiatorPtr get() { return initiator; }
	private:
		/** iSCSI initiator */
		iSCSIInitiatorPtr initiator;
	} initiator;

	/** Write Cache container */
	vector<boost::shared_ptr<WriteCache> > wcaches;
	/** */
	boost::shared_ptr<WriteCache> findLargestWriteCache();

	/** Command queue size */
	static const uint32_t CMD_QUEUE_SIZE = 0x8f;

protected:
	/** Manager cache */
	iSCSIManager &manager;

	/** EXPCMDSN generator */
	SequenceCounter expcmdsn_gen;
	/** MAXCMDSN generator */
	SequenceCounter maxcmdsn_gen;
	size_t getCMDQueue() const
	{ return maxcmdsn_gen.cur() - expcmdsn_gen.cur() + 1; }

	/** iSCSI tokens queue */
	iSCSITokenizer tokens;

	/** Configuration Attributes map type */
	typedef map<string, boost::variant<bool,uint32_t> > attr_map_t;
	/** Configuration Attributes map definition */
	attr_map_t attrs;

	/** iSCSI protocol handlers map type  */
	typedef map<uint8_t, iSCSIHandlerPtr> handler_map_t;
	/** iSCSI protocol handlers map definition */
	handler_map_t protos;
private:
	/** my own MaxRecvDataSegmentLength */
	uint32_t mrds_length;
	status_t doRecv(boost::shared_ptr<iSCSISession> session,
			const socket_t fd);
	status_t doWork(boost::shared_ptr<iSCSISession> session,
			const socket_t fd);
	status_t cleanUp(const socket_t fd);
};

typedef boost::shared_ptr<iSCSISession> iSCSISessionPtr;

template<typename T>
bool iSCSISession::getAttr(const string &attr, T &value)
{
	attr_map_t::iterator itr = attrs.find(attr);
	if(itr == attrs.end())
		return false;
	value = boost::get<T>(itr->second);
	return true;
}

#endif /* __ISCSI_SESSION_H__ */
