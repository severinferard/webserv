#ifndef RESPONSE_HPP
#define RESPONSE_HPP

# include <string>
# include <map>
# include <iostream>
# include "utils.hpp"
# include <sys/socket.h>

#define HTTP_STATUS_200 "200 OK"

class Response
{
    private:
        std::string _body;
        std::map<std::string, std::string> _headers;
        std::string _payload;
        std::string _status;
    
    public:
        Response();
        ~Response();

        void setHeader(std::string fieldName, std::string value);
        void setStatus(std::string status);
        void appendToBody(std::string str);
        void send(int fd);

};

#endif /* RESPONSE_HPP */
