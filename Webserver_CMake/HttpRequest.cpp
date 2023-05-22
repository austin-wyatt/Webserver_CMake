#include "HttpRequest.h"
#include "HttpResponse.h"
#include "Endpoints.h"
#include "Definitions.h"
#include <iostream>

using namespace std;

void HttpRequest::BuildFromBuffer(char* buffer, int bufferLen) 
{
    vector<string> lines = vector<string>();

    const char* prevBufPos = buffer;
    const char* currBufPos = strchr(buffer, CRLF);
    const char* httpBodyPos = nullptr;

    string_view str = string_view(buffer);

    while (currBufPos != NULL) 
    {
        string line = string(str.substr(prevBufPos - buffer, currBufPos - prevBufPos));

        if (line.size() == 0) 
        {
            currBufPos += CRLF_LEN;

            httpBodyPos = currBufPos;
            break;
        }

        lines.push_back(line);

        currBufPos += CRLF_LEN; //\r\n
        prevBufPos = currBufPos;
        currBufPos = strchr(currBufPos, CRLF);
    }

    if (lines.size() == 0)
        return;

    long long requestType = 0;

    prevBufPos = lines[0].c_str();
    currBufPos = strchr(prevBufPos, ' ');

    long long bufOffset = currBufPos - prevBufPos;

    memcpy(&requestType, buffer, bufOffset);

    Type = (REQUEST_TYPE)requestType;

    //Get the resource string
    currBufPos++;
    prevBufPos = currBufPos;
    currBufPos = strchr(currBufPos, ' ');

    str = string_view(prevBufPos);
    Resource = string(str.substr(0, currBufPos - prevBufPos));

    //Get the protocol version
    ProtocolVersion = string(lines[0].substr(lines[0].find_last_of(' ') + 1));

    for(auto line : lines)
    {
        int delimiterIndex = line.find(':');

        if (delimiterIndex != -1) 
        {
            string key = line.substr(0, delimiterIndex);
            delimiterIndex += 2; 

            string value = line.substr(delimiterIndex);

            Headers->insert({ key, value });
        }
    }

    if (Headers->count("Accepted-Encoding") == 1) 
    {
        //TEMP: only accept deflate encoding for now
        if(Headers->at("Accepted-Encoding").find(ENCODING_STRINGS.at(ENCODING_TYPE::Deflate)) != string::npos)
        {
            AcceptedEncodings->push_back(ENCODING_TYPE::Deflate);
        }
    }

    if (Headers->count("Content-Encoding") == 1)
    {
        //TEMP: only accept deflate encoding for now
        if (Headers->at("Content-Encoding").find(ENCODING_STRINGS.at(ENCODING_TYPE::Deflate)) != string::npos)
        {
            PacketEncoding = ENCODING_TYPE::Deflate;
        }
    }

    if (Headers->count("Content-Length") == 1 && httpBodyPos != nullptr) 
    {
        int contentLength = 0;

        try
        {
            contentLength = stoi(Headers->at("Content-Length"));
        }
        catch (exception e) 
        {

        }

        int remainingSpaceInBuffer = bufferLen - (httpBodyPos - buffer);

        if (contentLength > 0 && remainingSpaceInBuffer >= contentLength) 
        {
            MessageBody = new char[contentLength];
            memset(MessageBody, 0, contentLength);

            memcpy((void*)httpBodyPos, MessageBody, contentLength);
        }
    }
}

bool HttpRequest::HandleRequest()
{
    switch (Type) 
    {
        case REQUEST_TYPE::Get:
            ResolveGetRequest(this);

            return false;
            break;
    }

    return false;
}