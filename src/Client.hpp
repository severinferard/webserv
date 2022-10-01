#ifndef CLIENT_HPP
#define CLIENT_HPP

# include <string>
# include <algorithm>
# include "Socket.hpp"
# include "Request.hpp"
# include "Response.hpp"
# include "epoll.h"
# include <fcntl.h>
# include "utils.hpp"


typedef enum ClientStatus_s {
	STATUS_WAIT_FOR_REQUEST,
	STATUS_WAIT_TO_READ_FILE,
	STATUS_WAIT_TO_SEND,
}           ClientStatus_t;

#define BUFFER_SIZE 65139

class Socket;
class Request;
class WebservCore;

#define EPOLL_OP_NOOP -1
#define EPOLL_OP_ADD 1
#define EPOLL_OP_MODIFY 2
#define EPOLL_OP_REMOVE 3

typedef struct epoll_operation_s{
	int      	fd;
	uint32_t    events;
	int         type;
}       epoll_operation_t;

class Client
{
	private:
		static char		_buffer[BUFFER_SIZE];
		ClientStatus_t	_status;
		Request			_request;
		Response		_response;

		int				_file_fd;
		

	public:
		const std::string    addr;
		const int            port;
		const Socket*        socket;
		const int            connection_fd;

		Client(std::string addr, int port, const Socket *socket, int fd);
		~Client();
		Server *			findServer(void);
		bool				readFileToResponseBody(void);
		void   				readRequest(void);
		void				resume(int epoll_fd, std::map<int, Client *> *clients);

};

#endif /* CLIENT_HPP */
