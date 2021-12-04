#include "eventtype.h"

static EErrorReason EER[] =
{
	{ERRC_SUCCESS, "Ok."},
	{ERRC_INVALID_MSG, "Invalid message."},
	{ERRC_INVALID_DATA, "Invalid data."},
	{ERRC_METHOD_NOT_ALLOWED, "Method not allowed."},
	{ERRO_PROCCESS_FAILED, "Proccess failed."},
	{ERRO_BIKE_IS_TOOK, "Bike is took."},
	{ERRO_BIKE_IS_RUNNING, "Bike is running."},
	{ERRO_BIKE_IS_DAMAGED,"Bike is damaged."},
	{ERR_NULL,"Undefined"}
};

static EidString ES[] =
{
	{EEVENTID_GET_MOBILE_CODE_REQ, "EEVENTID_GET_MOBILE_CODE_REQ"},
	{EEVENTID_GET_MOBILE_CODE_RSP, "EEVENTID_GET_MOBILE_CODE_RSP"},
	{EEVENTID_LOGIN_REQ, "EEVENTID_LOGIN_REQ"},
	{EEVENTID_LOGIN_RSP, "EEVENTID_LOGIN_RSP"},
	{EEVENTID_RECHARGE_REQ, "EEVENTID_RECHARGE_REQ"},
	{EEVENTID_RECHARGE_RSP, "EEVENTID_RECHARGE_RSP"},
	{EEVENTID_GET_ACCOUNT_BALANCE_REQ, "EEVENTID_GET_ACCOUNT_BALANCE_REQ"},
	{EEVENTID_ACCOUNT_BALANCE_RSP, "EEVENTID_ACCOUNT_BALANCE_RSP"},
	{EEVENTID_LIST_ACCOUNT_RECORDS_REQ, "EEVENTID_LIST_ACCOUNT_RECORDS_REQ"},
	{EEVENTID_ACCOUNT_RECORDS_RSP, "EEVENTID_ACCOUNT_RECORDS_RSP"},
	{EEVENTID_LIST_TRAVELS_REQ, "EEVENTID_LIST_TRAVELS_REQ"},
	{EEVENTID_LIST_TRAVELS_RSP, "EEVENTID_LIST_TRAVELS_RSP"},
	{EEVENTID_EXIT_RSP, "EEVENTID_EXIT_RSP"},
	{EEVENTID_UNKOWN, "EEVENTID_UNKOWN"}
};

const char* getReasonByErrorCode(i32 code)
{
	i32 i = 0;
	for (i = 0; EER[i].code != ERR_NULL; i++)
	{
		if (EER[i].code == code)
		{
			return EER[i].reason;
		}
	}

	return EER[i].reason;  //"Undefined"
}

const char* getEidToString(i32 eid)
{
	i32 i = 0;
	for (i = 0; ES[i].eid != EEVENTID_UNKOWN; i++)
	{
		if (ES[i].eid == eid)
		{
			return ES[i].phrase;
		}
	}

	return ES[i].phrase;    //"EEVENTID_UNKOWN";
}
