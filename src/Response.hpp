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
    
    public:
        Response();
        ~Response();

        static std::map<int, std::string> HTTP_STATUS;
        static std::map<std::string, std::string> MIME_TYPES;
        static std::map<int, std::string> initHttpStatus(void);
        static std::map<std::string, std::string> initMimeTypes(void);
        static std::string     getContentType(std::string route);

        void setHeader(std::string fieldName, std::string value);
        void setStatus(int status);
        int getStatus(void) const;
        void appendToBody(std::string str, size_t size);
        void appendToBody(std::string str);
        void appendToRawPayload(std::string str);
        void send(int fd);
        void sendRaw(int fd);
        void setIgnoreBody(bool b);
        void clearBody(void);
        bool keepAlive(void);

};

#endif /* RESPONSE_HPP */
