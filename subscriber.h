// Necula Mihail 323CAa 2024 - 2025
#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H

#include "msg.h"

/**********************
 * Commands
 **********************/
#define EXIT_CMD "exit"
#define SUBSCRIBE_CMD "subscribe"
#define UNSUBSCRIBE_CMD "unsubscribe"

/**********************
 * Outputs
 **********************/
#define SUBSCRIBED "Subscribed to topic %s\n"
#define UNSUBSCRIBED "Unsubscribed from topic %s\n"
#define INT "%s:%hu - %s - INT - %d\n"
#define SHORT_REAL "%s:%hu - %s - SHORT_REAL - %f.2\n"
#define FLOAT "%s:%hu - %s - FLOAT - %f.4\n"
#define STRING "%s:%hu - %s - STRING - %s\n"
#define INVALID_RECV_MSG "Received invalid message from server:" \
						 "topic - %s type - %d data - %s\n"

/**********************
 * The perspective of the subscriber about server.
 **********************/
typedef struct srv_t {
	const char *ip;
	unsigned short port;
} srv_t;

/**********************
 * The perspective of the subscriber about himself.
 **********************/
typedef struct sub_t {
	/* Info about the connection with the server */
	int sock_fd;
	const char *id;
	srv_t srv;

	/* Array with the events in which we are interested. */
	struct pollfd pfds[2];
	int nfds;
} sub_t;

/**********************
 * @brief Create a new socket, connect it to server and
 * send to server our id.
 **********************/
void connect_to_server(sub_t *const sub);

/**********************
 * @brief Fill the array with the events in which we
 * are interested.
 **********************/
void init_poll_fds(sub_t *const sub);

/**********************
 * @brief Execute the EXIT command. 
 *        This function should be called just from handle_stdin_msg();
 *
 * @param sub info
 * @param line which contains the command read from stdin
 **********************/
void stop_program(sub_t *const sub, char *const line);

/**********************
 * @brief Execute the SUBSCRIBE / UNSUBSCRIBE command.
 *		  This function should be called just from handle_stdin_msg();
 *
 * @param sub info
 * @param command read from stdin (subscribe or unsubscribe)
 **********************/
void update_subscriptions(sub_t *const sub, const char *const command);

/**********************
 * @brief Have a message (command) at stdin. We read it and execute
 * it if it's valid. The only valid commands are:
 *		exit,
 *		subscribe <topic>,
 *		unsubscribe <topic>.
 * The <topic> can not contain the next chars: ' ' and '\n'.
 **********************/
void handle_stdin_msg(sub_t *const sub);

/**********************
 * @brief Process an INT message from server and print it at stdout.
 **********************/
void print_int_msg(const srv_msg_t msg);

/**********************
 * @brief Process a SHORT_REAL message from server and print it at stdout.
 **********************/
void print_short_real_msg(const srv_msg_t msg);

/**********************
 * @brief Process a FLOAT message from server and print it at stdout.
 **********************/
void print_float_msg(const srv_msg_t msg);

/**********************
 * @brief Process a STRING message from server and print it at stdout.
 **********************/
void print_string_msg(const srv_msg_t msg);

/**********************
 * @brief Receive a message from server, process it and show it at stdout.
 **********************/
void handle_server_msg(sub_t *const sub);

#endif
