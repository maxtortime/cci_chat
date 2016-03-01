LIB=-lcci -lpthread

all: chat
	echo "Done"

chat: chat.o utils.o
	gcc chat.o utils.o $(LIB) -o chat

chat.o: chat.c
	gcc -c chat.c

utils.o: utils.c
	gcc -c utils.c

remove:
	rm client server
