#pragma once

#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <netinet/in.h>

#include <arpa/inet.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <ctime>

#include <map>
#include <vector>

#include "HttpRequest.hpp"
#include "ServerManager.hpp"
#include "Server.hpp"
#include "ConfigParser.hpp"
#include "Client.hpp"
#include "Logger.hpp"

/*   Logger Settings   */
#define LOGGER_STATE                                ON
#define LOGGER_OUTPUT_MODE                          STDOUT
#define LOGGER_LOG_LVL                              ERROR

/*   HTTP Error Codes   */
#define BAD_REQUEST                                 400
#define FORBIDDEN                                   403
#define NOT_FOUND                                   404
#define PAYLOAD_TOO_LARGE                           413
#define URI_TOO_LONG                                414
#define REQUEST_HEADER_FIELDS_TOO_LARGE             431
#define NOT_IMPLEMENTED                             501

/*   Default Config Settings   */
#define DEFAULT_CONFIG                              "conf/default.conf"
#define DEFAULT_PORT                                80
#define DEFAULT_HOST                                "127.0.0.1"
#define DEFAULT_NAME                                "default"
#define DEFAULT_ROOT                                "docs/"
#define DEFAULT_CLIENT_MAX_BODY_SIZE                10240

/*   Technical Settings   */
#define MAX_EPOLL_EVENTS                            10
#define MAX_CONNECTIONS                             10
#define BACKLOG                                     128
#define MAX_URI_LENGHT                              4096
#define MAX_HEADER_LENGHT                           8192
#define CLIENT_CONNECTION_TIMEOUT                   60
#define LOGFILE_NAME                                "webserv.log"

/*   ANSI escape codes for colors   */
#define RESET                                       "\033[0m"
#define RED                                         "\033[31m"
#define WHITE                                       "\033[37m"
#define CYAN                                        "\033[36m"
#define GREEN                                       "\033[32m"
#define BLUE                                        "\033[34m"
#define YELLOW                                      "\033[33m"
#define MAGENTA                                     "\033[35m"
