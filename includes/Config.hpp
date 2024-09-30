#pragma once

#include "Webserv.hpp"

class Config
{
private:
    std::string                 _host;
    uint16_t                    _port;
    std::string                 _server_name;

    
    // std::map<int, std::string>  _error_pages;
    // size_t                      _max_client_body_size;
    // bool                        _auto_index;
                                    // _routes;

public:
// Constructor
    Config();

// Deconstructor
    ~Config();

};

// Server {
//     listen 0.0.0.0:8080;                                 # optional IP, default is 0.0.0.0:8080
//     server_name _;                                       # optional
//     error_page 404 /relative/path/to/error/page.html;    # optional
//     client_max_body_size 1M;                             # optional, default is 1M

//     location /delete/ {
//         limit_except GET POST DELETE;                    # optional
//         return URL;                                      # optional
//         alias path/rooted/to;                            # optional
//         autoindex on/off;                                # optional, default is off
//         index index_file.html;                           # optional
//         
//        # root /path/to/root;
//         
//     }

//     root /absolute/path;
//     index index.html; # optional
//     autoindex true; # optional, default is false


//     upload_store /path/to/uploaded/files; # optional, default is /uploads


//     location .py {
//         # path to executable
//         cgi_pass /absolute/path/to/executable;
//     }
// }
