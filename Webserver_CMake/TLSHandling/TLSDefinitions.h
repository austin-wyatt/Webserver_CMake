#pragma once

namespace TLS
{
	struct ChangeCipherSpec 
	{
		enum type : char
		{
			change_cipher_spec = 1
		};
	};

	enum ContentType : char
	{
		change_cipher_spec = 20,
		alert = 21,
		handshake = 22,
		application_data = 23
	};

	struct ProtocolVersion
	{
		unsigned char major;
		unsigned char minor;
	};


	enum HandshakeType : char
	{
		hello_request = 0,
		client_hello = 1,
		server_hello = 2,
		certificate = 11,
		server_key_exchange = 12,
		certificate_request = 13,
		server_hello_done = 14,
		certificate_verify = 15,
		client_key_exchange = 16,
		finished = 20
	};

	struct Handshake
	{
		HandshakeType msg_type;
		unsigned int length;
	};

	struct TLSPlaintext
	{
		ContentType type;
		ProtocolVersion version;
		unsigned short length;
		unsigned char* fragment;

		TLSPlaintext(char* buffer) 
		{
			*this = *(TLSPlaintext*)buffer;
			fragment = new unsigned char[length];

			memcpy(fragment, buffer + sizeof(ContentType) + sizeof(ProtocolVersion) + sizeof(short), length);
		}

		~TLSPlaintext()
		{
			delete[] fragment;
		}
	};

	enum AlertLevel : char 
	{
		warning = 1,
		fatal = 2
	};

	enum AlertDescription : char
	{
		close_notify = 0,
		unexpected_message = 10,
		bad_record_mac = 20,
		decryption_failed_RESERVED = 21,
		record_overflow = 22,
		decompression_failure = 30,
		handshake_failure = 40,
		no_certificate_RESERVED = 41,
		bad_certificate = 42,
		unsupported_certificate = 43,
		certificate_revoked = 44,
		certificate_expired = 45,
		certificate_unknown = 46,
		illegal_parameter = 47,
		unknown_ca = 48,
		access_denied = 49,
		decode_error = 50,
		decrypt_error = 51,
		export_restriction_RESERVED = 60,
		protocol_version = 70,
		insufficient_security = 71,
		internal_error = 80,
		user_canceled = 90,
		no_renegotiation = 100,
		unsupported_extension = 110,
	};

	struct Alert 
	{
		AlertLevel level;
		AlertDescription description;
	};
}