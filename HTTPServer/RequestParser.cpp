#include "PCH.h"
#include "RequestParser.h"
#include "Request.h"

namespace
{
	bool IsChar(int c)
	{
		return c >= 0 && c <= 127;
	}

	bool IsCtl(int c)
	{
		return c >= 0 && c <= 31 || c == 127;
	}

	bool IsTSpecial(int c)
	{
		switch (c)
		{
		case '(': case ')': case '<': case '>': case '@':
		case ',': case ';': case ':': case '\\': case '"':
		case '/': case '[': case ']': case '?': case '=':
		case '{': case '}': case ' ': case '\t':
			return true;
		default:
			return false;
		}
	}

	bool IsDigit(int c)
	{
		return c >= '0' && c <= '9';
	}
}

RequestParser::RequestParser()
{
	Reset();
}

void RequestParser::Reset()
{
	m_State = method_start;
}

RequestParser::ResultType RequestParser::Consume(Request& request, char input)
{
	switch (m_State)
	{
	case method_start:
		if (!IsChar(input) || IsCtl(input) || IsTSpecial(input))
		{
			return bad;
		}

		m_State = method;
		request.Method.push_back(input);
		return indeterminate;
	case method:
		if (input == ' ')
		{
			m_State = uri;
			return indeterminate;
		}
		if (!IsChar(input) || IsCtl(input) || IsTSpecial(input))
		{
			return bad;
		}

		request.Method.push_back(input);
		return indeterminate;
	case uri:
		if (input == ' ')
		{
			m_State = http_version_h;
			return indeterminate;
		}
		if (IsCtl(input))
		{
			return bad;
		}

		request.Uri.push_back(input);
		return indeterminate;
	case http_version_h:
		if (input == 'H')
		{
			m_State = http_version_t_1;
			return indeterminate;
		}

		return bad;
	case http_version_t_1:
		if (input == 'T')
		{
			m_State = http_version_t_2;
			return indeterminate;
		}

		return bad;
	case http_version_t_2:
		if (input == 'T')
		{
			m_State = http_version_p;
			return indeterminate;
		}
		
		return bad;
	case http_version_p:
		if (input == 'P')
		{
			m_State = http_version_slash;
			return indeterminate;
		}
		
		return bad;
	case http_version_slash:
		if (input == '/')
		{
			request.Http_version_major = 0;
			request.Http_version_minor = 0;
			m_State = http_version_major_start;
			return indeterminate;
		}
		
		return bad;
	case http_version_major_start:
		if (IsDigit(input))
		{
			request.Http_version_major = request.Http_version_major * 10 + input - '0';
			m_State = http_version_major;
			return indeterminate;
		}
		
		return bad;
	case http_version_major:
		if (input == '.')
		{
			m_State = http_version_minor_start;
			return indeterminate;
		}
		if (IsDigit(input))
		{
			request.Http_version_major = request.Http_version_major * 10 + input - '0';
			return indeterminate;
		}
		
		return bad;
	case http_version_minor_start:
		if (IsDigit(input))
		{
			request.Http_version_minor = request.Http_version_minor * 10 + input - '0';
			m_State = http_version_minor;
			return indeterminate;
		}
		
		return bad;
	case http_version_minor:
		if (input == '\r')
		{
			m_State = expecting_newline_1;
			return indeterminate;
		}
		if (IsDigit(input))
		{
			request.Http_version_minor = request.Http_version_minor * 10 + input - '0';
			return indeterminate;
		}
		
		return bad;
	case expecting_newline_1:
		if (input == '\n')
		{
			m_State = header_line_start;
			return indeterminate;
		}
		
		return bad;
	case header_line_start:
		if (input == '\r')
		{
			m_State = expecting_newline_3;
			return indeterminate;
		}
		if (!request.Headers.empty() && (input == ' ' || input == '\t'))
		{
			m_State = header_lws;
			return indeterminate;
		}
		if (!IsChar(input) || IsCtl(input) || IsTSpecial(input))
		{
			return bad;
		}
		
		request.Headers.emplace_back();
		request.Headers.back().Name.push_back(input);
		m_State = header_name;
		return indeterminate;
	case header_lws:
		if (input == '\r')
		{
			m_State = expecting_newline_2;
			return indeterminate;
		}
		if (input == ' ' || input == '\t')
		{
			return indeterminate;
		}
		if (IsCtl(input))
		{
			return bad;
		}

		m_State = header_value;
		request.Headers.back().Value.push_back(input);
		return indeterminate;
	case header_name:
		if (input == ':')
		{
			m_State = space_before_header_value;
			return indeterminate;
		}
		if (!IsChar(input) || IsCtl(input) || IsTSpecial(input))
		{
			return bad;
		}
		
		request.Headers.back().Name.push_back(input);
		return indeterminate;
	case space_before_header_value:
		if (input == ' ')
		{
			m_State = header_value;
			return indeterminate;
		}

		return bad;
	case header_value:
		if (input == '\r')
		{
			m_State = expecting_newline_2;
			return indeterminate;
		}
		if (IsCtl(input))
		{
			return bad;
		}

		request.Headers.back().Value.push_back(input);
		return indeterminate;
	case expecting_newline_2:
		if (input == '\n')
		{
			m_State = header_line_start;
			return indeterminate;
		}
		
		return bad;
	case expecting_newline_3:
		return input == '\n' ? good : bad;
	default:
		return bad;
	}
}
