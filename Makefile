CC = gcc
CFLAGS = -c -g -Wall -Werror -Wno-error=unused-variable -fPIC
SUBSCRIBER = subscriber
SERVER = server
LIST = list
HASHMAP = hashmap
TRIE = trie

all: $(SUBSCRIBER) $(SERVER)

$(LIST).o: $(LIST).c $(LIST).h
	$(CC) $(CFLAGS) $< -o $@

$(HASHMAP).o: $(HASHMAP).c $(HASHMAP).h
	$(CC) $(CFLAGS) $< -o $@

$(TRIE).o: $(TRIE).c $(TRIE).h
	$(CC) $(CFLAGS) $< -o $@

$(SERVER).o: $(SERVER).c $(LIST).h $(HASHMAP).h $(TRIE).h
	$(CC) $(CFLAGS) $< -o $@

$(SUBSCRIBER).o: $(SUBSCRIBER).c
	$(CC) $(CFLAGS) $< -o $@

$(SUBSCRIBER): $(SUBSCRIBER).o
	$(CC) $^ -o $@

$(SERVER): $(LIST).o $(HASHMAP).o $(TRIE).o $(SERVER).o
	$(CC) $^ -o $@

clean:
	rm -rf *.o $(SUBSCRIBER) $(SERVER) $(LIST) $(HASHMAP)