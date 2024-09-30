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

#include "HttpRequest.hpp"
#include "Server.hpp"
#include "Config.hpp"