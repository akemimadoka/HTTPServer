#pragma once
#include <natType.h>
#include "Header.h"
#include <vector>

struct Request final
{
	std::string Method;
	std::string Uri;
	nuInt Http_version_major;
	nuInt Http_version_minor;
	std::vector<Header> Headers;
};
