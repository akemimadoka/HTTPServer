#include "PCH.h"
#include <iostream>
#include "HTTPServer.h"
#include <filesystem>

using namespace NatsuLib::natUtil;

void PrintUsage(std::wostream& os)
{
	os << _T("Usage: HTTPServer <address> <port> [rootdir]") << std::endl;
}

int _tmain(int argc, const nUnsafePtr<ncTStr> argv)
{
	std::locale::global(std::locale("", LC_CTYPE));
	nTString rootdir;

	switch (argc)
	{
	case 3:
	{
		/*rootdir.resize(GetCurrentDirectory(0, nullptr) + 1);
		GetCurrentDirectory(static_cast<DWORD>(rootdir.size() - 1), &rootdir.front());*/
		rootdir = std::experimental::filesystem::current_path();
	}
		break;
	case 4:
		rootdir = argv[3];
		break;
	default:
		PrintUsage(std::wcerr);
		return EXIT_FAILURE;
	}

	HTTPServer server(rootdir.c_str(), argv[1], argv[2]);
	server.RegisterDefaultHandlers();
	server.GetRequestHandler().SetLogFunc([](Request const& request)
	{
		std::wclog << FormatString(_T("Method: {0}\nHTTP version: {1}.{2}\nURI: {3}\nHeaders:"), C2Wstr(request.Method), request.Http_version_major, request.Http_version_minor, C2Wstr(request.Uri)) << std::endl;
		for (auto&& item : request.Headers)
		{
			std::wclog << FormatString(_T("Name: {0}, Value: {1}"), C2Wstr(item.Name), C2Wstr(item.Value)) << std::endl;
		}
		std::wclog << std::endl;
	});

	server.Run();
}
