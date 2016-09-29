#pragma once
#include <natType.h>
#include "MimeTypes.h"
#include <functional>

struct Reply;
struct Request;

class RequestHandler final
{
public:
	typedef NatsuLib::Delegate<void(Request const&)> LogFuncType;
	typedef NatsuLib::Delegate<Reply(Request const&)> MethodHandler;

	explicit RequestHandler(MimeTypeManager const& mimeTypeManager, std::string const& rootdir, ncStr defaultFile = "index.html");

	std::string const& GetRootDir() const noexcept;
	Reply HandleRequest(Request const& request) const;
	LogFuncType SetLogFunc(LogFuncType logfunc);

private:
	MimeTypeManager const& m_MimeTypeManager;
	std::string m_RootDir;
	std::string m_DefaultFile;
	LogFuncType m_Logfunc;
	std::unordered_map<std::string, MethodHandler> m_MethodHandler;
};
