#include "../inc/Client.hpp"

// =============   Constructor   ============= //
Client::Client()
{
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

// ================   Setter   =============== //
void    Client::setClientFd(int fd)
{

}

void    Client::setLastMsgTime(time_t time)
{

}