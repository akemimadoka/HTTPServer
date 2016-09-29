#include "PCH.h"
#include "RequestHandler.h"
#include <natStream.h>
#include <filesystem>
#include "Reply.h"
#include "Request.h"

using namespace NatsuLib;

namespace
{
	bool UrlDecode(std::string const& in, std::string& out)
	{
		out.clear();
		out.reserve(in.size());

		for (auto i = begin(in); i != end(in); ++i)
		{
			switch (*i)
			{
			case '%':
				if (distance(i, in.end()) > 2)
				{
					try
					{
						out += static_cast<char>(stoi(in.substr(distance(begin(in), next(i)), 2), nullptr, 16));
					}
					catch (...)
					{
						return false;
					}
					advance(i, 2);
				}
				else
				{
					return false;
				}
				break;
			case '+':
				out += ' ';
				break;
			default:
				out += *i;
				break;
			}
		}

		return true;
	}
}

RequestHandler::RequestHandler(MimeTypeManager const& mimeTypeManager, std::string const& rootdir, ncStr defaultFile)
	: m_MimeTypeManager(mimeTypeManager), m_RootDir(rootdir), m_DefaultFile(defaultFile)
{
	m_MethodHandler["GET"] = [this](Request const& request)
	{
		std::string requestPath;
		if (!UrlDecode(request.Uri, requestPath))
		{
			return Reply::ShockReply(Reply::bad_request);
		}

		if (requestPath.empty() || requestPath.front() != '/' || requestPath.find("..") != std::string::npos)
		{
			return Reply::ShockReply(Reply::bad_request);
		}

		if (requestPath.back() == '/')
		{
			requestPath += m_DefaultFile;
		}
		
		return m_MimeTypeManager.HandleMime(m_RootDir + requestPath);
	};
}

std::string const& RequestHandler::GetRootDir() const noexcept
{
	return m_RootDir;
}

Reply RequestHandler::HandleRequest(Request const& request) const
{
	if (m_Logfunc)
	{
		m_Logfunc(request);
	}

	auto iter = m_MethodHandler.find(request.Method);
	if (iter == m_MethodHandler.end())
	{
		return Reply::ShockReply(Reply::bad_request);
	}

	return iter->second(request);
}

RequestHandler::LogFuncType RequestHandler::SetLogFunc(LogFuncType logfunc)
{
	return std::exchange(m_Logfunc, logfunc);
}
