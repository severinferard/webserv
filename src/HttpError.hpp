#ifndef HTTPERROR_HPP
#define HTTPERROR_HPP

#define DEFAULT_ERROR_PAGES_ROOT "./src/www/errors/"

#define HTTP_STATUS_SUCCESS					200
#define HTTP_STATUS_CREATED					201
#define HTTP_STATUS_BAD_REQUEST				400
#define HTTP_STATUS_FORBIDDEN				403
#define HTTP_STATUS_NOT_FOUND				404
#define HTTP_STATUS_METHOD_NOT_ALLOWED		405
#define HTTP_STATUS_REQUEST_TIMEOUT			408
#define HTTP_STATUS_LENGTH_REQUIRED			411
#define HTTP_STATUS_PAYLOAD_TOO_LARGE		413
#define HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE	415
#define HTTP_STATUS_INTERNAL_SERVER_ERROR	500
#define HTTP_STATUS_NOT_IMPLEMENTED			501
#define HTTP_STATUS_GATEWAY_TIMEOUT			504
#define HTTP_STATUS_VERSION_NOT_SUPPORTED	505

#define QUOTE(name) #name
#define STR(macro) QUOTE(macro)

const static char * HTTP_ERRORS_STR[] = {
	STR(HTTP_STATUS_BAD_REQUEST),
	STR(HTTP_STATUS_FORBIDDEN),				
	STR(HTTP_STATUS_NOT_FOUND),				
	STR(HTTP_STATUS_METHOD_NOT_ALLOWED),		
	STR(HTTP_STATUS_REQUEST_TIMEOUT),			
	STR(HTTP_STATUS_LENGTH_REQUIRED),			
	STR(HTTP_STATUS_PAYLOAD_TOO_LARGE),		
	STR(HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE),	
	STR(HTTP_STATUS_INTERNAL_SERVER_ERROR),	
	STR(HTTP_STATUS_NOT_IMPLEMENTED),			
	STR(HTTP_STATUS_GATEWAY_TIMEOUT),			
	STR(HTTP_STATUS_VERSION_NOT_SUPPORTED),	
};

class HttpError : public std::runtime_error {
	public:
		int status;
		HttpError(int status): std::runtime_error("Http Error"), status(status){};
		~HttpError() throw() {}
};

#endif /* HTTPERROR_HPP */
