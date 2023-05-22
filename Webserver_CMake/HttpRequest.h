#pragma once
#include "public.h"
#include "Definitions.h"
#include <cstdint>
#include <map>
#include <vector>
#include <iostream>

using namespace std;

class HttpRequest 
{
public:
	REQUEST_TYPE Type = REQUEST_TYPE::None;
	string ProtocolVersion;
	string Resource;
	map<string, string>* Headers = new map<string, string>();
	char* MessageBody = nullptr;

	vector<ENCODING_TYPE>* AcceptedEncodings = new vector<ENCODING_TYPE>();
	ENCODING_TYPE PacketEncoding = ENCODING_TYPE::None;

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
		delete AcceptedEncodings;

		if(MessageBody != nullptr)
		{
			delete MessageBody;
		}
	}
};