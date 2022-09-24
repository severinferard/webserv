# include "request.hpp"

inline std::ostream &operator<<(std::ostream &os, const http_methods_e &method)
{
    switch (method)
    {
    case GET:
        os << "GET";
        break;
    case HEAD:
        os << "HEAD";
        break;
    case POST:
        os << "POST";
        break;
    case DELETE:
        os << "DELETE";
        break;
    case PUT:
        os << "PUT";
        break;
    };
    return os;
}