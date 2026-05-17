#include "noctctl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * noctctl - IPC client for the noctis Wayland compositor
 * 
 * Usage: noctctl <command> [args]
 */

typedef struct {
    const char *name;
    const char *description;
    int min_args;
    int max_args;
} command_t;

static const command_t commands[] = {
    {"reload", "Reload config file live", 0, 0},
    {"kill", "Close focused window", 0, 0},
    {"exit", "Exit the compositor", 0, 0},
    {"focus", "Focus next/prev window", 1, 1},
    {"exec", "Run any command", 1, -1},
    {"gap", "Set gap size in pixels", 1, 1},
    {"border", "Change border color (active/inactive)", 2, 2},
    {"ratio", "Set master pane ratio", 1, 1},
    {NULL, NULL, 0, 0}
};

/**
 * Print usage information
 */
static void print_usage(const char *prog_name) {
    fprintf(stdout, "noctctl - IPC client for the noctis Wayland compositor\n\n");
    fprintf(stdout, "USAGE:\n");
    fprintf(stdout, "    %s <command> [args]\n\n", prog_name);
    fprintf(stdout, "COMMANDS:\n");
    
    for (int i = 0; commands[i].name != NULL; i++) {
        fprintf(stdout, "    %-15s  %s\n", commands[i].name, commands[i].description);
    }
    
    fprintf(stdout, "\nEXAMPLES:\n");
    fprintf(stdout, "    # Reload config after editing it\n");
    fprintf(stdout, "    %s reload\n\n", prog_name);
    fprintf(stdout, "    # Launch apps\n");
    fprintf(stdout, "    %s exec firefox\n", prog_name);
    fprintf(stdout, "    %s exec \"kitty --class floating\"\n\n", prog_name);
    fprintf(stdout, "    # Adjust layout on the fly\n");
    fprintf(stdout, "    %s gap 10\n", prog_name);
    fprintf(stdout, "    %s ratio 0.6\n\n", prog_name);
    fprintf(stdout, "    # Change border colors\n");
    fprintf(stdout, "    %s border active \"#AB6C6A\"\n", prog_name);
    fprintf(stdout, "    %s border inactive \"#333333\"\n", prog_name);
}

/**
 * Validate command and arguments
 */
static int validate_command(const char *cmd, int arg_count) {
    for (int i = 0; commands[i].name != NULL; i++) {
        if (strcmp(commands[i].name, cmd) == 0) {
            if (arg_count < commands[i].min_args) {
                fprintf(stderr, "Error: command '%s' requires at least %d argument(s)\n", 
                        cmd, commands[i].min_args);
                return -1;
            }
            if (commands[i].max_args >= 0 && arg_count > commands[i].max_args) {
                fprintf(stderr, "Error: command '%s' takes at most %d argument(s)\n", 
                        cmd, commands[i].max_args);
                return -1;
            }
            return 0;
        }
    }
    fprintf(stderr, "Error: unknown command '%s'\n", cmd);
    return -1;
}

/**
 * Build command string from arguments
 */
static char* build_command(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Error: no command specified\n");
        return NULL;
    }

    const char *cmd = argv[1];
    int arg_count = argc - 2;

    // Validate command
    if (validate_command(cmd, arg_count) < 0) {
        return NULL;
    }

    // Calculate total length needed
    size_t total_len = strlen(cmd) + 1; // command + space
    for (int i = 2; i < argc; i++) {
        total_len += strlen(argv[i]) + 1; // arg + space
    }
    total_len++; // null terminator

    // Allocate and build command string
    char *command = malloc(total_len);
    if (!command) {
        fprintf(stderr, "Error: memory allocation failed\n");
        return NULL;
    }

    strcpy(command, cmd);
    for (int i = 2; i < argc; i++) {
        strcat(command, " ");
        strcat(command, argv[i]);
    }

    return command;
}

/**
 * Main entry point
 */
int main(int argc, char *argv[]) {
    // Show help if no args
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    // Show help for --help or -h
    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
        print_usage(argv[0]);
        return 0;
    }

    // Show version if requested
    if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0) {
        fprintf(stdout, "noctctl version 0.1.0\n");
        return 0;
    }

    // Build command string
    char *command = build_command(argc, argv);
    if (!command) {
        return 1;
    }

    // Initialize IPC connection
    noctctl_ipc_t ipc = {.socket_fd = -1, .socket_path = NULL};
    if (noctctl_ipc_init(&ipc) < 0) {
        fprintf(stderr, "Error: Failed to connect to noctis compositor\n");
        fprintf(stderr, "Make sure noctis is running and XDG_RUNTIME_DIR is set\n");
        free(command);
        return 1;
    }

    // Send command
    if (noctctl_ipc_send(&ipc, command) < 0) {
        fprintf(stderr, "Error: Failed to send command to noctis\n");
        noctctl_ipc_close(&ipc);
        free(command);
        return 1;
    }

    // Receive response
    char response[NOCTCTL_RESPONSE_SIZE];
    int recv_result = noctctl_ipc_recv(&ipc, response, sizeof(response));
    
    noctctl_ipc_close(&ipc);
    free(command);

    if (recv_result < 0) {
        fprintf(stderr, "Error: Failed to receive response from noctis\n");
        return 1;
    }

    // Print response and check for errors
    if (strncmp(response, "ok", 2) == 0) {
        // Success - response is just "ok"
        return 0;
    } else if (strncmp(response, "error", 5) == 0) {
        // Error response from compositor
        fprintf(stderr, "%s\n", response);
        return 1;
    } else {
        // Some other response - print it
        fprintf(stdout, "%s\n", response);
        return 0;
    }

    return 0;
}