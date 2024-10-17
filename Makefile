NAME		= webserv

SRC			=	src/main.cpp			\
				src/Server.cpp			\
				src/ServerManager.cpp	\
				src/Logger.cpp			\
				src/HttpRequest.cpp		\
				src/ConfigParser.cpp	\
				src/Client.cpp			\

OBJ			= $(SRC:.cpp=.o)

CC      	= c++

FLAGS	= -Wall -Werror -Wextra #-std=c++98

all: $(NAME)

$(NAME): $(OBJ)
	@$(CC) $(OBJ) $(FLAGS) -o $@

%.o: %.cpp
	$(CC) $(FLAGS) -o $@ -c $<

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)


re:    	fclean all

.PHONY: all clean fclean format re