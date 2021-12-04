#ifndef _BRKS_BUS_MAIN_H_
#define _BRKS_BUS_MAIN_H_

#include "SqlTables.h"
#include "UserEventHandler.h"

class BusinessProcessor
{
public:
	BusinessProcessor(std::shared_ptr<MysqlConnection> conn);
	
	bool init();      

	virtual ~BusinessProcessor();

private:
	std::shared_ptr<MysqlConnection> mysqlconn_;   
	std::shared_ptr<UserEventHandler> ueh_;        //用户事件处理器
	//...可以增添其它事件处理器
};

#endif // _BRKS_BUS_MAIN_H_

