# Webserv
work in progress ...



### Description

This goal of this project was to write our own HTTP server in C++98 from scratch.
The project emphasizes a deep understanding of HTTP, networking and CGI (Common Gateway Interface) standard.

More information about the project is in the subject.pdf

This was a group project consisting of [@frbeyer](https://github.com/frbeyer1), [@Edy-Nelson](https://github.com/Edy-Nelson) and [@me](https://github.com/mottjes)

### Features

- **HTTP/1.1:** handles GET, POST and DELETE requests
- **Content:** serves static content(HTML, css , etc.) and dynamic content with executing CGI scripts
- **Configuration:** Utilizing an configuration file to specify server settings(more details under Usage)
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

The configuration file is

