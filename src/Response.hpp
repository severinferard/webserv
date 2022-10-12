#ifndef RESPONSE_HPP
#define RESPONSE_HPP

# include <string>
# include <map>
# include <iostream>
# include "utils.hpp"
# include <sys/socket.h>

class Response
{
    private:
        std::string _body;
        std::map<std::string, std::string> _headers;
        std::string _payload;
        std::string _uri;
        int _status;

        static void _initHttpStatus(void);
        static void _initMimeTypes(void);
	std::string  get_content_type(std::string ext);
    
    public:
        Response();
        ~Response();

        static std::map<int, std::string> HTTP_STATUS;
        static std::map<std::string, std::string> MIME_TYPES;

        void setHeader(std::string fieldName, std::string value);
	void setUri(std::string uri);
        void setStatus(int status);
        void appendToBody(std::string str);
        void send(int fd);

};

#endif /* RESPONSE_HPP */
