#include "sqlConnection.h"
#include "string.h"


MysqlConnection::MysqlConnection()
{
	mysql_ = (MYSQL*)malloc(sizeof(MYSQL));
}

MysqlConnection::~MysqlConnection()
{
	if (mysql_ != NULL)
	{
		mysql_close(mysql_);
		free(mysql_);
		mysql_ = NULL;
	}

	return;
}

bool MysqlConnection::init(const char* szHost, int nPort, const char* szUser, const char* szPasswd, const char* szDb)
{
	LOG_INFO("enter MysqlConnection::init\n");

	if (NULL == (mysql_ = mysql_init(mysql_)))
	{
		LOG_ERROR("init mysql failed %s, %d", this->GetErrInfo(), errno);
		return false;
	}

	/*mysql_options(mysql_, MYSQL_SET_CHARSET_NAME, "gbk");    //�����ַ����� */

	char cAuto = 1;
	if (mysql_options(mysql_, MYSQL_OPT_RECONNECT, &cAuto) != 0)       //����Ͽ����ӣ��Զ�����   //�ɹ�����0
	{
		LOG_ERROR("mysql_options MYSQL_OPT_RECONNECT failed.");
	}

	if (NULL == (mysql_real_connect(mysql_, szHost, szUser, szPasswd, szDb, nPort, NULL, 0)))
	{
		LOG_ERROR("connect mysql failed %s", this->GetErrInfo());
		return false;
	}

	return true;
}

bool MysqlConnection::Execute(const char* szSql)
{
	if (mysql_real_query(mysql_, szSql, strlen(szSql)) != 0)
	{
		if (mysql_errno(mysql_) == CR_SERVER_GONE_ERROR)     //�������ԭ��Ϊ�Ͽ����ӣ���������
		{
			Reconnect();
		}
		return false;
	}
	return true;
}

bool MysqlConnection::Execute(const char* szSql, SqlRecordSet& recordSet)
{
	if (mysql_real_query(mysql_, szSql, strlen(szSql)) != 0)
	{
		if (mysql_errno(mysql_) == CR_SERVER_GONE_ERROR)     //�������ԭ��Ϊ�Ͽ����ӣ���������
		{
			Reconnect();
		}
		return false;
	}
	MYSQL_RES* pRes = mysql_store_result(mysql_);    //��ȡ�����
	if (!pRes)
	{
		return NULL;
	}
	recordSet.SetResult(pRes);

	return true;
}

int MysqlConnection::EscapeString(const char* pSrc, int nSrcLen, char* pDest)
{
	if (!mysql_)
	{
		return 0;
	}
	return mysql_real_escape_string(mysql_, pDest, pSrc, nSrcLen);
}

void MysqlConnection::close()
{
	if (mysql_ != NULL)
	{
		mysql_close(mysql_);
		free(mysql_);
		mysql_ = NULL;
	}

	return;
}

const char* MysqlConnection::GetErrInfo()
{
	return mysql_error(mysql_);     //������һ�� MySQL �����������ı�������Ϣ
}

void MysqlConnection::Reconnect()
{
	mysql_ping(mysql_);
}
