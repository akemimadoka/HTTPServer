#include "PCH.h"
#include "Reply.h"
#include <natType.h>

#define STANDARD_STOCK_REPLIES(title, content) "<html><head><title>" ## title ## "</title></head><body><h1>" ## content ## "</h1></body></html>"

namespace
{
	constexpr nChar NameValueSeparator[] = { ':', ' ' };
	constexpr nChar Crlf[] = { '\r', '\n' };

	boost::asio::const_buffer ToBuffer(Reply::StatusType status)
	{
		switch (status)
		{
		case Reply::ok:
			return boost::asio::buffer("HTTP/1.0 200 OK\r\n");
		case Reply::created:
			return boost::asio::buffer("HTTP/1.0 201 Created\r\n");
		case Reply::accepted:
			return boost::asio::buffer("HTTP/1.0 202 Accepted\r\n");
		case Reply::no_content:
			return boost::asio::buffer("HTTP/1.0 204 No Content\r\n");
		case Reply::multiple_choices:
			return boost::asio::buffer("HTTP/1.0 300 Multiple Choices\r\n");
		case Reply::moved_permanently:
			return boost::asio::buffer("HTTP/1.0 301 Moved Permanently\r\n");
		case Reply::moved_temporarily:
			return boost::asio::buffer("HTTP/1.0 302 Moved Temporarily\r\n");
		case Reply::not_modified:
			return boost::asio::buffer("HTTP/1.0 304 Not Modified\r\n");
		case Reply::bad_request:
			return boost::asio::buffer("HTTP/1.0 400 Bad Request\r\n");
		case Reply::unauthorized:
			return boost::asio::buffer("HTTP/1.0 401 Unauthorized\r\n");
		case Reply::forbidden:
			return boost::asio::buffer("HTTP/1.0 403 Forbidden\r\n");
		case Reply::not_found:
			return boost::asio::buffer("HTTP/1.0 404 Not Found\r\n");
		default:
		case Reply::internal_server_error:
			return boost::asio::buffer("HTTP/1.0 500 Internal Server Error\r\n");
		case Reply::not_implemented:
			return boost::asio::buffer("HTTP/1.0 501 Not Implemented\r\n");
		case Reply::bad_gateway:
			return boost::asio::buffer("HTTP/1.0 502 Bad Gateway\r\n");
		case Reply::service_unavailable:
			return boost::asio::buffer("HTTP/1.0 503 Service Unavailable\r\n");
		}
	}

	ncStr ToStandardStockReply(Reply::StatusType status)
	{
		switch (status)
		{
		case Reply::ok:
			return "";
		case Reply::created:
			return STANDARD_STOCK_REPLIES("Created", "201 Created");
		case Reply::accepted:
			return STANDARD_STOCK_REPLIES("Accepted", "202 Accepted");
		case Reply::no_content:
			return STANDARD_STOCK_REPLIES("No Content", "204 No Content");
		case Reply::multiple_choices:
			return STANDARD_STOCK_REPLIES("Multiple Choices", "300 Multiple Choices");
		case Reply::moved_permanently:
			return STANDARD_STOCK_REPLIES("Moved Permanently", "301 Moved Permanently");
		case Reply::moved_temporarily:
			return STANDARD_STOCK_REPLIES("Moved Temporarily", "302 Moved Temporarily");
		case Reply::not_modified:
			return STANDARD_STOCK_REPLIES("Not Modified", "304 Not Modified");
		case Reply::bad_request:
			return STANDARD_STOCK_REPLIES("Bad Request", "400 Bad Request");
		case Reply::unauthorized:
			return STANDARD_STOCK_REPLIES("Unauthorizied", "401 Unauthorizied");
		case Reply::forbidden:
			return STANDARD_STOCK_REPLIES("Forbidden", "403 Forbidden");
		case Reply::not_found:
			return STANDARD_STOCK_REPLIES("Not Found", "404 Not Found");
		default:
		case Reply::internal_server_error:
			return STANDARD_STOCK_REPLIES("Internal Server Error", "500 Internal Server Error");
		case Reply::not_implemented:
			return STANDARD_STOCK_REPLIES("Not Implemented", "501 Not Implemented");
		case Reply::bad_gateway:
			return STANDARD_STOCK_REPLIES("Bad Gateway", "502 Bad Gateway");
		case Reply::service_unavailable:
			return STANDARD_STOCK_REPLIES("Service Unavailable", "503 Service Unavailable");
		}
	}
}

std::vector<boost::asio::const_buffer> Reply::GetBuffers()
{
	std::vector<boost::asio::const_buffer> buffers;
	buffers.emplace_back(ToBuffer(Status));
	for (auto&& item : Headers)
	{
		buffers.emplace_back(boost::asio::buffer(item.Name));
		buffers.emplace_back(boost::asio::buffer(NameValueSeparator));
		buffers.emplace_back(boost::asio::buffer(item.Value));
		buffers.emplace_back(boost::asio::buffer(Crlf));
	}
	buffers.emplace_back(boost::asio::buffer(Crlf));
	buffers.emplace_back(boost::asio::buffer(Content));

	return move(buffers);
}

Reply Reply::ShockReply(StatusType status)
{
	auto content = ToStandardStockReply(status);
	return { status, {{"Content-Length", std::to_string(std::char_traits<nChar>::length(content))}, {"Content-Type", "text/html"}}, content };
}
