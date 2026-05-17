#ifndef NOCTCTL_IPC_H
#define NOCTCTL_IPC_H

#include <stddef.h>

/**
 * noctctl IPC header
 * 
 * Communicates with the noctis compositor via Unix socket
 * Socket location: $XDG_RUNTIME_DIR/noctis.sock
 */

typedef struct {
    int socket_fd;
    char *socket_path;
} noctctl_ipc_t;

/**
 * Initialize IPC connection
 * Returns 0 on success, -1 on failure
 */
int noctctl_ipc_init(noctctl_ipc_t *ipc);

/**
 * Send command to noctis
 * Returns 0 on success, -1 on failure
 */
int noctctl_ipc_send(noctctl_ipc_t *ipc, const char *cmd);

/**
 * Receive response from noctis
 * buf should be at least NOCTCTL_RESPONSE_SIZE bytes
 * Returns number of bytes received, -1 on error
 */
int noctctl_ipc_recv(noctctl_ipc_t *ipc, char *buf, size_t len);

/**
 * Close IPC connection
 */
void noctctl_ipc_close(noctctl_ipc_t *ipc);

#define NOCTCTL_RESPONSE_SIZE 1024

#endif /* NOCTCTL_IPC_H */
