#pragma once

#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <dirent.h>

#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <netinet/in.h>

#include <arpa/inet.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <ctime>
#include <cstring>
#include <cstdarg>

#include <map>
#include <vector>

#include "ConfigParser.hpp"
#include "Server.hpp"
#include "HttpRequest.hpp"
#include "Logger.hpp"

/*   Logger Settings   */
#define DEFAULT_LOGGER_STATE                        ON              // ON or OFF
#define DEFAULT_LOGGER_OUTPUT                       STDOUT          // STDOUT or OUT_FILE  
#define DEFAULT_LOGGER_LEVEL                        INFO            // ERROR, INFO or DEBUG
#define LOGFILE_NAME                                "webserv.log"

/*   Default Config Settings   */
#define DEFAULT_CONFIG                              "conf/default.conf"
#define DEFAULT_INDEX                               "index.html"
#define DEFAULT_PORT                                80
#define DEFAULT_HOST                                "127.0.0.1"
#define DEFAULT_NAME                                "default"
#define DEFAULT_ROOT                                "docs/"
#define DEFAULT_CLIENT_MAX_BODY_SIZE                10240

/*   Technical Settings   */
#define MAX_EPOLL_EVENTS                            10
#define MAX_CONNECTIONS                             10
#define BACKLOG                                     128
#define MAX_URI_LENGTH                              4096
#define MAX_HEADER_LENGTH                           8192
#define CLIENT_CONNECTION_TIMEOUT                   10
#define REQUEST_READ_SIZE                           4096
#define RESPONSE_WRITE_SIZE                         4096

/*   HTTP Error Codes   */
#define OK                                          200
#define CREATED                                     201
#define ACCEPTED                                    202
#define NO_CONTENT                                  204
#define MOVED_PERMANENTLY                           301
#define BAD_REQUEST                                 400
#define FORBIDDEN                                   403
#define NOT_FOUND                                   404
#define NOT_ALLOWED                                 405
#define PAYLOAD_TOO_LARGE                           413
#define URI_TOO_LONG                                414
#define UNSUPPORTED_MEDIA_TYPE                      415
#define REQUEST_HEADER_FIELDS_TOO_LARGE             431
#define INTERNAL_SERVER_ERROR                       500
#define NOT_IMPLEMENTED                             501

/*   ANSI escape codes for colors   */
#define RESET                                       "\033[0m"
#define RED                                         "\033[31m"
#define YELLOW                                      "\033[33m"
#define WHITE                                       "\033[37m"
#define CYAN                                        "\033[36m"
#define GREY                                        "\033[90m"
