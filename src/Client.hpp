#ifndef CLIENT_HPP
#define CLIENT_HPP

# include <string>
# include <algorithm>
# include "Socket.hpp"
# include "Server.hpp"
# include "Request.hpp"
# include "Response.hpp"
// # include "epoll.h"
# include <fcntl.h>
# include "utils.hpp"
# include "Core.hpp"
# include <sstream>
# include <sstream>
# include <sys/stat.h>
# include "Logger.hpp"

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>

#define HTTP_STATUS_SUCCESS 200
#define HTTP_STATUS_NOT_FOUND 404
#define HTTP_STATUS_METHOD_NOT_ALLOWED 405

typedef enum ClientStatus_s {
	STATUS_WAIT_FOR_REQUEST,
	STATUS_WAIT_TO_READ_FILE,
	STATUS_WAIT_TO_READ_DIR,
	STATUS_WAIT_TO_SEND,
}           ClientStatus_t;

#define BUFFER_SIZE 65139

class Socket;
class Request;
class WebservCore;
class Server;

#define EPOLL_OP_NOOP -1
#define EPOLL_OP_ADD 1
#define EPOLL_OP_MODIFY 2
#define EPOLL_OP_REMOVE 3

class HttpError : public std::runtime_error {
	public:
		int status;
		HttpError(int status): std::runtime_error("Http Error"), status(status){};
		~HttpError() throw() {}
};

class Client
{
	private:
		static char		_buffer[BUFFER_SIZE];
		ClientStatus_t	_status;
		Request			_request;
		Response		_response;
		Server			*_server;
		location_t		*_location;
		WebservCore		*_core;

		int				_file_fd;
		void			_onReadToReadRequest();
		void			_onReadToReadFile();
		void			_onReadToSend();
		int				_findIndex(std::string dir, std::vector<std::string> const &candidates);
		void			_handleGet(void);
		void			_handlePost(void);
		void			_handlePut(void);
		void			_handleDelete(void);
		void			_autoIndex(std::string uri, std::string path);
		
		void		Log(LogLevel level, const char* format, ...);
		

	public:
		const std::string    addr;
		const int            port;
		const Socket*        socket;
		const int            connection_fd;

		Client(std::string addr, int port, const Socket *socket, int fd);
		~Client();
		Server *			findServer(void);
		bool				readFileToResponseBody(void);
		Request   			readRequest(void);
		void				resume();
		void				bindCore(WebservCore *core);

};

#endif /* CLIENT_HPP */
