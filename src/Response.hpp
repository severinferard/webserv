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
        int _status;
        bool                                _ignoreBody;

        static void _initHttpStatus(void);
    
    public:
        Response();
        ~Response();

        static std::map<int, std::string> HTTP_STATUS;

        void setHeader(std::string fieldName, std::string value);
        void setStatus(int status);
        int getStatus(void) const;
        void appendToBody(std::string str);
        void send(int fd);
        void setIgnoreBody(bool b);

};

#endif /* RESPONSE_HPP */
