// Necula Mihail 323CAa 2024 - 2025
#ifndef MSG_H
#define MSG_H

/**********************
 * Type of messages from subscriber.
 **********************/
#define SUBSCRIBE_SUB_MSG 0
#define UNSUBSCRIBE_SUB_MSG 1
#define CLOSE_SUB_MSG 2

/**********************
 * Type of messages from server.
 **********************/
#define INT_SRV_MSG 0
#define SHORT_REAL_SRV_MSG 1
#define FLOAT_SRV_MSG 2
#define STRING_SRV_MSG 3
#define CLOSE_SRV_MSG 4

/**********************
 * Max sizes
 **********************/
#define MAX_TOPIC_SIZE 50
#define MAX_DATA_SIZE 1500
#define MAX_ID_SIZE 10

/**********************
 * Alphabets
 **********************/
#define TOPIC_ALPHABET "+*/-_."							\
					   "abcdefghijklmnopqrstuvwxyz" 	\
					   "ABCDEFGHIJKLMNOPQRTSUVWXYZ"		\
					   "0123456789"						\
					   "~`!@#$%^&*()+={}[];:'<>,?|\"\\"	\

/**********************
 * Structure to save the info of a subscriber's message.
 **********************/
typedef struct __attribute__((packed)) sub_msg_t {
	uint32_t type;
	char topic[MAX_TOPIC_SIZE + 1];
} sub_msg_t;

/**********************
 * Structure to save the info of a server's message to a subscriber.
 **********************/
typedef struct __attribute__((packed)) srv_msg_t {
	uint8_t type;
	uint32_t provider_ip;
	uint16_t provider_port;
	char topic[MAX_TOPIC_SIZE + 1];
	char data[MAX_DATA_SIZE + 1];
} srv_msg_t;

/**********************
 * Structure to save the info of a udp provider's message.
 **********************/
typedef struct __attribute__((packed)) udp_msg_t {
	char topic[MAX_TOPIC_SIZE];
	uint8_t type;
	char data[MAX_DATA_SIZE];
} udp_msg_t;

#endif
