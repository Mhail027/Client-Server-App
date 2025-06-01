// Necula Mihail 323CAa 2024 - 2025
#ifndef SERVER_H
#define SERVER_H

#include "list.h"
#include "hashmap.h"
#include "trie.h"
#include "msg.h"

#define EXIT_CMD "exit"

/**********************
 * The perspective of the server about itself.
 **********************/
typedef struct srv_t {
	/* Info of server */
	unsigned short port;
	int udp_sock_fd;
	int tcp_sock_fd;

	/* Info about subscribers. */
	ht_t *subs_by_id;
	ht_t *subs_by_fd;
	ll_t *subs;

	/* Array with the events in which we are interested. */
	struct pollfd *pfds;
	uint32_t nfds;
	uint32_t max_nfds;
} srv_t;

/**********************
 * The perspective of the server about a subscriber.
 **********************/
typedef struct sub_t {
	/* Identification info */
	char id[MAX_ID_SIZE + 1];
	int sock_fd;

	/* His pleasures */
	trie_t *subscriptions;
} sub_t;

/**********************
 * @brief Bind a socket to a port.
 *
 * @param sock_fd: which will be bound to port
 * @param port: number
 **********************/
void bind_sock(const int sock_fd, const int port);

/**********************
 * @brief Create the socket for UDP clients and bind it to server's port.
 **********************/
void init_udp_socket(srv_t *const srv);

/**********************
 * @brief Create the socket for TCP clients, bind it to server's port
 * and put it to listen.
 **********************/
void init_tcp_socket(srv_t *const srv);

/**********************
 * @brief Fill the array with the events in which we
 * are interested.
 **********************/
void init_poll_fds(srv_t *const srv);

/**********************
 * @brief Allocate the memory for the 2 hashmaps and the list which
 * will contain the info about subscribers.  
 **********************/
void init_subs(srv_t *const srv);

/**********************
 * @brief Free the memory of a subscriber.
 **********************/
void free_sub(void *data);

/**********************
 * @brief Tell to a subscriber that we will stop his connection.
 *        Also, close the socket of that connection.
 **********************/
void srv_closes_sub_connection(sub_t *const sub);

/**********************
 * @brief Tell to all connections that will stop the server.
 *		  +
 *		  Free all allocated memory.
 *		  +
 *		  Shut down the server.
 **********************/
void stop_program(srv_t *const srv);

/**********************
 * @brief Execute the EXIT command.
 *		  This function should be called just from handle_stdin_msg();
 *
 * @param srv info
 * @param line which contains the command read from stdin
 **********************/
void execute_exit_cmd(srv_t *const srv, char *const line);

/**********************
 * @brief Have a message (command) at stdin. We read it and execute
 * it if it's valid. The only valid command is: exit.
 **********************/
void handle_stdin_msg(srv_t *const srv);

/**********************
 * @brief Add a new poll fd in the array with the events after which we
 * 		  watching. We will monitoring just that if we have something to
 *		  read fromthat fd.
 * 		  If we do not have space in array, we realloc and double its size.
 **********************/
void add_pfd(srv_t *const srv, const int fd);

/**********************
 * @brief Check if a topic is in the subscription's trie of a subscriber.
 *
 * @param topics the root of the trie
 * @param topic name which we'll be searched
 * @return true, if the subscriber wants notification from that topic
 *		   false, else
 **********************/
bool is_subscribed(trie_node_t *topics, char *topic);

/**********************
 * @brief Receive a message from an udp client and send it further to all
 * his subscribers.
 **********************/
void handle_udp_msg(srv_t *const srv) ;

/**********************
 * @brief A new subscriber connects for the first time at server. We add
 *		  his data in database.
 **********************/
void add_new_sub(srv_t *const srv, sub_t *const new_sub, 
				 const struct sockaddr_in *const addr);

/**********************
 * @brief A subscriber connects again after some time. We actualize his
 *		  socket fd and monitor this new fd.
 **********************/
void revive_sub_connection(srv_t *srv, sub_t *sub, u_int sock_fd,
						   struct sockaddr_in * addr);
/**********************
 * @brief Accept a connection with a subscriber, if his id is not active.
 **********************/
void handle_listen_tcp_msg(srv_t *const srv);


/**********************
 * @brief Remove the subscription of a client from a topic. If the client is
 *		  not subscribed to the given topic, nothing it is done.
 **********************/
void rmv_subscription(sub_t *const sub, const char *const topic);

/**********************
 * @brief Add a subscription to a topic for a client.
 **********************/
void add_subscription(sub_t *const sub, const char *const topic);

/**********************
 * @brief Close the connection with a subscriber. Eliminate the pfd of that
 *		  connection from everywhere.
 **********************/
void sub_closes_srv_connection(srv_t *const srv, sub_t *const sub,
							   int *const pfd_idx);

/**********************
 * @brief Receive a command from a subscribe and do it. These commands are of
 *		  3 types: subscribe to a topic, unsubscribe from a topic and close
 *		  the connection.
 *
 * @param sub info
 * @param pfd_idx of the sub
 **********************/
void handle_sub_msg(srv_t *const srv, int *const pfd_idx);

#endif
