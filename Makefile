LIB=-lcci -lpthread

all: server client chat
	echo "Done"

server: server.c
	gcc -o server server.c  $(LIB)

client: client.c
	gcc -o client client.c $(LIB) 

chat: chat.c
	gcc -o chat chat.c $(LIB)

remove:
	rm client server
