#include "BusinessProcessor.h"

BusinessProcessor::BusinessProcessor(std::shared_ptr<MysqlConnection> conn)
	:mysqlconn_(conn), ueh_(new UserEventHandler())
{

}

bool BusinessProcessor::init()
{
	SqlTabel tables(mysqlconn_);
	tables.CreateUserInfo();        //�����û���
	tables.CreateBikeTable();       //����������

	return true;
}

BusinessProcessor::~BusinessProcessor()
{
	ueh_.reset();
	ueh_ = nullptr;
}
