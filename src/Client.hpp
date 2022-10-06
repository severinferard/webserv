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
# include "HttpError.hpp"

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>

#define DEFAULT_ERROR_PAGE_400 "./src/www/errors/400.html"
#define DEFAULT_ERROR_PAGE_404 "./src/www/errors/404.html"
#define DEFAULT_ERROR_PAGE_405 "./src/www/errors/405.html"
#define DEFAULT_ERROR_PAGE_411 "./src/www/errors/411.html"
#define DEFAULT_ERROR_PAGE_415 "./src/www/errors/415.html"
#define DEFAULT_ERROR_PAGE_500 "./src/www/errors/500.html"
#define DEFAULT_ERROR_PAGE_501 "./src/www/errors/501.html"
#define DEFAULT_ERROR_PAGE_505 "./src/www/errors/505.html"

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

		int				__log_fd;
		int				_file_fd;
		void			_onReadToReadRequest();
		void			_onReadToReadFile();
		void			_onReadToSend();
		int				_findIndex(std::string dir, std::vector<std::string> const &candidates);
		void			_handleGet(void);
		void			_handleHead(void);
		void			_handlePost(void);
		void			_handlePut(void);
		void			_handleDelete(void);
		void			_autoIndex(std::string uri, std::string path);
		static void		_initDefaultErrorPages(void);
		
	public:
		const std::string    addr;
		const int            port;
		const Socket*        socket;
		const int            connection_fd;
		static std::map<int, error_page_t>  DEFAULT_ERROR_PAGES;

		Client(std::string addr, int port, const Socket *socket, int fd);
		~Client();
		Server *			findServer(void);
		bool				readFileToResponseBody(void);
		// Request   			readRequest(void);
		void				resume();
		void				bindCore(WebservCore *core);

};

#endif /* CLIENT_HPP */
