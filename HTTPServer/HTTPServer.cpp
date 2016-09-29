#include "PCH.h"
#include "HTTPServer.h"
#include <natUtil.h>
#include <natRefObj.h>
#include <natStream.h>
#include <filesystem>

HTTPServer::Session::Session(boost::asio::ip::tcp::socket && socket, HTTPServer& server)
	: m_Socket(std::move(socket)), m_Server(server)
{
}

HTTPServer::Session::~Session()
{
	Stop();
}

void HTTPServer::Session::Start()
{
	do_read();
}

void HTTPServer::Session::Stop()
{
	if (m_Socket.is_open())
		m_Socket.close();
}

void HTTPServer::Session::do_read()
{
	m_Socket.async_read_some(boost::asio::buffer(m_Buffer), [this, self = shared_from_this()](boost::system::error_code const& ec, std::size_t bytes_transferred)
	{
		if (!ec)
		{
			RequestParser::ResultType result;
			tie(result, std::ignore) = m_RequestParser.Parse(m_Request, std::begin(m_Buffer), std::end(m_Buffer));

			switch (result)
			{
			case RequestParser::good:
				m_Reply = m_Server.m_RequestHandler.HandleRequest(m_Request);
				do_write();
				break;
			case RequestParser::bad:
				m_Reply = Reply::ShockReply(Reply::bad_request);
				do_write();
				break;
			default:
				do_read();
				break;
			}
		}
		else if (ec != boost::asio::error::operation_aborted)
		{
			m_Server.StopSession(self);
		}
	});
}

void HTTPServer::Session::do_write()
{
	boost::asio::async_write(m_Socket, m_Reply.GetBuffers(), [this, self = shared_from_this()](boost::system::error_code const& ec, std::size_t)
	{
		if (!ec)
		{
			boost::system::error_code ignored{};
			m_Socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored);
		}

		if (ec != boost::asio::error::operation_aborted)
		{
			m_Server.StopSession(self);
		}
	});
}

HTTPServer::HTTPServer(ncTStr rootDir, ncTStr address, ncTStr port, ncTStr defaultFile)
	: m_Signals(m_Service), m_Acceptor(m_Service), m_ServerSocket(m_Service), m_RequestHandler(m_MimeTypeManager, NatsuLib::natUtil::WidecharToMultibyte(rootDir), defaultFile ? NatsuLib::natUtil::WidecharToMultibyte(defaultFile).c_str() : "index.html")
{
	m_Signals.add(SIGINT);
	m_Signals.add(SIGTERM);

	do_await_stop();

	boost::asio::ip::tcp::resolver resolver(m_Service);
	boost::asio::ip::tcp::endpoint endpoint{ *resolver.resolve({ NatsuLib::natUtil::W2Cstr(address), NatsuLib::natUtil::W2Cstr(port) }) };
	m_Acceptor.open(endpoint.protocol());
	m_Acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
	m_Acceptor.bind(endpoint);
	m_Acceptor.listen();

	do_accept();
}

HTTPServer::~HTTPServer()
{
}

MimeTypeManager& HTTPServer::GetMimeTypeManager()
{
	return m_MimeTypeManager;
}

RequestHandler& HTTPServer::GetRequestHandler()
{
	return m_RequestHandler;
}

void HTTPServer::RegisterDefaultHandlers()
{
	m_MimeTypeManager.RegisterMimeType("gif", "image/gif");
	m_MimeTypeManager.RegisterMimeType("htm", "text/html");
	m_MimeTypeManager.RegisterMimeType("html", "text/html");
	m_MimeTypeManager.RegisterMimeType("jpg", "image/jpeg");
	m_MimeTypeManager.RegisterMimeType("png", "image/png");
	m_MimeTypeManager.SetDefaultHandler([](std::string const& mimeType, std::string const& uri) -> Reply
	{
		NatsuLib::natRefPointer<NatsuLib::natFileStream> pStream;

		try
		{
			pStream = NatsuLib::make_ref<NatsuLib::natFileStream>(std::experimental::filesystem::path{ NatsuLib::natUtil::MultibyteToUnicode(uri.c_str(), CP_UTF8) }.make_preferred().c_str(), true, false);
		}
		catch (std::bad_alloc&)
		{
			return Reply::ShockReply(Reply::internal_server_error);
		}
		catch (...)
		{
			return Reply::ShockReply(Reply::not_found);
		}

		std::vector<nChar> fileContent(static_cast<size_t>(pStream->GetSize()));
		pStream->ReadBytes(reinterpret_cast<nData>(fileContent.data()), pStream->GetSize());

		return{ Reply::ok,{ { "Content-Length", std::to_string(fileContent.size()) },{ "Content-Type", mimeType } },{ begin(fileContent), end(fileContent) } };
	});
}

void HTTPServer::Run()
{
	m_Service.run();
}

void HTTPServer::do_accept()
{
	m_Acceptor.async_accept(m_ServerSocket, [this](boost::system::error_code const& ec)
	{
		if (!m_Acceptor.is_open())
		{
			return;
		}

		if (!ec)
		{
			auto pSession = std::make_shared<Session>(std::move(m_ServerSocket), *this);
			pSession->Start();
			m_Sessions.emplace(move(pSession));
		}

		do_accept();
	});
}

void HTTPServer::do_await_stop()
{
	m_Signals.async_wait([this](boost::system::error_code const& /*ec*/, int /*signo*/)
	{
		m_Acceptor.close();
		for (auto&& item : m_Sessions)
			item->Stop();
		m_Sessions.clear();
		m_Service.stop();
	});
}

void HTTPServer::StopSession(std::shared_ptr<Session> const& pSession)
{
	pSession->Stop();
	m_Sessions.erase(pSession);
}
