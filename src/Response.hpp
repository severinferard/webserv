#ifndef RESPONSE_HPP
#define RESPONSE_HPP

# include <string>
# include <map>

class Response
{
    private:
        std::string _body;
        std::map<std::string, std::string> _headers;
    
    public:
        Response();
        ~Response();

        void setHeader(std::string fieldName, std::string value);
        void appendToBody(std::string str);

};

#endif /* RESPONSE_HPP */
