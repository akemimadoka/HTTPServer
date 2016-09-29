#pragma once
#include <iterator>

struct Request;

class RequestParser final
{
public:
	enum ResultType
	{
		good, bad, indeterminate
	};

	enum State
	{
		method_start,
		method,
		uri,
		http_version_h,
		http_version_t_1,
		http_version_t_2,
		http_version_p,
		http_version_slash,
		http_version_major_start,
		http_version_major,
		http_version_minor_start,
		http_version_minor,
		expecting_newline_1,
		header_line_start,
		header_lws,
		header_name,
		space_before_header_value,
		header_value,
		expecting_newline_2,
		expecting_newline_3
	};

	RequestParser();

	void Reset();

	template <typename InputIterator>
	std::pair<ResultType, InputIterator> Parse(Request& request, InputIterator begin, InputIterator end)
	{
		while (begin != end)
		{
			auto result = Consume(request, *begin);
			std::advance(begin, 1);
			if (result != indeterminate)
				return{ result, begin };
		}

		return{ indeterminate, begin };
	}

private:
	ResultType Consume(Request& request, char input);

	State m_State;
};
