/*
CSC stands for Communication Survivor Code
Runs a Web Server on Port 80
Displays an Emergency Selection Page
Logs Survivors' Emergency Levels
Automatically Runs at Boot
*/
#include <microhttpd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define PORT 80
#define RESPONSE_TEMPLATE "<html><head><title>Emergency Help Request</title></head><body>\
<h1>Emergency Help Request</h1>\
<p>Select your emergency level:</p>\
<form method='GET' action='/log'>\
<button name='level' value='1' type='submit'>Level 1 - Critical</button><br>\
<button name='level' value='2' type='submit'>Level 2 - Serious</button><br>\
<button name='level' value='3' type='submit'>Level 3 - Less Urgent</button>\
</form></body></html>"

#define LOG_FILE "/var/log/captive_portal.log"

struct MHD_Daemon *server_daemon = NULL; // Renamed from `daemon` to `server_daemon`

// Function to write emergency responses to a log file
void log_emergency(const char *level, const char *ip) {
    FILE *log = fopen(LOG_FILE, "a");
    if (log == NULL) {
        perror("Error opening log file");
        return;
    }
    fprintf(log, "IP: %s, Emergency Level: %s\n", ip, level);
    fclose(log);
}

// Fixed: Function signature must return `enum MHD_Result`
enum MHD_Result answer_to_connection(void *cls, struct MHD_Connection *connection,
                                     const char *url, const char *method,
                                     const char *version, const char *upload_data,
                                     size_t *upload_data_size, void **con_cls) {
    struct MHD_Response *response;
    int ret;

    if (strcmp(url, "/log") == 0) {
        const char *level = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "level");
        const char *client_ip = MHD_lookup_connection_value(connection, MHD_CONNECTION_INFO_CLIENT_ADDRESS, NULL);
        
        if (level) {
            log_emergency(level, client_ip);
        }

        response = MHD_create_response_from_buffer(strlen(RESPONSE_TEMPLATE),
                                                   (void *) RESPONSE_TEMPLATE,
                                                   MHD_RESPMEM_PERSISTENT);
    } else {
        response = MHD_create_response_from_buffer(strlen(RESPONSE_TEMPLATE),
                                                   (void *) RESPONSE_TEMPLATE,
                                                   MHD_RESPMEM_PERSISTENT);
    }

    ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return ret;
}

// Function to start the Captive Portal
void start_server() {
    if (server_daemon == NULL) {
        server_daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL,
                                         &answer_to_connection, NULL, MHD_OPTION_END);
        if (server_daemon == NULL) {
            printf("Failed to start Captive Portal.\n");
            exit(1);
        }
        printf("Captive Portal is running. Press ENTER to stop...\n");
    } else {
        printf("Server is already running.\n");
    }
}

// Function to stop the Captive Portal
void stop_server() {
    if (server_daemon != NULL) {
        MHD_stop_daemon(server_daemon);
        server_daemon = NULL;
        printf("Captive Portal has been stopped.\n");
    } else {
        printf("No server is currently running.\n");
    }
}

// Main function to control starting/stopping manually
int main() {
    char input;
    printf("Captive Portal Control\n");
    printf("Press 's' to start scanning for survivors.\n");
    printf("Press 'q' to stop the search and exit.\n");

    while (1) {
        input = getchar();
        if (input == 's') {
            start_server();
        } else if (input == 'q') {
            stop_server();
            break;
        }
    }
    return 0;
}
