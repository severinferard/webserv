#ifndef RESPONSE_HPP
#define RESPONSE_HPP

# include <string>
# include <map>
# include <iostream>
# include "utils.hpp"
# include <sys/socket.h>

#define HTTP_STATUS_200 "200 OK"
#define HTTP_STATUS_404 "404 Not Found"

class Response
{
    private:
        std::string _body;
        std::map<std::string, std::string> _headers;
        std::string _payload;
        int _status;

        static void _initHttpStatus(void);
    
    public:
        Response();
        ~Response();

        static std::map<int, std::string> HTTP_STATUS;

        void setHeader(std::string fieldName, std::string value);
        void setStatus(int status);
        void appendToBody(std::string str);
        void send(int fd);

};

#endif /* RESPONSE_HPP */
