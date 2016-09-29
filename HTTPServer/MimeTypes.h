#pragma once
#include <unordered_map>
#include "Reply.h"
#include <natDelegate.h>

class MimeTypeManager final
{
public:
	typedef NatsuLib::Delegate<Reply(std::string const&, std::string const&)> MimeTypeHandler;

	void RegisterMimeType(std::string const& extension, std::string const& mimeType);
	void RegisterMimeTypeHandler(std::string const& mimeType, MimeTypeHandler handler);
	std::string ExtensionToType(std::string const& extension) const;
	Reply HandleMime(std::string const& uri) const;
	MimeTypeHandler SetDefaultHandler(MimeTypeHandler newHandler);

private:
	std::unordered_map<std::string, std::string> m_MimeTypeMap;
	std::unordered_map<std::string, MimeTypeHandler> m_MimeTypeHandlerMap;
	MimeTypeHandler m_DefaultHandler;
};
