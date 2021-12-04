#include "events_def.h"


std::ostream& MobileCodeReqEv::dump(std::ostream& out) const
{
	out << "MobileCodeReq sn = " << get_sn() << ", "
		<< "mobile = " << msg_.mobile() << std::endl;

	return out;
}

std::ostream& MobileCodeRspEv::dump(std::ostream& out) const
{
	out << "MobileCodeRsp sn = " << get_sn() << ", "
		<< "code = " << msg_.code() << ", "
		<< "icode = " << msg_.icode() << ", "
		<< "describe: " << msg_.data() << std::endl;

	return out;
}

std::ostream& LoginReqEv::dump(std::ostream& out) const
{
	out << "LoginReqEv sn = " << get_sn() << ", "
		<< "mobile = " << msg_.mobile() << ", "
		<< "icode: " << msg_.icode() << std::endl;

	return out;
}

std::ostream& LoginRspEv::dump(std::ostream& out) const
{
	out << "LoginRspEv sn = " << get_sn() << ", "
		<< "code = " << msg_.code() << ", "
		<< "describe: " << msg_.desc() << std::endl;

	return out;
}
