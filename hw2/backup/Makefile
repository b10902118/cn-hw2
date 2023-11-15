.PHONY: all clean

#CXX := g++
CXXFLAGS := -Wall -Wno-unused-variable

all: server client
server: server.cc request.h request.cc utils.h utils.cc utils/base64.c 
	g++ $(CXXFLAGS) -o server server.cc request.cc utils.cc utils/base64.c
client: client.cc utils/base64.c request.cc
	g++ -Wno-unused-variable -o client client.cc request.cc utils.cc utils/base64.c
clean:
	@rm -rf server client 
