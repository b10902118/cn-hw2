.PHONY: all clean

all: server client
server: server.cc request.h
	g++ -Wno-unused-variable -o server server.cc
client: client.cc
	g++ -Wno-unused-variable -o client client.cc
clean:
	@rm -rf server client 
