# Webserv

Work in progress ...

### Description

This goal of this project was to write our own HTTP server in C++98 from scratch.
The project emphasizes a deep understanding of HTTP, networking and CGI (Common Gateway Interface) standard.

More information about the project is in the subject.pdf

This was a group project consisting of [@frbeyer](https://github.com/frbeyer1), [@Edy-Nelson](https://github.com/Edy-Nelson) and [@me](https://github.com/mottjes)

### Features

- **HTTP/1.1:** handles GET, POST and DELETE requests
- **Content:** serves static content(HTML, css , etc.) and dynamic content with executing CGI scripts
- **Configuration:** Utilizing an configuration file to specify server settings (more details under Usage)
- **Network:** handles multiple incoming connections, stress tested with [siege](https://github.com/JoeDog/siege)
- **I/O Multiplexing:** uses epoll() to ensure the server to be single threaded and non blocking

### Insallation

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

3. A demo of the main functions can be accsess via the default.conf and typing following address in your browser:
```
http://localhost:8080
```

### Configuration

The Configuration file is a text file that contains various settings named directives that dictate how the web server should operate. If any directive is not set, it will take the default settings defined in Webserv.hpp.
You can put multiple server blocks inside one config with different settings.

#### Example:

```
server {

    server_name                     example.com www.example.com;                # sets names of the virtual server
    listen                          127.0.0.1:8080;                             # binds the given address to the port. if no address is given, binds 0.0.0.0.
    root                            docs/;                                      # sets the root directory for the server
    client_max_body_size            100000;                                     # limits the allowed client body size in
    error_page                      404 error_pages/404.html;                   # defines the URI that will be shown for the specified errors.

    location / {                                                                # sets configuration depending on the given uri
        allowed_methods     GET;                                                # defines allowed methods on that location
        index               index.html;                                         # defines file that will be used as an index for that location
    }
}
```

There are a couple more directives you can use inside an location:

```
autoindex           on;                                                 # enables the directory listing
alias               assets/images/;                                     # sets an alias
return              https://www.google.com/;                            # sets an redirection
upload              user/uploads/;                                      # defines a directory where files get uploaded
cgi                 .py /bin/python3;                                   # defines a CGI binary that will be executed for the given extension
```

### Deep dive

Work in progres ...

### Resources



