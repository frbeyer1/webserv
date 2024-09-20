
#include "Webserv.hpp"

int main(void)
{
    HttpRequest request;


    char  str[] = "GET /api/users/123 HTTP/1.1\r\nHost: example.com\r\nAccept: application/json\r\nUser-Agent: MyApp/1.0\r\nAuthorization: Bearer your_access_token_here\r\n";


    request.parse(str, strlen(str));
    std::cout << request.getError() << std::endl;





}