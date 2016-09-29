#include "PCH.h"
#include "MimeTypes.h"
#include <filesystem>

namespace
{
	constexpr char DefaultMimeType[] = "text/plain";
}

void MimeTypeManager::RegisterMimeType(std::string const& extension, std::string const& mimeType)
{
	m_MimeTypeMap[extension] = mimeType;
}

void MimeTypeManager::RegisterMimeTypeHandler(std::string const& mimeType, MimeTypeHandler handler)
{
	m_MimeTypeHandlerMap[mimeType] = handler;
}

std::string MimeTypeManager::ExtensionToType(std::string const& extension) const
{
	auto iter = m_MimeTypeMap.find(extension);
	if (iter == m_MimeTypeMap.end())
	{
		return DefaultMimeType;
	}

	return iter->second;
}

Reply MimeTypeManager::HandleMime(std::string const& uri) const
{
	auto type = ExtensionToType(std::experimental::filesystem::path{ uri }.extension().string().substr(1));
	auto iter = m_MimeTypeHandlerMap.find(type);
	return (iter == m_MimeTypeHandlerMap.end() ? m_DefaultHandler : iter->second)(type, uri);
}

MimeTypeManager::MimeTypeHandler MimeTypeManager::SetDefaultHandler(MimeTypeHandler newHandler)
{
	return exchange(m_DefaultHandler, newHandler);
}
