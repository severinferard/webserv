#ifndef HTTPERROR_HPP
#define HTTPERROR_HPP


#define HTTP_STATUS_SUCCESS					200
#define HTTP_STATUS_BAD_REQUEST				400
#define HTTP_STATUS_NOT_FOUND				404
#define HTTP_STATUS_METHOD_NOT_ALLOWED		405
#define HTTP_STATUS_REQUEST_TIMEOUT			408
#define HTTP_STATUS_LENGTH_REQUIRED			411
#define HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE	415
#define HTTP_STATUS_INTERNAL_SERVER_ERROR	500
#define HTTP_STATUS_NOT_IMPLEMENTED			501
#define HTTP_STATUS_GATEWAY_TIMEOUT			504
#define HTTP_STATUS_VERSION_NOT_SUPPORTED	505

class HttpError : public std::runtime_error {
	public:
		int status;
		HttpError(int status): std::runtime_error("Http Error"), status(status){};
		~HttpError() throw() {}
};

#endif /* HTTPERROR_HPP */
