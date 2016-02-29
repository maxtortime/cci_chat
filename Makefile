LIB=-lcci -lpthread

all: chat
	echo "Done"

chat: chat.c
	gcc -o chat -g chat.c $(LIB)

remove:
	rm client server
