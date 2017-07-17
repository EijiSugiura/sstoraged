/**
   @file taggedconfigurator.h
   @brief Tagged info Configurator
   @author Eiji Sugiura <eiji.sugiura@gmail.com> Research Institute of Systems Planning, Inc
   @version $Id: taggedconfigurator.h 40 2007-04-16 05:59:01Z sugiura $
 */

#ifndef __TAGGEDCONFIGURATOR_H__
#define __TAGGEDCONFIGURATOR_H__

#include <stdexcept>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/iterator_adaptors.hpp>
#include "configurator.h"

using namespace std;

/** Abstruct of Configuration infos */
template <typename KEYTYPE>
class ConfigInfo {
public:
	/** default constructor */
	ConfigInfo() {}
	/** destructor */
	virtual ~ConfigInfo() {}
	/** key getter */
	virtual KEYTYPE getKey() const = 0;
	/** member serializer */
	virtual Json::Value serialize() const = 0;
	/** validator */
	virtual bool valid() const = 0;
};

/** Tagged Configurator class*/
template <typename INFO, typename KEYTYPE = string>
class TaggedConfigurator {
	/** self declaration */
	typedef TaggedConfigurator<INFO,KEYTYPE> SELF;
	typedef boost::shared_ptr<INFO> INFO_PTR;
public:
	/** default constructor */
	TaggedConfigurator() throw()
		: pimpl(), index(0), prev_index(0), current_info() {}
	/** constructor
	    @param root : reference to the tagged Json::Value
	    @param tag  : the tag
	*/
	TaggedConfigurator(Json::Value &root, const string &tag) throw()
		: pimpl(), index(0), prev_index(0), current_info()
	{
		pimpl = TaggedConfiguratorImplPtr(new TaggedConfiguratorImpl(&(root[tag])));
	}
	/** destructor */
	virtual ~TaggedConfigurator() throw()
	{}
	/** Add new info
	    @param info : info to add
	 */
	bool add(const INFO &info)
	{
		/** duplicate check */
		if(find(info.getKey()) != end())
			return false;
		return pimpl->add(info);
	}
	/** Delete exist info by key
	    @param key : search key
	 */
	bool del(const KEYTYPE &key)
	{
		/** check entry existence */
		if(find(key) == end())
			return false;
		return pimpl->del(index);
	}
	/** info iterator */
        struct iterator : boost::iterator_adaptor<
		iterator,
		SELF*,
		INFO,
		boost::forward_traversal_tag
		>{
		/** constructor
		    @param p : self reference
		*/
		iterator( SELF* p ) : iterator::iterator_adaptor_ (p) {}
		/** step forward iterator */
		void increment(){ this->base_reference() = this->base()->next(); }
		/** dereferece from iterator */
		INFO& dereference() const { return this->base_reference()->current(); }
	};
	/** info iterator : begin */
        iterator begin()
	{
		if(pimpl->size() == 0)
			return end();

		prev_index = index = 0;
		current_info = INFO_PTR(pimpl->get(index));
		return iterator(this);
	}
	/** info iterator : end */
	iterator end() { return iterator(NULL); }
	/** search info by key
	    @param key : search key
	 */
	iterator find(const KEYTYPE &key)
	{
		iterator itr = begin();
		for(; itr != end(); ++itr){
			if((*itr).getKey() == key)
				break;
		}
		return itr;
	}
	/** remove info
	    @param itr : iterator to remove value
	 */
	iterator remove(iterator itr){
		pimpl->del(index);
		--index;
		return ++itr; 
	}
	/** clear all infos */
	void clear()
	{
		for(iterator itr = begin(); itr != end();){
			itr = remove(itr);
		}
	}
	/** info iterator : size */
	size_t size() { return pimpl->size(); }

	/** current info updater
	    @param info : refered value to update
	    @todo: remove this... */
	bool update(const INFO &info)
	{
		if(index > pimpl->size())
			return false;
		return pimpl->update(info, index);
	}

private:
	/** TaggedConfigurator implementation */
	struct TaggedConfiguratorImpl {
		/** mutex lock */
		typedef boost::recursive_mutex::scoped_lock lock;
		/** constructor
		    @param head : pointer to tagged Json::Value
		 */
		TaggedConfiguratorImpl(Json::Value *head)
			: root(head)
		{}
		/** Add new info
		    @param info : new info to add
		 */
		bool add(const INFO &info)
		{
 			if(!info.valid())
 				return false;
			lock lk(Configurator::getGuard());
			root->append(info.serialize());
			return true;
		}
		/** Delete exist info by key
		    @param key : search key to delete
		 */
		bool del(const size_t &key)
		{
			bool ret = false;
			lock lk(Configurator::getGuard());
			Json::Value tmp = (*root);
			root->clear();
			for( size_t counter = 0; counter < tmp.size(); ++counter ){
				if(counter == key){
					ret = true;
					continue;
				}
				root->append(tmp[counter]);
			}
			return ret;
		}
		/** info updater
		    @param info : value to update
		    @param key : search key
		 */
		bool update(const INFO &info, const int &key)
		{
 			if(!info.valid())
 				return false;
			lock lk(Configurator::getGuard());
			(*root)[key] = info.serialize();
			return true;
		}
		/** info getter
		    @param key : search key
		 */
		INFO *get(const size_t &key)
		{
			/** @todo: check existence */
			lock lk(Configurator::getGuard());
			if(!root->isArray())
				throw std::runtime_error("Not defined : ");
			return new INFO((*root)[key], key);
		}
		/** number of infos */
		size_t size()
		{
			lock lk(Configurator::getGuard());
			return root->size();
		}
		/** pointer to info maps */
		mutable Json::Value *root;
	};
	typedef boost::shared_ptr<TaggedConfiguratorImpl> TaggedConfiguratorImplPtr;
	TaggedConfiguratorImplPtr pimpl; 
	/** step forward to next info */
	SELF *next(){
		index++;
		if(index < pimpl->size())
			return this;
		return NULL;
	}
	/** get refer to current info */
	INFO &current()
	{
		if(index != prev_index) {
			current_info = INFO_PTR(pimpl->get(index));
			prev_index = index;
		}
		return *(current_info.get());
	}
	/** current index, previous index */
 	size_t index, prev_index;
	/** current info */
	INFO_PTR current_info;
};

#endif /* __TAGGEDCONFIGURATOR_H__ */
