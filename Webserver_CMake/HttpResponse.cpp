#include "HttpResponse.h"
#include "Definitions.h"

#include <fstream>
#include <filesystem>

using namespace std;

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

	FILE* file = fopen(path.c_str(), "rb");

	if (file == NULL) 
	{
#ifdef DEBUG
		std::cout << "File not found: " << path << endl;
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

	bool shouldCompress = false;
	auto mappedCompressBool = ContentTypeCompressMap.find(contentType);
	if (mappedCompressBool != ContentTypeCompressMap.end())
	{
		shouldCompress = mappedCompressBool->second;
	}

	if (shouldCompress) 
		PacketEncoding = ENCODING_TYPE::Deflate;
	else
		PacketEncoding = ENCODING_TYPE::None;
		

	switch (PacketEncoding) 
	{
		case ENCODING_TYPE::Deflate:
		{
			//If we should compress this data, add the Deflate Content-Encoding header
			if(shouldCompress)
				AppendHeader("Content-Encoding", ENCODING_STRINGS.at(ENCODING_TYPE::Deflate));

			AppendHeader("Transfer-Encoding", TRANSFER_ENCODING_STRINGS.at(TRANSFER_ENCODING::Chunked));
			TransferEncoding = TRANSFER_ENCODING::Chunked;
			break;
		}
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
	std::cout << "RESPONSE" << endl;
	std::cout << "+++++++++++++++++++++++" << endl;
	std::cout << responseString << endl;
#endif

	switch (TransferEncoding) 
	{
		case TRANSFER_ENCODING::Chunked:
			HandleChunkedFileTransfer(file, size, request, path, shouldCompress);
			break;
		case TRANSFER_ENCODING::None:
		default:
			HandleBasicFileTransfer(file, size, request, path);
			break;
	}

	fclose(file);
}


void HttpResponse::HandleBasicFileTransfer(FILE* file, uintmax_t fileSize, HttpRequest* request, string path)
{
	char* sendBuffer = new char[SEND_BLOCK_SIZE];
	ZeroMemory(sendBuffer, SEND_BLOCK_SIZE);

	//file->seekg(0, ios::beg);

	int readData = 0;
	int sentData = 0;
	int i = 0;

	while (true)
	{
		int readSize = fileSize - i;
		readSize = readSize > SEND_BLOCK_SIZE ? SEND_BLOCK_SIZE : readSize;

		readData += fread(sendBuffer, 1, readSize, file);

		sentData += send(request->ClientSocket, sendBuffer, readSize, 0);

		i += SEND_BLOCK_SIZE;

		if (readSize < SEND_BLOCK_SIZE)
			break;
	}

	#ifdef DEBUG
		std::cout << "File " << path << " transferred" << endl;
		std::cout << "________________" << endl;
		std::cout << readData << " Bytes transferred" << endl;
		std::cout << sentData << " Bytes sent" << endl;
		std::cout << "Mode: Basic" << endl << endl;
	#endif

	delete[] sendBuffer;
}


void HttpResponse::HandleChunkedFileTransfer(FILE* file, uintmax_t fileSize, HttpRequest* request, string path, bool shouldCompress)
{
	//leave 16 bytes of padding in the send buffer to accomodate the chunk size header
	const int SIZE_HEADER_PADDING = 16;
	//leave 2 bytes for the final /r/n
	const int END_PADDING = 2;

	char* sendBuffer = new char[COMPRESS_CHUNK_SIZE + SIZE_HEADER_PADDING + END_PADDING];

	stringstream sizeHeader = stringstream();

	int readData = 0;
	int sentData = 0;

	//COMPRESSION HANDLED HERE
	//-----------------
	z_stream* strm;
	unsigned char* in = NULL;
	unsigned char* out = NULL;

	int flush, ret;
	unsigned int have;

	if (shouldCompress) 
	{
		CompressionHandler::GetDeflateInstance(strm, in, out);

		int sendBufDataSize = 0;

		do 
		{
			strm->avail_in = fread(in, 1, COMPRESS_CHUNK_SIZE, file);

			readData += strm->avail_in;

			if (ferror(file)) 
			{
				deflateEnd(strm);
			}

			flush = feof(file) ? Z_FINISH : Z_NO_FLUSH;
			strm->next_in = in;

			do 
			{
				strm->avail_out = COMPRESS_CHUNK_SIZE;
				strm->next_out = out;

				ret = deflate(strm, flush);
				if(ret == Z_STREAM_ERROR)
					throw runtime_error("Compression step for file " + path + " failed.");

				have = COMPRESS_CHUNK_SIZE - strm->avail_out;

				//Buffer will overflow from compressed data, handle overflow here
				if (have + sendBufDataSize > COMPRESS_CHUNK_SIZE) 
				{
					//fill the buffer as much as possible
					int overflowAmount = COMPRESS_CHUNK_SIZE - sendBufDataSize;

					memcpy(sendBuffer + SIZE_HEADER_PADDING + sendBufDataSize, out, overflowAmount);
					have = have - overflowAmount;

					//insert headers
					//HANDLE CHUNK HEADERS
					//---------
					sizeHeader.str("");
					//convert the size of the chunk into hex and append the CRLF string
					sizeHeader << hex << COMPRESS_CHUNK_SIZE << dec << CRLF_STR;
					auto headerStr = sizeHeader.str();
					int sizeHeaderOffset = SIZE_HEADER_PADDING - headerStr.size();

					//place the size header in the allocated padding area of the buffer
					memcpy(sendBuffer + sizeHeaderOffset, headerStr.c_str(), headerStr.size());
					//insert the final CRLF
					memcpy(sendBuffer + COMPRESS_CHUNK_SIZE + SIZE_HEADER_PADDING, CRLF_STR, CRLF_LEN);
					//---------
					//END HEADERS
					 
					//send data
					sentData += send(request->ClientSocket, sendBuffer + sizeHeaderOffset, 
						headerStr.size() + COMPRESS_CHUNK_SIZE + CRLF_LEN, 0);
					
					//move remaining have data into sendBuffer
					memcpy(sendBuffer + SIZE_HEADER_PADDING, out + overflowAmount, have);

					sendBufDataSize = have;
					have = 0;
				}
				//If buffer is not going to overflow, simply copy data into the send buffer
				else 
				{
					memcpy(sendBuffer + SIZE_HEADER_PADDING + sendBufDataSize, out, have);
					sendBufDataSize += have;
				}
			} 
			while (strm->avail_out == 0);

			if (flush != Z_FINISH)
				continue;

			//HANDLE CHUNK HEADERS
			//---------
			sizeHeader.str("");
			//convert the size of the chunk into hex and append the CRLF string
			sizeHeader << hex << sendBufDataSize << dec << CRLF_STR;
			auto headerStr = sizeHeader.str();
			int sizeHeaderOffset = SIZE_HEADER_PADDING - headerStr.size();

			//place the size header in the allocated padding area of the buffer
			memcpy(sendBuffer + sizeHeaderOffset, headerStr.c_str(), headerStr.size());
			//insert the final CRLF
			memcpy(sendBuffer + sendBufDataSize + SIZE_HEADER_PADDING, CRLF_STR, CRLF_LEN);
			//---------
			//END HEADERS

			sentData += send(request->ClientSocket, sendBuffer + sizeHeaderOffset, headerStr.size() + sendBufDataSize + CRLF_LEN, 0);

			sendBufDataSize = 0;
		} 
		while (flush != Z_FINISH);

		//end the transfer
		sentData += send(request->ClientSocket, "0\r\n\r\n", 5, 0);
	}
	//-----------------
	//END COMPRESSION
	
	int chunkSize;
	while(true && !shouldCompress)
	{
		//empty the header
		sizeHeader.str("");

		//read from file
		chunkSize = fileSize - readData;
		chunkSize = chunkSize > COMPRESS_CHUNK_SIZE ? COMPRESS_CHUNK_SIZE : chunkSize;

		readData += fread(sendBuffer + SIZE_HEADER_PADDING, 1, chunkSize, file);

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

	if (shouldCompress)
		CompressionHandler::ReturnDeflateInstance(strm, in, out);

	#ifdef DEBUG
		std::cout << "File " << path << " transferred" << endl;
		std::cout << "________________" << endl;
		std::cout << readData << " Bytes transferred" << endl;
		std::cout << sentData << " Bytes sent" << endl;
		std::cout << "Mode: Chunked" << endl << endl;
	#endif

	delete[] sendBuffer;
}