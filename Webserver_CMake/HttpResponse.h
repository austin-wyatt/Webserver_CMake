#pragma once

#include "HttpRequest.h"
#include <sstream>

using namespace std;

class HttpResponse
{
public:
	string GetResponseString();

	void AppendStatusLine(string protocolVersion, int response, string statusText);
	void AppendHeader(string header, string value);
	void EndHeaders();
	void AppendBodyData(char* bodyData);

	void SendFile(string path, HttpRequest* request);
private:
	stringstream responseStream = stringstream();
};