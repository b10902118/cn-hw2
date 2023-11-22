.PHONY: all clean

CXX := g++
CXXFLAGS := -ggdb -Wall -Wno-unused-variable -Wno-sign-compare -Wno-char-subscripts 
#-fsanitize=address 

SERVER_SRCS := server.cc request.cc utils.cc utils/base64.c backend.cc html.cc fs.cc response.cc auth.cc
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

