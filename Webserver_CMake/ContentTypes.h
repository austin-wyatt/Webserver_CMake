#pragma once

#ifdef _WIN32
#include <string>
#else
#include <string.h>
#endif

#include <unordered_map>

static std::unordered_map<std::string, std::string> ContentTypeMap = std::unordered_map<std::string, std::string>
{
	//text
	{"css", "text/css"},
	{"csv", "text/csv"},
	{"html", "text/html"},
	{"htm", "text/html"},
	{"txt", "text/plain"},
	{"xml", "text/xml"},

	//video
	{"mp4", "video/mp4"},
	{"mpeg", "video/mpeg"},
	{"webm", "video/webm"},

	//image
	{"gif", "image/gif"},
	{"jpeg", "image/jpeg"},
	{"jpg", "image/jpeg"},
	{"png", "image/png"},
	{"tiff", "image/tiff"},
	{"ico", "image/x-icon"},

	//audio
	{"mpeg", "audio/mpeg"},
	{"wav", "audio/x-wav"},

	//application
	{"js", "application/javascript"},
	{"pdf", "application/pdf"},
	{"ogg", "application/ogg"},
	{"json", "application/json"},
	{"zip", "application/zip"},
};