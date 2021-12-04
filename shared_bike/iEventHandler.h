#ifndef _NS_IEVENT_HANDLER_H_
#define _NS_IEVENT_HANDLER_H_

#include "ievent.h"
#include "eventtype.h"

class iEventHandler
{
public:
	iEventHandler(const char* name) :name_(name) {}
	virtual ~iEventHandler() {}
	virtual iEvent* handle(const iEvent* ev) { return NULL; }
	std::string get_name() { return name_; }

private:
	std::string name_;
};




#endif // _NS_IEVENT_HANDLER_H_
