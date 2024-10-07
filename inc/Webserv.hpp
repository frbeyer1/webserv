#pragma once

#include <unistd.h>
#include <string.h>
#include <fcntl.h>

/*   Networking   */
#include <sys/socket.h>
#include <sys/epoll.h> 
#include <netinet/in.h>

#include <iostream>
#include <cstdint>
#include <sstream>
#include <fstream>

/*   STL Container   */
#include <map>
#include <vector>

#include "ServerManager.hpp"
#include "HttpRequest.hpp"
#include "Server.hpp"
#include "ConfigParser.hpp"


/*   Error Codes   */
#define BAD_REQUEST                                 400
#define FORBIDDEN                                   403
#define NOT_FOUND                                   404
#define PAYLOAD_TOO_LARGE                           413
#define URI_TOO_LONG                                414
#define REQUEST_HEADER_FIELDS_TOO_LARGE             431
#define NOT_IMPLEMENTED                             501

/*   Default Settings   */
#define DEFAULT_PORT                                80
#define DEFAULT_NAME                                "default"
#define DEFAULT_ROOT                                "docs/"
#define DEFAULT_CLIENT_MAX_BODY_SIZE                1048576

#define MAX_EPOLL_EVENTS                            10
#define BACKLOG                                     128

#define MAX_URI_LENGHT                              4096
#define MAX_HEADER_LENGHT                           8192

#define MAX_BODY_SIZE                               1048576




