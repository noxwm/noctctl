#include "noctctl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

/**
 * Get the noctis socket path from XDG_RUNTIME_DIR
 */
static char* get_socket_path(void) {
    const char *runtime_dir = getenv("XDG_RUNTIME_DIR");
    if (!runtime_dir) {
        fprintf(stderr, "Error: XDG_RUNTIME_DIR not set\n");
        return NULL;
    }

    size_t path_len = strlen(runtime_dir) + strlen("/noctis.sock") + 1;
    char *socket_path = malloc(path_len);
    if (!socket_path) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return NULL;
    }

    snprintf(socket_path, path_len, "%s/noctis.sock", runtime_dir);
    return socket_path;
}

/**
 * Initialize IPC connection to noctis
 */
int noctctl_ipc_init(noctctl_ipc_t *ipc) {
    if (!ipc) {
        fprintf(stderr, "Error: Invalid IPC structure\n");
        return -1;
    }

    // Get socket path
    ipc->socket_path = get_socket_path();
    if (!ipc->socket_path) {
        return -1;
    }

    // Create Unix socket
    ipc->socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (ipc->socket_fd < 0) {
        fprintf(stderr, "Error: Failed to create socket: %s\n", strerror(errno));
        free(ipc->socket_path);
        return -1;
    }

    // Setup socket address
    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, ipc->socket_path, sizeof(addr.sun_path) - 1);

    // Connect to noctis
    if (connect(ipc->socket_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        fprintf(stderr, "Error: Failed to connect to noctis socket at %s: %s\n", 
                ipc->socket_path, strerror(errno));
        close(ipc->socket_fd);
        free(ipc->socket_path);
        return -1;
    }

    return 0;
}

/**
 * Send command to noctis via socket
 */
int noctctl_ipc_send(noctctl_ipc_t *ipc, const char *cmd) {
    if (!ipc || !cmd) {
        fprintf(stderr, "Error: Invalid parameters\n");
        return -1;
    }

    size_t cmd_len = strlen(cmd);
    ssize_t sent = send(ipc->socket_fd, cmd, cmd_len, 0);

    if (sent < 0) {
        fprintf(stderr, "Error: Failed to send command: %s\n", strerror(errno));
        return -1;
    }

    if ((size_t)sent != cmd_len) {
        fprintf(stderr, "Error: Incomplete send (sent %ld of %zu bytes)\n", sent, cmd_len);
        return -1;
    }

    return 0;
}

/**
 * Receive response from noctis
 */
int noctctl_ipc_recv(noctctl_ipc_t *ipc, char *buf, size_t len) {
    if (!ipc || !buf || len == 0) {
        fprintf(stderr, "Error: Invalid parameters\n");
        return -1;
    }

    ssize_t received = recv(ipc->socket_fd, buf, len - 1, 0);

    if (received < 0) {
        fprintf(stderr, "Error: Failed to receive response: %s\n", strerror(errno));
        return -1;
    }

    if (received == 0) {
        fprintf(stderr, "Error: Connection closed by noctis\n");
        return -1;
    }

    buf[received] = '\0';
    return (int)received;
}

/**
 * Close IPC connection
 */
void noctctl_ipc_close(noctctl_ipc_t *ipc) {
    if (!ipc) return;

    if (ipc->socket_fd >= 0) {
        close(ipc->socket_fd);
        ipc->socket_fd = -1;
    }

    if (ipc->socket_path) {
        free(ipc->socket_path);
        ipc->socket_path = NULL;
    }
}
