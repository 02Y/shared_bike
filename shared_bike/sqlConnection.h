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
		assert(m_pRes_ == NULL);       //如果此时已经保存了结果集，那么应该让程序报错，防止内存泄漏
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

	MYSQL* mysql() { return mysql_; }     //获取Mysql

	bool init(const char* szHost, int nPort, const char* szUser, const char* szPasswd, const char* szDb);    //初始化Mysql

	bool Execute(const char* szSql);    //查找是否存在（执行语句）

	bool Execute(const char* szSql, SqlRecordSet& recordSet);  //MYSQL_RES*  //查找是否存在并返回结果集

	int EscapeString(const char* pSrc, int nSrcLen, char* pDest);       //查询前的字符串转义

	void close();     //关闭Mysql

	const char* GetErrInfo();      //返回错误信息

	void Reconnect();      //重连

private:
	MYSQL* mysql_;
};

#endif // _DATASTORE_MYSQL_CONNECTION_H_

