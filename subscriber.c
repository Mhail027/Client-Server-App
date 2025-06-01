// Necula Mihail 323CAa 2024 - 2025
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include "subscriber.h"
#include "utils.h"

void connect_to_server(sub_t *const sub) {
	int flag, ret;
	struct sockaddr_in server_addr;

	/* Create the socket. */
	sub->sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sub->sock_fd == -1, "socket() failed\n");

	/* Deactivate the Nagle's Algorithm. */
	flag = 1;
	ret = setsockopt(sub->sock_fd, IPPROTO_TCP, TCP_NODELAY, &flag,
					 sizeof(flag));
	DIE(ret == -1, "setsockopt() failed\n");

	/* Complete the server info. */
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(sub->srv.port);
	server_addr.sin_addr.s_addr = inet_addr(sub->srv.ip);

	/* Try to connect to server. */
	ret = connect(sub->sock_fd, (const struct sockaddr *) &server_addr,
				  sizeof(server_addr));
	DIE(ret == -1, "connect() failed\n");

	/* Send our id to server. */
	ret = send(sub->sock_fd, sub->id, strlen(sub->id), 0);
	DIE(ret == -1, "send() failed\n");
}

void init_poll_fds(sub_t *const sub) {
	/* We are interested in stdin. */
	sub->pfds[0].fd = STDIN_FILENO;
	sub->pfds[0].events = POLLIN;
	sub->pfds[0].revents = 0;

	/* We are interested in server. */
	sub->pfds[1].fd = sub->sock_fd;
	sub->pfds[1].events = POLLIN;
	sub->pfds[0].revents = 0;

	/* We care just of 2 files. */
	sub->nfds = 2;
}

void stop_program(sub_t *const sub, char *const line) {
	sub_msg_t msg;
	int ret;

	/* Does the command is valid? */
	if (strtok(NULL, " \n")) {
		printf("Invalid arguments for command.\n");
		return;
	}
	free(line);

	/* Inform the server that we close the connection. */
	memset(&msg, 0, sizeof(msg));
	msg.type = CLOSE_SUB_MSG;
	ret = send(sub->sock_fd, (char *) &msg, sizeof(msg), 0);
	DIE(ret == -1, "send() failed\n");
	
	/* Stop the program. */
	DIE(close(sub->sock_fd) == -1, "close() failed\n");
	exit(0);
}

void update_subscriptions(sub_t *const sub, const char *const command) {
	sub_msg_t msg;
	char *topic;
	int ret;

	/* Does the command is valid? */
	topic = strtok(NULL, " \n");
	if (strtok(NULL, " \n") != NULL) {
		printf("Invalid arguments for command.\n");
		return;
	} else if (strlen(topic) > MAX_TOPIC_SIZE) {
		printf("The topic can have maximum %d chars.\n", MAX_TOPIC_SIZE);
		return;
	}

	/* Create a message. */
	memset(&msg, 0, sizeof(msg));
	msg.type = !strcmp(command, SUBSCRIBE_CMD) ? SUBSCRIBE_SUB_MSG :
				UNSUBSCRIBE_SUB_MSG;
	memcpy(msg.topic, topic, strlen(topic));
	msg.topic[strlen(topic)] = '\0'; 

	/* Send the message to server. */
	ret = send(sub->sock_fd, (char *) &msg, sizeof(msg), 0);
	DIE(ret == -1, "send() failed\n");

	/* Confirm that the command executed. */
	const char *out_format = !strcmp(command, SUBSCRIBE_CMD) ? SUBSCRIBED :
							  UNSUBSCRIBED;
	printf(out_format, msg.topic);
}

void handle_stdin_msg(sub_t *const sub) {
	char *command, *line;
	size_t line_len;
	int ret;

	/* Get the current line. */
	line = NULL;
	line_len = 0;
	ret = getline(&line, &line_len, stdin);
	DIE(ret == -1, "getline() failed\n");

	/* Execute the command. */
	command = strtok(line, " \n");
	if (!command) {
		printf("Invalid command.\n");
	} else if (!strcmp(command, EXIT_CMD)) {
		stop_program(sub, line);
	} else if (!strcmp(command, SUBSCRIBE_CMD) || !strcmp(command, UNSUBSCRIBE_CMD)){
		update_subscriptions(sub, command);
	} else {
		printf("Invalid command.\n");
	}

	/* Free the allocated memory.*/
	free(line);
}

void print_int_msg(const srv_msg_t msg) {
	int val;
	uint8_t sign;
	char *provider_ip;

	/* Get the abs value of the number. */
	val = (int) ntohl(* (uint32_t *) &msg.data[1]);

	/* Put the correct sign. */
	sign = * (uint8_t *) &msg.data;
	val = (sign == 0) ? val : -val;

	/* Get the ip in dot form. */
	provider_ip = inet_ntoa(*(struct in_addr *) &msg.provider_ip);

	/* Print the message. */
	printf(INT, provider_ip, msg.provider_port, msg.topic, val);
}

void print_short_real_msg(const srv_msg_t msg) {
	double val;
	char *provider_ip;

	/* Get the number. */
	val = ((double) ntohs((* (uint16_t *) msg.data))) / 100;

	/* Get the ip in dot form. */
	provider_ip = inet_ntoa(*(struct in_addr *) &msg.provider_ip);

	/* Print the message. */
	printf(SHORT_REAL, provider_ip, msg.provider_port, msg.topic, val);
}

void print_float_msg(const srv_msg_t msg) {
	double val;
	uint8_t sign;
	char *provider_ip;

	/* Get abs value of number with the decimal point removed. */
	val = (double) ntohl(* (uint32_t *) &msg.data[1]);

	/* Get the correct sign. */
	sign = * (uint8_t *) msg.data;
	val = sign == 0 ? val : -val;

	/* Put the decimal point. */
	for (int i = 1; i <= * (uint8_t *) &msg.data[5]; ++i) {
		val /= 10;
	}

	/* Get the ip in dot form. */
	provider_ip = inet_ntoa(*(struct in_addr *) &msg.provider_ip);

	/* Print the message. */
	printf(FLOAT, provider_ip, msg.provider_port, msg.topic, val);
}

void print_string_msg(const srv_msg_t msg) {
	char *provider_ip;

	/* Get the ip in dot form. */
	provider_ip = inet_ntoa(*(struct in_addr *) &msg.provider_ip);

	/* Print the message. */
	printf(STRING, provider_ip, msg.provider_port, msg.topic, msg.data);
}

void handle_server_msg(sub_t *const sub) {
	srv_msg_t msg;
	int ret;

	/* Receive the message from server. */
	ret = recv(sub->sock_fd, (char *) &msg, sizeof(msg), 0);
	DIE(ret == -1, "recv() failed\n");

	/* Process the message and print it. */
	switch (msg.type) {
		case INT_SRV_MSG:
			print_int_msg(msg);
			return;

		case SHORT_REAL_SRV_MSG:
			print_short_real_msg(msg);
			return;

		case FLOAT_SRV_MSG:
			print_float_msg(msg);
			return;

		case STRING_SRV_MSG:
			print_string_msg(msg);
			return;

		case CLOSE_SRV_MSG:
			DIE(close(sub->sock_fd) == -1, "close() failed\n");
			exit(0);

		default:
			printf(INVALID_RECV_MSG, msg.topic, msg.type, msg.data);
			return;
	}
}

int main(int argc, char const* argv[]) {
	sub_t sub;

	/* Deactivate the buffering at stdout. */
	setvbuf(stdout, NULL, _IONBF, BUFSIZ);

	/* Get the arguments. */
	DIE(argc != 4, "Usage: ./subscriber <CLIENT_IP> <SERVER_IP> "
					"<SERVER_PORT>\n");
	sub.id = argv[1];
	sub.srv.ip = argv[2];
	sub.srv.port = (unsigned short) atoi(argv[3]);

	connect_to_server(&sub);
	init_poll_fds(&sub);

	while (1) {
		/* Wait until we can read something. */
		poll(sub.pfds, sub.nfds, -1);

		if ((sub.pfds[0].revents & POLLIN) != 0) {
			handle_stdin_msg(&sub);
			sub.pfds[0].revents = 0;
		}
		
		if ((sub.pfds[1].revents & POLLIN) != 0) {
			handle_server_msg(&sub);
			sub.pfds[1].revents = 0;
		}
	}

	return 0;
}