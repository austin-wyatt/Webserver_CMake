#include <zlib.h>
#include <vector>
#include <stdexcept>
#include <mutex>
#include <iostream>

using namespace std;

static class CompressionHandler 
{
public:
	static void Initialize()
	{
		if (_initialized) 
			return;
		_initialized = true;

		for (int i = 0; i < DEFLATE_INSTANCES; i++)
		{
			//two per deflate instance
			_dataChunks.push_back(new unsigned char[COMPRESS_CHUNK_SIZE]);
			_dataChunks.push_back(new unsigned char[COMPRESS_CHUNK_SIZE]);

			int ret;
			z_stream* strm = new z_stream();
			/* allocate deflate state */
			strm->zalloc = Z_NULL;
			strm->zfree = Z_NULL;
			strm->opaque = Z_NULL;
			ret = deflateInit(strm, Z_DEFAULT_COMPRESSION);
			if (ret != Z_OK)
			{
				throw runtime_error("unable to allocate memory for zlib compression stream");
			}
			else
				_deflateInstances.push_back(strm);
		}
	}

	~CompressionHandler() 
	{
		while(_deflateInstances.size() > 0)
		{
			auto strm = _deflateInstances.at(_deflateInstances.size() - 1);
			delete strm;

			_deflateInstances.pop_back();
		}

		while (_dataChunks.size() > 0)
		{
			auto chunk = _dataChunks.at(_dataChunks.size() - 1);
			delete[] chunk;

			_dataChunks.pop_back();
		}
	}

	bool GetDeflateInstance(z_stream* stream, unsigned char* in, unsigned char* out)
	{
		bool success = false;
		_deflateInstanceLock.lock();

		if (_deflateInstances.size() > 0 && _dataChunks.size() > 1)
		{
			stream = _deflateInstances.back();
			_deflateInstances.pop_back();

			in = _dataChunks.back();
			_dataChunks.pop_back();

			out = _dataChunks.back();
			_dataChunks.pop_back();

			success = true;
		}

		_deflateInstanceLock.unlock();
		return success;
	};

	void ReturnDeflateInstance(z_stream* stream, unsigned char* in, unsigned char* out) 
	{
		_deflateInstanceLock.lock();

		_deflateInstances.push_back(stream);
		_dataChunks.push_back(in);
		_dataChunks.push_back(out);

		_deflateInstanceLock.unlock();
	}

private:
	static const int DEFLATE_INSTANCES = 10;
	static bool _initialized;
	static vector<z_stream*> _deflateInstances;
	static vector<unsigned char*> _dataChunks;
	
	static mutex _deflateInstanceLock;
};

//Initialize static variables
bool CompressionHandler::_initialized = false;
vector<z_stream*> CompressionHandler::_deflateInstances = vector<z_stream*>();
vector<unsigned char*> CompressionHandler::_dataChunks = vector<unsigned char*>();