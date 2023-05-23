#include "HttpRequest.h"
#include "HttpResponse.h"
#include "Endpoints.h"


static unordered_map<string, int> _endpointsMap = unordered_map<string, int>
{
	{"/", 0}
};

void ResolveGetRequest(HttpRequest* request) 
{
	int resourceKey = -1;

	auto resourceEntry = _endpointsMap.find(request->Resource);
	if (resourceEntry != _endpointsMap.end())
	{
		resourceKey = resourceEntry->second;
	}

	HttpResponse* response = new HttpResponse();

	for (int i = 0; i < request->AcceptedEncodings->size(); i++) 
	{
		if (request->AcceptedEncodings->at(i) == ENCODING_TYPE::Deflate)
		{
			response->PacketEncoding = ENCODING_TYPE::Deflate;
			break;
		}
	}
	

	switch(resourceKey)
	{
		case 0:
			//return index.html
			response->SendFile("Resources/index.html", request);
			break;
		case -1:
			//generic file request
			response->SendFile("Resources" + request->Resource, request);
			break;
	}

	delete response;
}