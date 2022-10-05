#ifndef HTTPERROR_HPP
#define HTTPERROR_HPP


#define HTTP_STATUS_SUCCESS					200
#define HTTP_STATUS_BAD_REQUEST				400
#define HTTP_STATUS_NOT_FOUND				404
#define HTTP_STATUS_METHOD_NOT_ALLOWED		405
#define HTTP_STATUS_LENGTH_REQUIRED			411
#define HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE	415

class HttpError : public std::runtime_error {
	public:
		int status;
		HttpError(int status): std::runtime_error("Http Error"), status(status){};
		~HttpError() throw() {}
};

#endif /* HTTPERROR_HPP */
