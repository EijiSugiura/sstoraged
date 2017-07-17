/**
   @file configbinder.h
   @brief Configuration File Binder
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: configbinder.h 29 2007-02-27 05:29:56Z sugiura $
 */
#ifndef __CONFIGBINDER_H__
#define __CONFIGBINDER_H__

#include <stdexcept>
#include <boost/utility.hpp>

using namespace std;

/** Configuration File Binder class */
class ConfigBinder : boost::noncopyable{
	/** ConfigBinder implementation */
	struct ConfigBinderImpl{
		/** default constructor */
		ConfigBinderImpl() throw() : lines("") {}
		/** destructor */
		~ConfigBinderImpl() throw() {}

		/** append config attributes from file
		    @param file : file to read/append
		*/
		bool append(const string& file);

		/** parse config attributes */
		bool parse();

		/** dump stored configurations to file
		    @param file : file to write/dump
		*/
		bool dump(const string& file);

		/** whole configuration validator */
		bool validate() const;

		/** clear stored configurations */
		void clear() throw() { lines.clear(); }

		/** configuration file line cache */
		string lines;
	};
public:
	/** default constructor */
	ConfigBinder() throw(std::exception) : pimpl(NULL){ pimpl = new ConfigBinderImpl(); }
	/** destructor */
	~ConfigBinder() throw() { delete pimpl; pimpl = NULL; }

	/** load config file 
	    @param file : file to read/parse
	 */
	bool load(const string& file) throw(std::runtime_error)
	{
		pimpl->clear();
		bool ret = false;
		try {
			if(!pimpl->append(file))
				return false;
			ret = pimpl->parse();
		} catch (const std::exception &e) {
			throw std::runtime_error(e.what());
		}
		return ret;
	}
	/** append config attributes from file
	    @param file : file to read/append
	 */
	bool append(const string& file) throw(std::runtime_error)
	{
		bool ret = false;
		try {
			if(!pimpl->append(file))
				return false;
			ret = pimpl->parse();
		} catch (const std::exception &e) {
			throw std::runtime_error(e.what());
		}
		return ret;
	}
	/** dump stored configurations to file
	    @param file : file to write/dump
	 */
	bool dump(const string& file) throw(std::runtime_error)
	{
		bool ret = false;
		try {
			ret = pimpl->dump(file);
		} catch (const std::exception &e) {
			throw std::runtime_error(e.what());
		}
		return ret;
	}
	/** whole configuration validator */
	bool validate() const throw(std::runtime_error)
	{
		bool ret = false;
		try {
			ret = pimpl->validate();
		} catch (const std::exception &e) {
			throw std::runtime_error(e.what());
		}
		return ret;
	}

	/** clear stored configurations */
	void clear()  throw() { pimpl->clear(); }
private:
	/** ConfigurationBinder implementation */
	ConfigBinderImpl *pimpl;
};

#endif /* __CONFIGBINDER_H__ */
