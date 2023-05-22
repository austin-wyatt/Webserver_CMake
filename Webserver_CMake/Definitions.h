#pragma once

#ifdef _WIN32
#include <string>
#else
#include <string.h>
#endif

#include <unordered_map>
#include <map>

using namespace std;

static unordered_map<string, string> ContentTypeMap = unordered_map<string, string>
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

static unordered_map<string, bool> ContentTypeCompressMap = unordered_map<string, bool>
{
	//text
	{"text/css", true},
	{"text/csv", true},
	{"text/html", true},
	{"text/plain", true},
	{"text/xml", true},

	//video
	{"video/mp4", false},
	{"video/mpeg", false},
	{"video/webm", false},

	//image
	{"image/gif", false},
	{"image/jpeg", false},
	{"image/png", false},
	{"image/tiff", false},
	{"image/x-icon", false},

	//audio
	{"audio/mpeg", false},
	{"audio/x-wav", true},

	//application
	{"application/javascript", true},
	{"application/pdf", false},
	{"application/ogg", false},
	{"application/json", true},
	{"application/zip", false},
};

const long long GET_CODE = 5522759; //*(long long*)"GET\0\0\0\0"
const long long POST_CODE = 1414745936; //*(long long*)"POST\0\0\0"
const long long PUT_CODE = 5526864; //*(long long*)"PUT\0\0\0\0"
const long long DELETE_CODE = 76228242195780; //*(long long*)"DELETE\0"
const long long TRACE_CODE = 297481097812; //*(long long*)"TRACE\0\0"
const long long OPTIONS_CODE = 23448525506629711; //*(long long*)"OPTIONS"
const long long CONNECT_CODE = 23717862989254467; //*(long long*)"CONNECT"



enum class REQUEST_TYPE : long long
{
	None = 0,
	Get = GET_CODE,
	Post = POST_CODE,
	Put = PUT_CODE,
	Delete = DELETE_CODE,
	Trace = TRACE_CODE,
	Options = OPTIONS_CODE,
	Connect = CONNECT_CODE
};

static map<REQUEST_TYPE, string> REQUEST_STRINGS = map<REQUEST_TYPE, string>
{
	{REQUEST_TYPE::None, "None"},
	{REQUEST_TYPE::Get, "Get"},
	{REQUEST_TYPE::Post, "Post"},
	{REQUEST_TYPE::Put, "Put"},
	{REQUEST_TYPE::Delete, "Delete"},
	{REQUEST_TYPE::Trace, "Trace"},
	{REQUEST_TYPE::Options, "Options"},
	{REQUEST_TYPE::Connect, "Connect"}
};

enum class ENCODING_TYPE : short
{
	None = 0,
	GZIP = 1,
	BR = 2,
	Deflate = 4
};

static map<ENCODING_TYPE, string> ENCODING_STRINGS = map<ENCODING_TYPE, string>
{
	{ENCODING_TYPE::None, "None"},
	{ENCODING_TYPE::GZIP, "gzip"},
	{ENCODING_TYPE::BR, "br"},
	{ENCODING_TYPE::Deflate, "deflate"},
};

static map<string, ENCODING_TYPE> ENCODING_STRINGS_LOOKUP = map<string, ENCODING_TYPE>
{
	{"None", ENCODING_TYPE::None},
	{"gzip", ENCODING_TYPE::GZIP},
	{"br", ENCODING_TYPE::BR},
	{"deflate", ENCODING_TYPE::Deflate},
};


enum class TRANSFER_ENCODING : short 
{
	None = 0,
	Chunked = 1,
	Compress = 2,
	Deflate = 4,
	GZIP = 8
};

static map<TRANSFER_ENCODING, string> TRANSFER_ENCODING_STRINGS = map<TRANSFER_ENCODING, string>
{
	{TRANSFER_ENCODING::None, ""},
	{TRANSFER_ENCODING::Chunked, "chunked"},
	{TRANSFER_ENCODING::Compress, "compress"},
	{TRANSFER_ENCODING::GZIP, "gzip"},
};