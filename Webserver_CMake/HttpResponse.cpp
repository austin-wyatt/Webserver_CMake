#include "HttpResponse.h"
#include "ContentTypes.h"
#include <fstream>
#include <filesystem>

string HttpResponse::GetResponseString() 
{
	string str = responseStream.str();
	return str;
}

void HttpResponse::AppendStatusLine(string protocolVersion, int response, string statusText) 
{
	responseStream << protocolVersion << " " << to_string(response) << " " << statusText << CRLF_STR;
}

void HttpResponse::AppendHeader(string header, string value) 
{
	responseStream << header << ": " << value << CRLF_STR;
}

void HttpResponse::EndHeaders() 
{
	responseStream << CRLF_STR;
}

void HttpResponse::AppendBodyData(char* bodyData) 
{
	responseStream << bodyData;
}

void HttpResponse::SendFile(string path, HttpRequest* request) 
{
	int fileExtensionIndex = path.find_last_of('.');

	string contentType = "text/plain";

	if (fileExtensionIndex != -1 || fileExtensionIndex != path.size() - 1) 
	{
		string fileExtension = path.substr(fileExtensionIndex + 1);

		auto foundType = ContentTypeMap.find(fileExtension);
		if (foundType != ContentTypeMap.end())
		{
			contentType = foundType->second;
		}
	}

	string responseString;

	ifstream file(path, ios::binary);

	if (!file.is_open()) 
	{
#ifdef DEBUG
		cout << "File not found: " << path << endl;
#endif

		AppendStatusLine(request->ProtocolVersion, 404, "Resource not found");
		AppendHeader("Connection", "close");
		EndHeaders();

		responseString = GetResponseString();
		send(request->ClientSocket, responseString.c_str(), responseString.size(), 0);

		return;
	}

	auto size = filesystem::file_size(path);

	AppendStatusLine(request->ProtocolVersion, 200, "");

	AppendHeader("Content-Size", to_string(size));
	AppendHeader("Content-Type", contentType);

	EndHeaders();

	responseString = GetResponseString();
	send(request->ClientSocket, responseString.c_str(), responseString.size(), 0);

#ifdef DEBUG
	cout << "RESPONSE" << endl;
	cout << "+++++++++++++++++++++++" << endl;
	cout << responseString << endl;
#endif

	char* sendBuffer = new char[SEND_BLOCK_SIZE];
	ZeroMemory(sendBuffer, SEND_BLOCK_SIZE);

	file.seekg(0, ios::beg);

	int readData = 0;
	int sentData = 0;
	int i = 0;

	while(true)
	{
		int readSize = size - i;
		readSize = readSize > SEND_BLOCK_SIZE ? SEND_BLOCK_SIZE : readSize;

		readData += readSize;

		file.read(sendBuffer, readSize);

		sentData += send(request->ClientSocket, sendBuffer, readSize, 0);

		i += SEND_BLOCK_SIZE;

		if (readSize < SEND_BLOCK_SIZE)
			break;
	}

#ifdef DEBUG
	cout << "File " << path << " transferred" << endl;
	cout << "________________" << endl;
	cout << readData << " Bytes transferred" << endl << endl;
	cout << sentData << " Bytes sent" << endl << endl;
#endif

	file.close();

	delete[] sendBuffer;
}