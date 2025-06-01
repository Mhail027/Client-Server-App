// Necula Mihail 323CAa 2024 - 2025
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include "utils.h"
#include "server.h"

void bind_sock(const int sock_fd, const int port) {
	struct sockaddr_in addr;
	int ret;

	/* Complete the info about socket. */
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;

	/* Bind the socket. */
	ret = bind(sock_fd, (const struct sockaddr *) &addr, sizeof(addr));
	DIE(ret == -1, "bind() failed\n");
}

void init_udp_socket(srv_t *const srv) {
	/* Create the socket. */
	srv->udp_sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	DIE(srv->udp_sock_fd == -1, "socket() failed\n");    

	/* Bind the socket. */
	bind_sock(srv->udp_sock_fd, srv->port);
}

void init_tcp_socket(srv_t *const srv) {
	int ret, flag;

	/* Create the socket. */
	srv->tcp_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(srv->tcp_sock_fd == -1, "socket() failed\n");    

	/* Deactivate Nagle's algorithm. */
	flag = 1;
	ret = setsockopt(srv->tcp_sock_fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int));
	DIE(ret == -1, "setsockopt() failed\n");

	/* Bind the socket. */
	bind_sock(srv->tcp_sock_fd, srv->port);

	/* Put the socket to listen. */
	ret = listen(srv->tcp_sock_fd, 100);
	DIE(ret == -1, "listen() failed\n");
}

void init_poll_fds(srv_t *const srv) {
	srv->pfds = (struct pollfd *) malloc(256 * sizeof(struct pollfd));
	DIE(!srv->pfds, "malloc() failed\n");
	srv->max_nfds = 256;

	/* We are interested in stdin. */
	srv->pfds[0].fd = STDIN_FILENO;
	srv->pfds[0].events = POLLIN;
	srv->pfds[0].revents = 0;

	/* We are interested in udp socket. */
	srv->pfds[1].fd = srv->udp_sock_fd;
	srv->pfds[1].events = POLLIN;
	srv->pfds[1].revents = 0;

	/* We are interested in tcp socket. */
	srv->pfds[2].fd = srv->tcp_sock_fd;
	srv->pfds[2].events = POLLIN;
	srv->pfds[2].revents = 0;

	/* We care just of 3 files. */
	srv->nfds = 3;
}

void free_sub(void *data) {
	/* Make the cast. */
	sub_t *sub = (sub_t *)data;

	/* Close the socket if it is open. */
	if (sub->sock_fd != -1) {
		DIE(close(sub->sock_fd) == -1, "close() failed\n");
	}

	/* Free the memory */
	trie_free(&sub->subscriptions);
	free(sub);
}

void init_subs(srv_t *const srv) {
	srv->subs_by_id = ht_create(2, &hash_string, &compare_strings,
								&free_simple_pair);
	srv->subs_by_fd = ht_create(2, &hash_uint, &compare_uints,
								&free_simple_pair);
	srv->subs = ll_create(sizeof(sub_t), &free_sub);
}



void srv_closes_sub_connection(sub_t *const sub) {
	srv_msg_t msg;
	int ret;

	/* Inform the subscriber that will close the connection if we
	 * have one with him. */
	if (sub->sock_fd != -1) {
		/* Create the message. */
		memset(&msg, 0, sizeof(msg));
		msg.type = CLOSE_SRV_MSG;

		/* Send the message. */
		ret = send(sub->sock_fd, (char *) &msg, sizeof(msg), 0);
		DIE(ret == -1, "send() failed\n");

		/* Close the connection. */
		DIE(close(sub->sock_fd) == -1, "close() failed\n");
		sub->sock_fd = -1;
	}
}

void stop_program(srv_t *const srv) {
	/* Tell to all connections that will stop the server. */
	ll_node_t *curr_node = srv->subs->head;
	while (curr_node) {
		srv_closes_sub_connection((sub_t*)curr_node->data);
		curr_node = curr_node->next;
	}

	/* Free all allocated memory. */
	ht_free(&srv->subs_by_id);
	ht_free(&srv->subs_by_fd);
	ll_free(&srv->subs);
	free(srv->pfds);

	DIE(close(srv->udp_sock_fd) == -1, "close() failed\n");
	DIE(close(srv->tcp_sock_fd) == -1, "close() failed\n");

	/* Shut down */
	exit(0);
}

void execute_exit_cmd(srv_t *const srv, char *const line) {
	/* Does the command is valid? */
	if (strtok(NULL, " \n")) {
		printf("Invalid arguments for command.\n");
		return;
	}
	free(line);
	stop_program(srv);
}

void handle_stdin_msg(srv_t *const srv) {
	char *line, *command;
	size_t line_len;
	int ret;

    /* Get the line. */
    line = NULL;
    line_len = 0;
    ret = getline(&line, &line_len, stdin);
    DIE(ret == -1, "getline() failed\n");

    /* Execute the command. */
    command = strtok(line, " \n");
    if (command == NULL) {
        printf("Invalid command.\n");
    } else if (!strcmp(command, EXIT_CMD)) {
		execute_exit_cmd(srv, line);
    } else {
        printf("Invalid command.\n");
    }

	free(line);
}

void add_pfd(srv_t *const srv, const int fd) {
	/* We have enough space? If not, double the array. */
	if (srv->nfds == srv->max_nfds) {
		srv->max_nfds *= 2;
		srv->pfds = (struct pollfd *) realloc(srv->pfds, srv->max_nfds * sizeof(struct pollfd));
		DIE(!srv->pfds, "realloc() failed\n");
	}
	
	/* Add the new pfd. */
	srv->pfds[srv->nfds].fd = fd;
	srv->pfds[srv->nfds].events = POLLIN;
	srv->pfds[srv->nfds].revents = 0;
	srv->nfds++;
}

bool is_subscribed(trie_node_t *topics, char *topic) {
	u_int idx, pos;

	/* Do we reached the topic's end? */
	if (topic[0] == '\0') {
		return topics->end_of_word;
	}

	/* Go over the '/' character */
	if (topic[0] == '/') {
		topic++;
		
		idx = get_index_from_alphabet(TOPIC_ALPHABET, '/');
		if (topics->children[idx]) {
			topics = topics->children[idx];
		} else {
			return false;
		}
	}

	/* + wildcard */
	idx = get_index_from_alphabet(TOPIC_ALPHABET, '+');
	if (topics->children[idx]) {
		/* Go until find the next '/' or the end of string. */
		pos = 0;
		while (topic[pos] != '\0' && topic[pos] != '/') {
			pos++;
		}

		if (is_subscribed(topics->children[idx], topic + pos)) {
			return true;
		}
	}

	/* * wildcard */
	idx = get_index_from_alphabet(TOPIC_ALPHABET, '*');
	if (topics->children[idx]) {
		pos = 0;
		while (topic[pos] != '\0') {
			/* Go over the '/' char.*/
			if (topic[pos] == '/') {
				pos++;
			}

			/* Go until find the next '/' or the end of string. */
			while (topic[pos] != '\0' && topic[pos] != '/') {
				pos++;
			}

			if (is_subscribed(topics->children[idx], topic + pos)) {
				return true;
			}
		}
	}

	/* No wildcard */
	while (topic[0] != '\0' && topic[0] != '/') {
		idx = get_index_from_alphabet(TOPIC_ALPHABET, topic[0]);
		if (!topics->children[idx]) {
			return false;
		}

		topic++;
		topics = topics->children[idx];
	}
	return is_subscribed(topics, topic);
}

void handle_udp_msg(srv_t *const srv) {
	struct sockaddr_in udp_provider_addr;
	socklen_t addr_len;
	int recv_bytes, send_bytes;
	udp_msg_t udp_msg;
	srv_msg_t srv_msg;
	ll_node_t *curr_node;
	sub_t *sub;
	trie_node_t *topics;

	/* Receive the message from an udp client. */
	addr_len = sizeof(udp_provider_addr);
	recv_bytes = recvfrom(srv->udp_sock_fd, &udp_msg, sizeof(udp_msg), 0,
						  (struct sockaddr *) &udp_provider_addr, &addr_len);
	DIE(recv_bytes == -1, "recv() failed\n");

	/* Create the message which will be send to subscribers.*/
	memset(&srv_msg, 0, sizeof(srv_msg));
	srv_msg.provider_ip = ntohl(udp_provider_addr.sin_addr.s_addr);
	srv_msg.provider_port = ntohs(udp_provider_addr.sin_port);
	memcpy(&srv_msg.topic, &udp_msg.topic, MAX_TOPIC_SIZE);
	memcpy(&srv_msg.type, &udp_msg.type, sizeof(uint8_t));
	memcpy(&srv_msg.data, &udp_msg.data, recv_bytes - MAX_TOPIC_SIZE -
		   sizeof(uint8_t));

	/* Send the message to subscribers. */
	curr_node = srv->subs->head;
	while (curr_node) {
		sub = (sub_t *) curr_node->data;
		topics = sub->subscriptions->root;

		if (sub->sock_fd != -1 && is_subscribed(topics, srv_msg.topic)) {
			send_bytes = send(sub->sock_fd, (const void *) &srv_msg,
							  sizeof(srv_msg), 0);
			DIE(send_bytes == -1, "send() failed\n");
		}

		curr_node = curr_node->next;
	}
}

void add_new_sub(srv_t *const srv, sub_t *const new_sub, 
				 const struct sockaddr_in *const addr) {
	sub_t *sub_ptr;
	/* Monitor the fd of the connection. */
	add_pfd(srv, new_sub->sock_fd);
	
	/* Create space to save his pleasures. */
	new_sub->subscriptions = trie_create(1, strlen(TOPIC_ALPHABET),
							TOPIC_ALPHABET, &free);

	/* Update the info about subscribers. */
	ll_add_nth_node(srv->subs, 0, new_sub);

	/* Both hashmaps and the list will share the same zone of memory for
	 * subscriber's info. */
	sub_ptr = (sub_t *) srv->subs->head->data;
	ht_put(srv->subs_by_fd, &sub_ptr->sock_fd, sizeof(sub_ptr->sock_fd),
			&sub_ptr, sizeof(sub_ptr));
	ht_put(srv->subs_by_id, &sub_ptr->id, strlen(sub_ptr->id) + 1,
			&sub_ptr, sizeof(sub_ptr));

	/* Print message with current state at stdout.*/
	printf("New client %s connected from %s:%i.\n", new_sub->id,
		inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
}

void revive_sub_connection(srv_t *srv, sub_t *sub, u_int sock_fd,
						   struct sockaddr_in * addr) {
	/* Update fd and monitor the new fd of the connection. */
	sub->sock_fd = sock_fd;
	add_pfd(srv, sub->sock_fd);
	ht_put(srv->subs_by_fd, &sub->sock_fd,
		sizeof(sub->sock_fd), &sub, sizeof(sub));

	/* Print message with current state at stdout.*/
	printf("New client %s connected from %s:%i.\n", sub->id,
		inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
}

void handle_listen_tcp_msg(srv_t *const srv) {
	struct sockaddr_in client_addr;
	u_int client_size;
	int ret, flag;
	sub_t new_sub, *old_sub;

	/* Accept connection. */
	client_size = sizeof(client_addr);
	new_sub.sock_fd = accept(srv->tcp_sock_fd, (struct sockaddr*) &client_addr,
						 &client_size);
	DIE(new_sub.sock_fd == -1, "accept() failed\n");

	/* Deactivate Nagle's algorithm. */
	flag = 1;
	ret = setsockopt(new_sub.sock_fd, IPPROTO_TCP, TCP_NODELAY, &flag,
					 sizeof(int));
	DIE(ret == -1, "setsockopt() failed\n");

	/* Receive user's id. */ 
	ret = recv(new_sub.sock_fd, &new_sub.id, sizeof(new_sub.id), 0);
	DIE(ret == -1, "recv() failed\n");
	new_sub.id[ret]  ='\0';

	/* Have already a connection with this id or had it one in past? */
	if (ht_has_key(srv->subs_by_id, &new_sub.id)) {
		old_sub = *(sub_t **) ht_get(srv->subs_by_id, &new_sub.id);

		if (old_sub->sock_fd == -1) {	/* Past Connection */
			revive_sub_connection(srv, old_sub, new_sub.sock_fd, &client_addr);
		} else {	/* Present Connection */
			srv_closes_sub_connection(&new_sub);
			printf("Client %s already connected.\n", old_sub->id);
		}
	} else { 	/* First time connecting. */
		add_new_sub(srv, &new_sub, &client_addr);
	}
}

void rmv_subscription(sub_t *const sub, const char *const topic) {
	if (trie_search(sub->subscriptions, topic)) {
		trie_remove(sub->subscriptions, topic);
	}
}

void add_subscription(sub_t *const sub, const char *const topic) {
	char *val = "TOPIC";
	trie_insert(sub->subscriptions, topic, val);
}

void sub_closes_srv_connection(srv_t *const srv, sub_t *const sub,
							   int *const pfd_idx) {
	/* Close connection. */
	DIE(close(sub->sock_fd) == -1, "close() failed\n");
	sub->sock_fd = -1;

	/* Remove the connection's fd from hashmap. */
	ht_remove_entry(srv->subs_by_fd, &srv->pfds[*pfd_idx].fd);

	/* Shift all pfd from right to left. */
	for (int j = *pfd_idx; j < srv->nfds - 1; ++j) {
		srv->pfds[j] = srv->pfds[j + 1];
	}
	srv->nfds--;
	(*pfd_idx)--;

	/* Print new state at output. */
	printf("Client %s disconnected.\n", sub->id);
}

void handle_sub_msg(srv_t *const srv, int *const pfd_idx) {
	sub_t *sub;
	sub_msg_t msg;
	int ret;

	/* Get the sub's info. */
	sub = *(sub_t **) ht_get(srv->subs_by_fd, &srv->pfds[*pfd_idx].fd);

	/* Receive the message */
	ret = recv(sub->sock_fd, &msg, sizeof(msg), 0);
	DIE(ret == -1, "recv() failed\n");

	/* Handle the message. */
	if (msg.type == UNSUBSCRIBE_SUB_MSG) {
		rmv_subscription(sub, msg.topic);
	} else if (msg.type == SUBSCRIBE_SUB_MSG) {
		add_subscription(sub, msg.topic);
	} else if (msg.type == CLOSE_SUB_MSG) {
		sub_closes_srv_connection(srv, sub, pfd_idx);
	}
}

int main(int argc, char const* argv[]) {
	srv_t srv;

	/* Deactivate the buffering at stdout. */
	setvbuf(stdout, NULL, _IONBF, BUFSIZ);

	/* Get the server port. */
	DIE(argc != 2, "Usage: ./server <SERVER_PORT>\n");
	srv.port = (unsigned short) atoi(argv[1]); 

	init_udp_socket(&srv);
	init_tcp_socket(&srv);
	init_poll_fds(&srv);
	init_subs(&srv);

	while (1) {
		/* Wait until we can read something. */
		poll(srv.pfds, srv.nfds, -1);

		/* STDIN */
		if ((srv.pfds[0].revents & POLLIN) != 0) {
			handle_stdin_msg(&srv);
		}
		
		/* UDP CLIENT */
		if ((srv.pfds[1].revents & POLLIN) != 0) {
			handle_udp_msg(&srv);
		}
		
		/* LISTEN TCP */
		if ((srv.pfds[2].revents & POLLIN) != 0) {
			handle_listen_tcp_msg(&srv);
		}

		/* SUBSCRIBERS */
		for (int i = 3; i < srv.nfds; ++i) {
			if ((srv.pfds[i].revents & POLLIN) != 0) {
				handle_sub_msg(&srv, &i);
			}
		}
	}

	return 0;
}