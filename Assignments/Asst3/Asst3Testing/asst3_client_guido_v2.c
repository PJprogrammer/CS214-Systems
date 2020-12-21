#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <errno.h>

#define BUF_LEN 1000

#define RED "\033[1;31m"
#define YELLOW "\033[1;33m"
#define GREEN "\033[1;32m"
#define RESET "\033[0m"

void buffered_write(int fd, char *buf, int len) {
	int cur = 0;
	while (cur < len) {
		int nwrite = write(fd, buf + cur, len - cur);
		if (nwrite == -1) {
			fprintf(stderr, "%s", strerror(errno));
			exit(EXIT_FAILURE);
		}
		cur += nwrite;
	}
}

int client_connect(char *host, char *port) {
	// addrinfo specifies the type of network, connection, protocol, etc.
	struct addrinfo hints;
	// Clear all fields
	memset(&hints, 0, sizeof(hints));
	
	// What kind of network?
	hints.ai_family = AF_UNSPEC;	// We are OK with IPv4 and IPv6 addresses
	
	// What type of connection?
	hints.ai_socktype = SOCK_STREAM;

	struct addrinfo *address_list;
	// Gets a list of all possible ways to connect to the server
	int error = getaddrinfo(host, port, &hints, &address_list);
	// host: the remote host
	// port: the service (by name, or port number)
	// hints: our additional requirements
	// address_list: the list of results
	if (error) {
		fprintf(stderr, "%s", gai_strerror(error));
		exit(EXIT_FAILURE);
	}

	struct addrinfo *addr;
	int sock;
	// Try every connection method
	for (addr = address_list; addr != NULL; addr = addr->ai_next) {
		// Create the socket
		sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
		
		// If failed, try the next method
		if (sock < 0) continue;
		
		// Connect to the remote host using the socket
		if (connect(sock, addr->ai_addr, addr->ai_addrlen) == 0) {
			break;
		}

		// Connection failed; close the socket and try the next method		
		close(sock);
	}
	
	if (addr == NULL) {
		// Couldn't set up a socket
		fprintf(stderr, "Could not connect to %s:%s\n", host, port);
		exit(EXIT_FAILURE);
	}
	
	// Free the list because we won't use it any more
	freeaddrinfo(address_list);

	return sock;
}

void warning(char *msg) {
	printf("%sWARNING: %s%s\n", YELLOW, msg, RESET);
}

int parse_reg(char *payload) {
	char *token = strtok(NULL, "|");
	if (token == NULL)
		return 0;

	int len = atoi(token);

	if (len < 2)
		warning("invalid message length");

	token = strtok(NULL, "|");
	if (token == NULL)
		return 0;

	if (len > strlen(token))
		return 0;

	if (len < strlen(token))
		warning("mismatching length");

	char punct = token[len - 1];
	if (punct != '.' &&
		punct != '!' &&
		punct != '?')
		warning("missing punctuation sign");
	else
		token[len - 1] = '\0';
	
	strcpy(payload, token);

	token = strtok(NULL, "|");
	if (token != NULL)
		warning("message continues after the last segment; ignoring remainder");

	return 1;
}

int parse_err(char *payload) {
	char *token = strtok(NULL, "|");
	if (token == NULL)
		return 0;

	if (strlen(token) != 4 ||
		token[0] != 'M' ||
			(strcmp(token + 2, "CT") != 0 &&
			strcmp(token + 2, "LN") != 0 &&
			strcmp(token + 2, "FT") != 0))
		warning("invalid error code");

	strcpy(payload, token);

	return 1;
}

// Parses message. Returns 1 iff message is complete. Specifically:
// - The message has type REG or ERR.
// - If the type is REG, it has a length and a payload fields.
// - If the type is ERR, it has a code field.
// The function prints warnings if the fields are inconsistent.
// The function modifies message, so make a copy before calling.
// Stores the message payload in the homonymous buffer.
int parse_message(char *message, char *payload) {
	int len = strlen(message);
	if (len == 0)
		return 0;	// This should never run

	char *token = strtok(message, "|");
	if (strcmp(token, "REG") == 0)
		return parse_reg(payload);
	else if (strcmp(token, "ERR") == 0)
		return parse_err(payload);
	else
		return 0;
}

void read_message(int sock, char *message, char *payload) {
	memset(message, 0, BUF_LEN);
	memset(payload, 0, BUF_LEN);
	while (1) {
		char tmp[BUF_LEN];
		memset(tmp, 0, BUF_LEN);
		if (read(sock, tmp, BUF_LEN) == 0) {
			printf("SERVER: EOF\n");
			return;
		}
		printf("SERVER: %s\n", tmp);
		strcat(message, tmp);
		char copy[BUF_LEN];
		memset(copy, 0, BUF_LEN);
		strcpy(copy, message);
		if (parse_message(copy, payload))
			break;
	}
}

void write_message(int sock, char *message) {
	buffered_write(sock, message, strlen(message));
	printf("CLIENT: %s\n", message);
}

void check_kk(char *kk) {
	if (strcmp(kk, "Knock, knock") != 0
		&& strcmp(kk, "Knock, Knock") != 0
		&& strcmp(kk, "KNOCK, KNOCK") != 0
		&& strcmp(kk, "knock, knock") != 0
		&& strcmp(kk, "Knock knock") != 0
		&& strcmp(kk, "Knock! Knock!") != 0)
		warning("not a knock, knock message");
}

void wait_for_close(int sock) {
	printf("Waiting for EOF...\n");
	char buf;
	while (read(sock, &buf, 1) != 0);
	printf("SERVER: EOF\n");
}

void test_1(int sock) {
	char message[BUF_LEN];
	char payload[BUF_LEN];
	read_message(sock, message, payload);
	check_kk(payload);
	write_message(sock, "ERR|M0LN|");
	wait_for_close(sock);
}

void test_2(int sock) {
	char message[BUF_LEN];
	char payload[BUF_LEN];
	read_message(sock, message, payload);
	check_kk(payload);
	write_message(sock, "REG|5|Who's There.|");

	read_message(sock, message, payload);
	if (strcmp(message, "ERR|M1FT|") != 0)
		warning("not error M1FT");
}

void test_3(int sock) {
	char message[BUF_LEN];
	char payload[BUF_LEN];
	read_message(sock, message, payload);
	check_kk(payload);

	write_message(sock, "REG|12|Who's there?|");

	read_message(sock, message, payload);
	
	char size_of_message[5];
	memset(size_of_message, 0, 5);
	sprintf(size_of_message, "%i", (int)(strlen(payload) + 6));

	memset(message, 0, BUF_LEN);
	strcpy(message, "REG|");
	strcat(message, size_of_message);
	strcat(message, "|");
	strcat(message, payload);
	strcat(message, ", who?|");
	write_message(sock, message);

	read_message(sock, message, payload);

	write_message(sock, "REG|4|Meh.|");
}

void test_4(int sock) {
	char message[BUF_LEN];
	char payload[BUF_LEN];
	read_message(sock, message, payload);
	check_kk(payload);
}

int main(int argc, char **argv) {
	if (argc < 4) {
		printf("Usage: %s [host] [port] [test]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	char *host = argv[1];
	char *port = argv[2];
	int test = atoi(argv[3]);

	int sock = client_connect(host, port);

	if (test == 1) {
		test_1(sock);
	} else if (test == 2) {
		test_2(sock);
	} else if (test == 3) {
		test_3(sock);
	} else if (test == 4) {
		test_4(sock);
	}
	
	close(sock);

	return 0;	
}
