#include "../inc/Client.hpp"

// =============   Constructor   ============= //
Client::Client(Server &s) :server(s)
{
    request.setClientMaxBodySize(server.getClientMaxBodySize());
    _last_msg_time = time(NULL);
}

// ============   Deconstructor   ============ //
Client::~Client()
{
}

// ================   Getter   =============== //
int     Client::getClientFd() const
{
    return (_client_fd);
}

time_t  Client::getLastMsgTime() const
{
    return (_last_msg_time);
}

struct sockaddr_in  Client::getClientAddress() const
{
    return (_client_address);
}

// ================   Setter   =============== //
void    Client::setClientFd(int fd)
{
    _client_fd = fd;
}

void    Client::setLastMsgTime(time_t time)
{
    _last_msg_time = time;
}
void    Client::setClientAddress(struct sockaddr_in client_address)
{
    _client_address = client_address;
}
