#pragma once
#include "public.h"
#include <cstdint>
#include <map>
#include <vector>
#include <iostream>

using namespace std;

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

class HttpRequest 
{
public:
	REQUEST_TYPE Type = REQUEST_TYPE::None;
	string ProtocolVersion;
	string Resource;
	map<string, string>* Headers = new map<string, string>();
	char* MessageBody = nullptr;

	SOCKET ClientSocket;

	void BuildFromBuffer(char* buffer, int bufferLen);

	void Print()
	{
		cout << "REQUEST" << endl;
		cout << "--------------------------------" << endl;
		cout << "Type: " << REQUEST_STRINGS[Type] << endl;
		cout << "Resource: " << Resource << endl;

		for (auto& item : *Headers)
		{
			cout << item.first << ": " << item.second << endl;
		}

		cout << endl;
	}

	bool HandleRequest();

	~HttpRequest() 
	{
		delete Headers;

		if(MessageBody != nullptr)
		{
			delete MessageBody;
		}
	}
};