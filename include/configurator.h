/**
   @file configurator.h
   @brief Configurator
   @author Eiji Sugiura <eiji.sugiura@gmail.com> Research Institute of Systems Planning, Inc
   @version $Id: configurator.h 102 2007-05-29 01:35:12Z sugiura $
 */
#ifndef __CONFIGURATOR_H__
#define __CONFIGURATOR_H__

#include <errno.h>
#include <stdexcept>
#include <map>
#include <boost/utility.hpp>
#include <boost/variant.hpp>
#include <boost/thread.hpp>
#include "json/value.h"
#include "rootvalidator.h"

using namespace std;

/** Configurator class */
class Configurator : private boost::noncopyable{
	friend class TargetPortConfigurator;
	friend class TargetConfigurator;
	friend class InitiatorConfigurator;
	friend class VolumeConfigurator;
	friend class VolumeInfo;
	typedef boost::recursive_mutex::scoped_lock lock;
private:
	/** Configurator implementation */
	struct ConfiguratorImpl {
		ConfiguratorImpl();
		~ConfiguratorImpl();
		/** clear configurations */
		void clear();
		/** set default configuration */
		bool setDefaults();
		/** parse lines, and store it 
		    @param lines : input lines to parse
		*/
		bool parse(const string &lines);
		/** dump stored configurations 
		    @param lines : output buffer for dump
		*/
		bool dump(string &lines);
		/** bool Attribute getter 
		    @param attr : attribute name
		    @param value : value for output
		*/
		bool getAttr(const string& attr, bool& value);
		/** string Attribute getter 
		    @param attr : attribute name
		    @param value : value for output
		*/
		bool getAttr(const string& attr, string& value);
		/** int Attribute getter 
		    @param attr : attribute name
		    @param value : value for output
		*/
		bool getAttr(const string& attr, int& value);
		/** uint64 Attribute getter 
		    @param attr : attribute name
		    @param value : value for output
		*/
		bool getAttr(const string& attr, uint64_t& value);
		/** Attribute setter 
		    @param attr : attribute name
		    @param value : value to input
		*/
		template<typename T>
		bool setAttr(const string& attr, const T& value);
		/** Save Command Args */
		void saveCommandArgs();
		/** Restore Command Args */
		void restoreCommandArgs();

		/** configuration root tree */
		Json::Value root;
		/** command args container */
		Json::Value cmdargs;

		/** configuration attribute/value variants map */
		typedef map<string, boost::variant<uint64_t,bool,string> > config_map_t;
		/** default attribute/value map */
		config_map_t defaultValue;
		/** Configuration mutex */
		boost::recursive_mutex config_guard;
	};

public:
	/** Configurator accessor */
	static Configurator &getInstance() throw(std::out_of_range);
	virtual ~Configurator() throw()
	{
		if(pimpl){
			delete pimpl;
			pimpl = NULL;
		}
	}
	/** Configurator mutex accessor */
	static boost::recursive_mutex &getGuard();

	/**
	   Set default values
	 */
	bool setDefaults() throw(std::runtime_error);

	/** Attribute setter
	    @param attr : attribute name
	    @param value : value to input
	*/
	template <typename T>
	bool setAttr(const string& attr, const T& value) throw ()
	{
		bool ret = false;
		try {
			RootValidator validator;
			ret = validator.validAttr(attr, value);
			if(ret)
				ret = pimpl->setAttr(attr, value);
		} catch(...) {
			ret = false;
		}
		return ret;
	}

	/** Attribute getter 
	    @param attr : attribute name
	    @param value : value for output
	*/
	template <typename T>
	bool getAttr(const string& attr, T& value) const throw ()
	{
		bool ret = false;
		try {
			ret = pimpl->getAttr(attr, value);
		} catch(const std::exception &e) {
			ret = false;
		}
		return ret;
	}

	/** parse input lines 
	    @param lines : input configurations
	 */
	bool parse(const string& lines) throw (std::runtime_error);

	/** dump out to lines */
	string dump() throw (std::runtime_error);

	/** clear stored configurations */
	void clear() throw();

	/** validate stored attrs & values */
	bool validate() throw (std::runtime_error);

private:
	Configurator() throw();
	/** initializer */
	static void init() throw(std::out_of_range);
	/** instanse cache */
	static Configurator *cache;
	/** Configurator implementation */
	ConfiguratorImpl *pimpl;
};

template<typename T>
bool Configurator::ConfiguratorImpl::setAttr(const string& attr, const T& value)
{
	lock lk(Configurator::getGuard());
	root[attr] = value;
	return true;
}


#endif /* __CONFIGURATOR_H__ */
