
#include "../inc/Server.hpp"
#include "../inc/Logger.hpp"
#include "../inc/Response.hpp"
#include "../inc/Client.hpp"
#include "../inc/ConfigParser.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string>
#include "../inc/Logger.hpp"
#include "../inc/Webserv.hpp"


// Info fuer Freddy
// FREE THE newenv after using it for your nefarious purposes
// close fdread when you are done with it
// check if fdread is -1, if so, system function has failed, Error Code 500


// g++ cgi.cpp ./webserv_git/src/Client.cpp ./webserv_git/src/ConfigParser.cpp ./webserv_git/src/HttpRequest.cpp ./webserv_git/src/Logger.cpp ./webserv_git/src/Response.cpp ./webserv_git/src/Server.cpp ./webserv_git/src/ServerManager.cpp 

struct cgireturn {
    int fdread;
    char **newenv;
};

// cgifile muss so aufgebaut werden:
// <PATH>/python3
// <CGIFILE>
// <ARGUMENT 1 etc.>

// Takes two lines and allocates them into one, skips line 2 if its \0
char *newlinecombine(const char *line1, const char *line2) {
    char *newline = (char*)malloc(256 * sizeof(char));
    int i = 0; 
    int j = 0;
    while(line1[i]) {
        newline[i] = line1[i];
        i++;
    }
    while(line2[j]) {
        newline[i] = line2[j];
        i++;
        j++;
    }
    newline[i] = '\0';
    return newline;
}

char* itoa(int num) {
    char *str;
    int i = 10;
    int minus = 0;
    str = (char*)malloc(16 * sizeof(char));

    if (num == 0) {
        str[0] = '0';
        str[1] = '\0';
        return str;
    }
    if (num < 0) {
        minus = 1;
        num = -num;
    }
    str[11] = '\0';
    while (num != 0) {
        str[i] = (num % 10) + '0';
        num = num / 10;
        i--;
    }
    if (minus) {
        str[i] = '-';
        return &str[i];
    }
    return &str[i+1];
}

// /HttpRequest &ref1, Server &ref2, Response &ref3
char **buildenv(char *cgifile, char **env, Client &ref1) { 
    char **newenv = (char **)malloc(400 * sizeof(char *));;
    char *line;

    int j = 0;
    for (int i = 0; env[i] ;i++) {
        newenv[i] =  newlinecombine(env[i], "\0");
        j++;
    }

    newenv[j++] = newlinecombine("REDIRECT_STATUS=\0", itoa(ref1.response.getError()));
    //newenv[j++] = newlinecombine("CONTENT_TYPE=\0", ref1.response.getContent_type()); //getContent_type existiert nicht
    //newenv[j++] = newlinecombine("CONTENT_LENGTH=\0", ref1.response.getContent_length()); // content length not defined in Response

    newenv[j++] = newlinecombine("GATEWAY_INTERFACE=\0", "CGI/1.1\0");
    newenv[j++] = newlinecombine("PATH_INFO=\0", ref1.request.getPath().c_str()); 
    newenv[j++] = newlinecombine("PATH_TRANSLATED=\0", (ref1.server->getRoot() + ref1.request.getPath()).c_str()); // path to CGI script but out of root
    newenv[j++] = newlinecombine("QUERY_STRING=\0", ref1.request.getQuery().c_str());
    newenv[j++] = newlinecombine("REMOTE_ADDR=\0", sockaddrToIpString(ref1.getClientAddress()).c_str());
    newenv[j++] = newlinecombine("REMOTE_HOST=\0", ""); // not defined in Client, empty because unlikely
    newenv[j++] = newlinecombine("REMOTE_USER=\0", itoa(ref1.getClientFd()));
    newenv[j++] = newlinecombine("REQUEST_METHOD=\0", itoa(ref1.request.getMethod()));
    newenv[j++] = newlinecombine("SCRIPT_NAME=\0", cgifile);
    newenv[j++] = newlinecombine("SERVER_NAME=\0", ref1.server->getServerName().c_str());
    newenv[j++] = newlinecombine("SERVER_PORT=\0", itoa(ref1.server->getPort()));
    newenv[j++] = newlinecombine("SERVER_PROTOCOL=\0", "HTTP/1.1\0");
    newenv[j++] = newlinecombine("SERVER_SOFTWARE=\0", "Webserv/1.0\0");
    newenv[j] = 0;
    return newenv;
}


//we return an int, which is a file descriptor where everything has been dumped into.
cgireturn *process_cgi(char **cgifile, char **env, Client &ref1) { //, Client &ref1
    cgireturn *data;
    data = (cgireturn*)malloc(2 * sizeof(cgireturn));
    buildenv(cgifile[1], env, ref1);
    int fd[2], pid, status;
    if (pipe(fd) == -1) {
        Logger::log(RED, ERROR, "Creating pipe has failed, aborting CGI init process.");
        data->fdread = -1;
        return data;
    }
    if ((pid = fork()) == -1) {
        Logger::log(RED, ERROR, "Creating fork has failed, aborting CGI init process.");
        data->fdread = -1;
        return data;
    }
    if (!pid) {
        if (dup2(fd[1], 1) == -1) {
            Logger::log(RED, ERROR, "Child Process ID: %i: dup2 has failed (IN)", pid);
            abort();
        }
        if (dup2(fd[0], 0) == -1) {
            Logger::log(RED, ERROR, "Child Process ID: %i: dup2 has failed (OUT)", pid);
            abort();
        }
        execve(*cgifile, cgifile, env);
        abort();
        Logger::log(RED, ERROR, "Child Process ID: %i: Execve has failed. Program name: %s", pid, *cgifile);
    }
    waitpid(pid, &status, 0);
    if (!WIFEXITED(status)) {
       data->fdread = -1;
       return data;
    }
    close(fd[1]); //WRITE END
    //close(fd[0]); //READ END
    data->fdread = fd[0];
    return data;
}

//test for buildenv
// int main(int argc, char **argv, char **env) {
//     (void)argc;
//     (void)argv;
//     char troll[25] = "TROLOLOL\0";
//     for (int i = 0; env[i]; i++) {
//         printf("ORG:%i: %s\n", i, env[i]);
//     }
//     env = buildenv(troll, env);
//     for (int i = 0; env[i]; i++) {
//         printf("%i: %s\n", i, env[i]);
//     }
// }

//main to test pipe with
void print_fd_contents(int fd) {
    char buffer[4096];
    size_t bytes_read;

    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        write(STDOUT_FILENO, buffer, bytes_read);
    }
}

int main(int argc, char **argv, char **env) {
    cgireturn *testdata;
    char *cargv[] = {
        "/bin/python3", //bin/python3
        "cookie.py",
        NULL
    };
    testdata = process_cgi(cargv, env);
    printf("test:%i\n", testdata->fdread);
    if (testdata->fdread != -1)
        print_fd_contents(testdata->fdread);
    close(testdata->fdread);
}