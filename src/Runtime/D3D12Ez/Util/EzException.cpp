#include "EzException.h"

std::atomic_size_t D3EZ::EzException::s_eid_ciunt;

D3EZ::EzException::EzException(std::string_view function, int line)
{
	AddDetail_Description("Exception occurred!");
	AddDetail_Location(function, line);
}

D3EZ::EzException::EzException(std::string_view function, int line, std::string_view what)
{
	AddDetail_Description("Exception occurred!");
	AddDetail_Description(what);
	AddDetail_Location(function, line);
}

D3EZ::EzException::EzException(std::string_view function, int line, std::string_view what, std::string_view why)
{
	AddDetail_Description("Exception occurred!");
	AddDetail_Description(what);
	AddDetail_Location(function, line);
	AddDetail_Reason(why);
}

void D3EZ::EzException::AddDetail_Location(std::string_view function, int line)
{
	AppendMessage(fmt::format("In Funtion {} on line {}", function, line));
}

void D3EZ::EzException::AddDetail_Description(std::string_view desc)
{
	AppendMessage(desc);
}

void D3EZ::EzException::AddDetail_Reason(std::string_view reason)
{
	AppendMessage(fmt::format("Caused by: {}", reason));
}

void D3EZ::EzException::AppendMessage(std::string_view msg)
{
	if (m_msg.empty())
	{
		m_msg = msg;
	}
	else
	{
		m_msg += "\n";
		m_msg += msg;
	}

	spdlog::error("[Exception] [{}] {}", m_eid, msg);
}

char const* D3EZ::EzException::what() const
{
	return m_msg.c_str();
}

