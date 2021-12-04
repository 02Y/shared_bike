#ifndef _DATASTORE_MYSQL_CONNECTION_H_
#define _DATASTORE_MYSQL_CONNECTION_H_

#include <mysql/mysql.h>
#include <string>
#include <mysql/errmsg.h>
#include <assert.h>

#include "glo_def.h"

class SqlRecordSet
{
public:
	SqlRecordSet() :m_pRes_(NULL){}

	explicit SqlRecordSet(MYSQL_RES* pRes) { m_pRes_ = pRes; }
		
	MYSQL_RES* mysqlRes() { return m_pRes_; }

	~SqlRecordSet()
	{
		if (m_pRes_)
		{
			mysql_free_result(m_pRes_);
		}
	}

	inline void SetResult(MYSQL_RES* pRes)
	{
		assert(m_pRes_ == NULL);       //�����ʱ�Ѿ������˽��������ôӦ���ó��򱨴���ֹ�ڴ�й©
		if (m_pRes_)
		{
			LOG_WARN("the MYSQL_RES has already stored result, mabe will cause memory leak.");
		}
		m_pRes_ = pRes;
	}

	inline MYSQL_RES* GetResult() { return m_pRes_; }

	void FetchRow(MYSQL_ROW& row) { row = mysql_fetch_row(m_pRes_); }

	inline i32 GetRowCount() { return m_pRes_->row_count; }

private:
	MYSQL_RES* m_pRes_;
};

class MysqlConnection
{
public:
	MysqlConnection();
	~MysqlConnection();

	MYSQL* mysql() { return mysql_; }     //��ȡMysql

	bool init(const char* szHost, int nPort, const char* szUser, const char* szPasswd, const char* szDb);    //��ʼ��Mysql

	bool Execute(const char* szSql);    //�����Ƿ���ڣ�ִ����䣩

	bool Execute(const char* szSql, SqlRecordSet& recordSet);  //MYSQL_RES*  //�����Ƿ���ڲ����ؽ����

	int EscapeString(const char* pSrc, int nSrcLen, char* pDest);       //��ѯǰ���ַ���ת��

	void close();     //�ر�Mysql

	const char* GetErrInfo();      //���ش�����Ϣ

	void Reconnect();      //����

private:
	MYSQL* mysql_;
};

#endif // _DATASTORE_MYSQL_CONNECTION_H_

