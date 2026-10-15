#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/bspwm_0_0-socket"

// Helper function to send commands safely to bspwm
int send_bspwm_cmd(int sock_fd, char *args[], int argc) {
    // 1. Calculate the exact length of the message payload
    size_t total_len = 0;
    for (int i = 0; i < argc; i++) {
        total_len += strlen(args[i]) + 1; // Length of word + 1 byte for '\0'
    }

    // 2. Allocate a temporary buffer for the combined stream
    char *payload = malloc(total_len);
    if (!payload) return -1;

    // 3. Serialize arguments sequentially into the buffer with '\0' dividers
    char *ptr = payload;
    for (int i = 0; i < argc; i++) {
        size_t len = strlen(args[i]);
        memcpy(ptr, args[i], len);
        ptr[len] = '\0'; // Explicitly add null terminator
        ptr += len + 1;  // Move pointer forward past the null byte
    }

    // 4. Write the exact payload size to the socket
    ssize_t written = write(sock_fd, payload, total_len);
    free(payload);
    
    return (written == (ssize_t)total_len) ? 0 : -1;
}


int create_socket() {

    int sock_fd;
    struct sockaddr_un addr;

    sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(sock_fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1) {
        perror("Connect failed. Is bspwm running?");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

	return sock_fd;
}
