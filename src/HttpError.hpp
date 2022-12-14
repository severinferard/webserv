#ifndef HTTPERROR_HPP
#define HTTPERROR_HPP

#define DEFAULT_ERROR_PAGES_ROOT "./www/errors/"

#define HTTP_STATUS_CONTINUE 100
#define HTTP_STATUS_SUCCESS 200
#define HTTP_STATUS_CREATED 201
#define HTTP_STATUS_MOVED_PERMANENTLY 301
#define HTTP_STATUS_BAD_REQUEST 400
#define HTTP_STATUS_FORBIDDEN 403
#define HTTP_STATUS_NOT_FOUND 404
#define HTTP_STATUS_METHOD_NOT_ALLOWED 405
#define HTTP_STATUS_REQUEST_TIMEOUT 408
#define HTTP_STATUS_LENGTH_REQUIRED 411
#define HTTP_STATUS_PAYLOAD_TOO_LARGE 413
#define HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE 415
#define HTTP_STATUS_REQUEST_HEADER_FIELD_TOO_LARGE 431
#define HTTP_STATUS_INTERNAL_SERVER_ERROR 500
#define HTTP_STATUS_NOT_IMPLEMENTED 501
#define HTTP_STATUS_GATEWAY_TIMEOUT 504
#define HTTP_STATUS_VERSION_NOT_SUPPORTED 505

#define QUOTE(name) #name
#define STR(macro) QUOTE(macro)

extern const char *HTTP_ERRORS_STR[];

class HttpError : public std::runtime_error
{
public:
	int status;
	HttpError(int status) : std::runtime_error("Http Error"), status(status){};
	~HttpError() throw() {}
};

class Expect100 : public std::runtime_error
{
public:
	Expect100() : std::runtime_error("Http Error") {}
	~Expect100() throw() {}
};

#endif /* HTTPERROR_HPP */
