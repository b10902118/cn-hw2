.PHONY: all clean

CXX := g++
CXXFLAGS := -Wall -Wno-unused-variable

SERVER_SRCS := server.cc request.cc utils.cc utils/base64.c
CLIENT_SRCS := client.cc request.cc utils.cc utils/base64.c

all: server client

# $@: target
# $^ prerequisites
server: $(SERVER_SRCS:.cc=.o)
	$(CXX) $(CXXFLAGS) -o $@ $^

client: $(CLIENT_SRCS:.cc=.o)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f server client *.o

