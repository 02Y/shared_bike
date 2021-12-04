#include "user_service.h"

UserService::UserService(std::shared_ptr<MysqlConnection> sql_conn)
	:sql_conn_(sql_conn)
{
}

bool UserService::exist(const std::string& mobile)
{
	char sql_content[1024] = { 0 };
	sprintf(sql_content, "SELECT * FROM userinfo WHERE mobile = %s", mobile.c_str());

	SqlRecordSet record_set;
	if (!sql_conn_->Execute(sql_content, record_set))
	{
		return false;       //û���ҵ�
	}
	else
	{
		return (record_set.GetRowCount() != 0);
	}
}

bool UserService::insert(const std::string& mobile)
{
	char sql_content[1024] = { 0 };
	sprintf(sql_content, "INSERT INTO userinfo (mobile) VALUES (%s)", mobile.c_str());

	return sql_conn_->Execute(sql_content);
}
