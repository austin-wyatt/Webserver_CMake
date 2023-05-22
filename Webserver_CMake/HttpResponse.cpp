#include "HttpResponse.h"
#include "Definitions.h"

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

	switch (PacketEncoding) 
	{
		case ENCODING_TYPE::Deflate:
			AppendHeader("Content-Encoding", ENCODING_STRINGS.at(ENCODING_TYPE::Deflate));
			AppendHeader("Transfer-Encoding", TRANSFER_ENCODING_STRINGS.at(TRANSFER_ENCODING::Chunked));
			TransferEncoding = TRANSFER_ENCODING::Chunked;
			break;
		case ENCODING_TYPE::None:
		default:
			AppendHeader("Content-Length", to_string(size));
			break;
	}

	AppendHeader("Content-Type", contentType);

	EndHeaders();

	responseString = GetResponseString();
	send(request->ClientSocket, responseString.c_str(), responseString.size(), 0);

#ifdef DEBUG
	cout << "RESPONSE" << endl;
	cout << "+++++++++++++++++++++++" << endl;
	cout << responseString << endl;
#endif


	bool shouldCompress = false;

	auto mappedCompressBool = ContentTypeCompressMap.find(contentType);
	if (mappedCompressBool != ContentTypeCompressMap.end())
	{
		shouldCompress = mappedCompressBool->second;
	}

	switch (TransferEncoding) 
	{
		case TRANSFER_ENCODING::Chunked:
			HandleChunkedFileTransfer(&file, size, request, path, shouldCompress);
			break;
		case TRANSFER_ENCODING::None:
		default:
			HandleBasicFileTransfer(&file, size, request, path);
			break;
	}

	file.close();
}


void HttpResponse::HandleBasicFileTransfer(ifstream* file, uintmax_t fileSize, HttpRequest* request, string path)
{
	char* sendBuffer = new char[SEND_BLOCK_SIZE];
	ZeroMemory(sendBuffer, SEND_BLOCK_SIZE);

	file->seekg(0, ios::beg);

	int readData = 0;
	int sentData = 0;
	int i = 0;

	while (true)
	{
		int readSize = fileSize - i;
		readSize = readSize > SEND_BLOCK_SIZE ? SEND_BLOCK_SIZE : readSize;

		readData += readSize;

		file->read(sendBuffer, readSize);

		sentData += send(request->ClientSocket, sendBuffer, readSize, 0);

		i += SEND_BLOCK_SIZE;

		if (readSize < SEND_BLOCK_SIZE)
			break;
	}

	#ifdef DEBUG
		cout << "File " << path << " transferred" << endl;
		cout << "________________" << endl;
		cout << readData << " Bytes transferred" << endl;
		cout << sentData << " Bytes sent" << endl;
		cout << "Mode: Basic" << endl << endl;
	#endif

	delete[] sendBuffer;
}


void HttpResponse::HandleChunkedFileTransfer(ifstream* file, uintmax_t fileSize, HttpRequest* request, string path, bool shouldCompress)
{
	//IF compressing, run the file through the compression engine here 
	//----------
	//-
	if (shouldCompress) 
	{
		//do stuff
	}
	//-
	//----------
	//END compression step


	//leave 16 bytes of padding in the send buffer to accomodate the chunk size header
	const int SIZE_HEADER_PADDING = 16;
	//leave 2 bytes for the final /r/n
	const int END_PADDING = 2;

	char* sendBuffer = new char[COMPRESS_CHUNK_SIZE + SIZE_HEADER_PADDING + END_PADDING];
	//ZeroMemory(sendBuffer, COMPRESS_CHUNK_SIZE);

	stringstream sizeHeader = stringstream();

	int readData = 0;
	int sentData = 0;

	int chunkSize;

	while(true)
	{
		//empty the header
		sizeHeader.str("");

		// IF COMPRESSING 
		// --------------
		// 
		if (shouldCompress) 
		{
			//read from compression engine

			chunkSize = 0;
		}
		// ELSE IF READING FROM FILE
		// --------------
		//
		else 
		{
			//read from file
			chunkSize = fileSize - readData;
			chunkSize = chunkSize > COMPRESS_CHUNK_SIZE ? COMPRESS_CHUNK_SIZE : chunkSize;

			readData += chunkSize;

			file->read(sendBuffer + SIZE_HEADER_PADDING, chunkSize);
		}

		//convert the size of the chunk into hex and append the CRLF string
		sizeHeader << hex << chunkSize << dec << CRLF_STR;

		auto headerStr = sizeHeader.str();
		int sizeHeaderOffset = SIZE_HEADER_PADDING - headerStr.size();

		//place the size header in the allocated padding area of the buffer
		memcpy(sendBuffer + sizeHeaderOffset, headerStr.c_str(), headerStr.size());
		//insert the final CRLF
		memcpy(sendBuffer + chunkSize + SIZE_HEADER_PADDING, CRLF_STR, CRLF_LEN);

		sentData += send(request->ClientSocket, sendBuffer + sizeHeaderOffset, headerStr.size() + chunkSize + CRLF_LEN, 0);

		if (chunkSize < COMPRESS_CHUNK_SIZE)
		{
			//end the transfer
			sentData += send(request->ClientSocket, "0\r\n\r\n", 5, 0);
			break;
		}
	}

	#ifdef DEBUG
		cout << "File " << path << " transferred" << endl;
		cout << "________________" << endl;
		cout << readData << " Bytes transferred" << endl;
		cout << sentData << " Bytes sent" << endl;
		cout << "Mode: Chunked" << endl << endl;
	#endif

	delete[] sendBuffer;
}