#ifndef _BRKS_COMMON_DATASEVER_SQLTABLES_H_
#define _BRKS_COMMON_DATASEVER_SQLTABLES_H_

#include <memory>
#include "sqlConnection.h"
#include "glo_def.h"

class SqlTabel
{
public:
	SqlTabel(std::shared_ptr<MysqlConnection> sqlconn) :sqlconn_(sqlconn){}

	bool CreateUserInfo()        //�����û���
	{
		const char* pUserInfoTabel = " \
									 CREATE TABLE IF NOT EXISTS userinfo( \
									 id            int(16)          NOT NULL PRIMARY KEY AUTO_INCREMENT COMMENT'�û�id', \
                                     mobile        varchar(16)      NOT NULL DEFAULT '13000000000' COMMENT'�ֻ���', \
                                     username      varchar(128)     NOT NULL DEFAULT '' COMMENT'�û���', \
                                     verify        int(4)           NOT NULL DEFAULT 0 COMMENT'��֤',  \
                                     registertm    timestamp        NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT'ע��ʱ��', \
                                     money         int(4)           NOT NULL DEFAULT 0 COMMENT'���', \
                                     INDEX         mobile_index(mobile)   \
					                 )";
		if (!sqlconn_->Execute(pUserInfoTabel))
		{
			LOG_ERROR("create table userinfo table failed. error msg: %s", sqlconn_->GetErrInfo());
			return false;
		}
		return true;
	}

	bool CreateBikeTable()        //����������
	{
		const char* pBikeInfoTabel = " \
			                         CREATE TABLE IF NOT EXISTS bikeInfo( \
	                                 id            int              NOT NULL PRIMARY KEY AUTO_INCREMENT COMMENT'����id', \
			                         devno         int              NOT NULL COMMENT'�������', \
                                     status        tinyint(1)       NOT NULL DEFAULT 0 COMMENT'����״̬', \
                                     trouble       int              NOT NULL DEFAULT 0 COMMENT'�����ͱ��', \
                                     tmsg          varchar(256)     NOT NULL DEFAULT '' COMMENT'��ԭ������', \
                                     latitude      double(10, 6)    NOT NULL DEFAULT 0 COMMENT'ά��', \
                                     longitude     double(10, 6)    NOT NULL DEFAULT 0 COMMENT'����', \
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

