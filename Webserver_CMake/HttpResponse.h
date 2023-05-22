#pragma once

#include "HttpRequest.h"
#include <sstream>

using namespace std;

class HttpResponse
{
public:
	string GetResponseString();

	//This ideally should support multiple encoding types but for the moment it will only support none and deflate
	ENCODING_TYPE PacketEncoding = ENCODING_TYPE::None;

	//This ideally should support multiple transfer types but for the moment it will only support chunked and none
	TRANSFER_ENCODING TransferEncoding = TRANSFER_ENCODING::None;

	void AppendStatusLine(string protocolVersion, int response, string statusText);
	void AppendHeader(string header, string value);
	void EndHeaders();
	void AppendBodyData(char* bodyData);

	void SendFile(string path, HttpRequest* request);
private:
	void HandleBasicFileTransfer(ifstream* file, uintmax_t fileSize, HttpRequest* request, string path);
	void HandleChunkedFileTransfer(ifstream* file, uintmax_t fileSize, HttpRequest* request, string path, bool shouldCompress);

	stringstream responseStream = stringstream();
};