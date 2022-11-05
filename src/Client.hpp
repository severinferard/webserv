#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <algorithm>
#include <sys/wait.h>
#include "Socket.hpp"
#include "Server.hpp"
#include "Request.hpp"
#include "Response.hpp"
// # include "epoll.h"
#include <fcntl.h>
#include "utils.hpp"
#include "Core.hpp"
#include <sstream>
#include <sstream>
#include <sys/stat.h>
#include "Logger.hpp"
#include "HttpError.hpp"

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>

typedef enum ClientStatus_s
{
	STATUS_WAIT_FOR_REQUEST,
	STATUS_PROCESSING,
	STATUS_WAIT_FOR_CONNECTION
} ClientStatus_t;

#define BUFFER_SIZE 65139

class Socket;
class Request;
class WebservCore;
class Server;

#define EPOLL_OP_NOOP -1
#define EPOLL_OP_ADD 1
#define EPOLL_OP_MODIFY 2
#define EPOLL_OP_REMOVE 3

typedef void (Client::*callback_t)(void);

typedef struct uploadedFile_s
{
	std::string filename;
	std::string content;
} uploadedFile_t;

class Client
{
private:
	static char _buffer[BUFFER_SIZE];
	WebservCore *_core;
	Request _request;
	Response _response;
	Server *_server;
	location_t *_location;
	location_t __location;
	DIR *_dp;
	std::vector<struct dirent> _autoindexNodes;
	std::string _cgiPayload;
	std::map<int, callback_t> _callbacks;
	struct timeval _t0;
	FILE *_cgiFileIn;
	FILE *_cgiFileOut;
	int _cgiFdIn;
	int _cgiFdOut;

	int __log_fd;
	int _file_fd;
	bool _timedOut;
	int _cgi_pid;
	int _cgi_stdin_fd;
	bool _isClosed;
	bool _keepAlive;
	std::vector<uploadedFile_t> _uploadedFiles;
	void _onReadyToReadRequest();
	void _onReadyToTest();
	void _onReadyToReadFile();
	void _onReadyToSend();
	void _onReadyToReadDir();
	void _onReadyToReadCgi();
	void _onReadyToWriteToCgi();
	void _onReadyToWriteFile(void);
	void _onReadyToWriteUploadedFile(void);
	void _saveNextFile(void);
	void _onHttpError(const HttpError &e);
	int _findIndex(std::string dir, std::vector<std::string> const &candidates);
	void _handleGet(void);
	void _handleHead(void);
	void _handlePost(void);
	void _handlePut(void);
	void _handleDelete(void);
	void _handleCgi(void);
	void _handleFormUpload(void);
	void _setupAutoIndex(std::string uri, std::string path);
	void _setCallback(int fd, callback_t cb);
	void _setCallback(int fd, callback_t cb, u_int32_t events);
	void _clearCallback(int fd);
	void _setStatus(ClientStatus_t status);

public:
	const std::string addr;
	const int port;
	const Socket *socket;
	const int connection_fd;
	static std::map<int, error_page_t> DEFAULT_ERROR_PAGES;
	time_t statusTimestamp;
	ClientStatus_t status;

	Client(WebservCore *core, std::string addr, int port, const Socket *socket, int fd);
	~Client();
	Server *findServer(void);
	bool readFileToResponseBody(void);
	bool resume(int fd);
	void timeoutIdlingConnection(void);
	void timeoutRequest(void);
	void timeoutGateway(void);
	static std::map<int, error_page_t> initDefaultErrorPages(void);
};

#endif /* CLIENT_HPP */
