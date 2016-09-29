#pragma once

#include <boost/asio.hpp>
#include <natType.h>
#include "MimeTypes.h"
#include "RequestHandler.h"
#include "Request.h"
#include "RequestParser.h"
#include "Reply.h"
#include <set>

class HTTPServer final
	: public boost::noncopyable
{
	class Session final
		: public boost::noncopyable, public std::enable_shared_from_this<Session>
	{
		enum
		{
			DefaultBufferSize = 1024,
		};
	public:
		Session(boost::asio::ip::tcp::socket && socket, HTTPServer& server);
		~Session();
		
		void Start();
		void Stop();

	private:
		void do_read();
		void do_write();

		boost::asio::ip::tcp::socket m_Socket;
		HTTPServer& m_Server;
		nByte m_Buffer[DefaultBufferSize];
		Request m_Request;
		RequestParser m_RequestParser;
		Reply m_Reply;
	};
public:
	HTTPServer(ncTStr rootDir, ncTStr address, ncTStr port, ncTStr defaultFile = nullptr);
	~HTTPServer();

	MimeTypeManager& GetMimeTypeManager();
	RequestHandler& GetRequestHandler();

	void RegisterDefaultHandlers();
	void Run();

private:
	void do_accept();
	void do_await_stop();
	void StopSession(std::shared_ptr<Session> const& pSession);

	boost::asio::io_service m_Service;
	boost::asio::signal_set m_Signals;
	boost::asio::ip::tcp::acceptor m_Acceptor;
	boost::asio::ip::tcp::socket m_ServerSocket;
	MimeTypeManager m_MimeTypeManager;
	RequestHandler m_RequestHandler;
	std::set<std::shared_ptr<Session>> m_Sessions;
};
