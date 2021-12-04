#ifndef _BRKS_COMMON_DATASEVER_SQLTABLES_H_
#define _BRKS_COMMON_DATASEVER_SQLTABLES_H_

#include <memory>
#include "sqlConnection.h"
#include "glo_def.h"

class SqlTabel
{
public:
	SqlTabel(std::shared_ptr<MysqlConnection> sqlconn) :sqlconn_(sqlconn){}

	bool CreateUserInfo()        //创建用户表
	{
		const char* pUserInfoTabel = " \
									 CREATE TABLE IF NOT EXISTS userinfo( \
									 id            int(16)          NOT NULL PRIMARY KEY AUTO_INCREMENT COMMENT'用户id', \
                                     mobile        varchar(16)      NOT NULL DEFAULT '13000000000' COMMENT'手机号', \
                                     username      varchar(128)     NOT NULL DEFAULT '' COMMENT'用户名', \
                                     verify        int(4)           NOT NULL DEFAULT 0 COMMENT'验证',  \
                                     registertm    timestamp        NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT'注册时间', \
                                     money         int(4)           NOT NULL DEFAULT 0 COMMENT'余额', \
                                     INDEX         mobile_index(mobile)   \
					                 )";
		if (!sqlconn_->Execute(pUserInfoTabel))
		{
			LOG_ERROR("create table userinfo table failed. error msg: %s", sqlconn_->GetErrInfo());
			return false;
		}
		return true;
	}

	bool CreateBikeTable()        //创建单车表
	{
		const char* pBikeInfoTabel = " \
			                         CREATE TABLE IF NOT EXISTS bikeInfo( \
	                                 id            int              NOT NULL PRIMARY KEY AUTO_INCREMENT COMMENT'单车id', \
			                         devno         int              NOT NULL COMMENT'单车编号', \
                                     status        tinyint(1)       NOT NULL DEFAULT 0 COMMENT'单车状态', \
                                     trouble       int              NOT NULL DEFAULT 0 COMMENT'损坏类型编号', \
                                     tmsg          varchar(256)     NOT NULL DEFAULT '' COMMENT'损坏原因描述', \
                                     latitude      double(10, 6)    NOT NULL DEFAULT 0 COMMENT'维度', \
                                     longitude     double(10, 6)    NOT NULL DEFAULT 0 COMMENT'经度', \
                                     UNIQUE(devno)    \
                                     )";
		if (!sqlconn_->Execute(pBikeInfoTabel))
		{
			LOG_ERROR("create table bikeinfo table failed. error msg: %s", sqlconn_->GetErrInfo());
			return false;
		}
		return true;
	}

private:
	std::shared_ptr<MysqlConnection> sqlconn_;
};

#endif // _BRKS_COMMON_DATASEVER_SQLTABLES_H_

