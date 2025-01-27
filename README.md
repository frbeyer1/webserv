# Webserv

This is a basic web server built from scratich in C++98 for linux, developed as part of the core curriculum at 42.
It aims to provide a thorough understanding of the HTTP protocol, networking principles, and the CGI (Common Gateway Interface) standard.

More information about the project is in the [subject.pdf](https://github.com/mottjes/webserv/blob/main/subject.pdf)

This was a group project consisting of [@frbeyer](https://github.com/frbeyer1), [@Edy-Nelson](https://github.com/Edy-Nelson) and [@me](https://github.com/mottjes)

### Features

- **HTTP/1.1 Protocol Support:** handles GET, POST and DELETE requests
- **Static Content:** serves static content (like HTML, CSS, ...)
- **Dynamic Content:** serves dynamic generated content with the execution of CGI scripts
- **File Upload:** can recieve and save uploads of multipart/form-data on the server
- **Configuration:** Utilizing an configuration file to specify server settings (more details under Configuration)
- **Network:** handles multiple incoming connections simultaneously, stress tested with [siege](https://github.com/JoeDog/siege)
- **I/O Multiplexing:** uses scalable event interface epoll to ensure the server to be single threaded and non blocking
- **Error Pages:** generates default error pages if non are provided
- **Directory Listening:**  generates an directory listening, if no index file is given and autoindex is enabled

### Installation

1. Clone the repository
```
git clone https://github.com/mottjes/webserv.git
```
2. Build the project:
```
cd webserv
make
```

### Usage

1. To start the server, run:
```
./webserv <config_file>
```
Insert the path to the config file or if no path is specified, it will look for default.conf in the conf directory.

2. Send an HTTP request to the server using your favorite browser. For CGI ensure, that the scripting language is installed.

3. A demo of the main functions can be accsessed via the default.conf and typing following address in your browser:
```
http://localhost:8080
```

### Configuration

The Configuration file is a text file that contains various settings named directives that dictate how the web server should operate. If any directive is not set, it will take the default settings (defined in Webserv.hpp). You can setup multiple servers in one configuration file. For that you can specify multiple server blocks with different settings.

#### Example:

```
server {
    server_name             example.com www.example.com;    # sets names of the virtual server
    listen                  127.0.0.1:8080;                 # binds the given address to the port. if no address is given binds 0.0.0.0.
    root                    docs/;                          # sets the root directory for the server
    client_max_body_size    10000;                          # limits the allowed client body size in
    error_page              404 error_pages/404.html;       # defines the URI that will be shown for the specifc error

    location / {                                            # sets configuration depending on the given uri
        allowed_methods     GET;                            # defines allowed methods on that location
        index               index.html;                     # defines file that will be used as an index for that location
    }
    location /google {
        allowed_methods     GET;
        return              https://www.google.com/;        # sets an redirection
    }
    location /images/ {
        allowed_methods     GET;
        alias               assets/images/;                 # sets an alias for the URI
    }
    location /uploads {
        allowed_methods     GET POST DELETE;
        autoindex           on;                             # enables the directory listing
        upload              uploads/;                       # defines a directory where files get uploaded
    }
    location /cgi-bin/ {
        allowed_methods     GET POST;
        cgi                 .py /bin/python3;               # defines a CGI binary that will be executed for the given extension
    }
}
```
### Architecture

![architectural_overview](https://github.com/user-attachments/assets/5de4ed7b-48da-4a1d-9919-c967193fbc3e)

