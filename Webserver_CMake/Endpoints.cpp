#include "HttpRequest.h"
#include "HttpResponse.h"
#include "Endpoints.h"
#include <unordered_map>

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

	HttpResponse* response;

	switch(resourceKey)
	{
		case 0:
			//return index.html
			response = new HttpResponse();

			response->SendFile("Resources/index.html", request);

			delete response;
			break;
		case -1:
			//generic file request
			response = new HttpResponse();

			response->SendFile("Resources" + request->Resource, request);
			
			delete response;
			break;
	}
}