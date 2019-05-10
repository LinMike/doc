

CC = g++
INCLUDE = 
LDFLAGS = 

target: ep_server ep_client

ep_server:ep_server.o
	$(CC) $< -o $@ $(LDFLAGS)

ep_client:ep_client.o
	$(CC) $< -o $@ $(LDFLAGS)

%.o: %.cc
	$(CC) $< -c $(INCLUDE)
	
clean:
	rm *.o ep_server ep_client
