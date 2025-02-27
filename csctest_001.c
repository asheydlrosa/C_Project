/*
CSC stands for Communication Survivor Code
Runs a Web Server on Port 80
Displays an Emergency Selection Page
Logs Survivors' Emergency Levels
Automatically Runs at Boot
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pcap.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <microhttpd.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX_SURVIVORS 100

// Survivor structure
typedef struct {
    char mac_address[18];
    int emergency_level; // 1 = critical, 2 = serious, 3 = less urgent
} Survivor;

// Global list of survivors
Survivor survivors[MAX_SURVIVORS];
int survivor_count = 0;

// Function prototypes
void sniff_packets();
void packet_handler(u_char *, const struct pcap_pkthdr *, const u_char *);
int start_http_server();
int answer_request(void *, struct MHD_Connection *, const char *, const char *, const char *, const char *, size_t *, void **);
void sort_survivors();
void display_survivors();

int main() {
    printf("Starting Disaster Recovery Network...\n");

    // Start packet sniffing in a separate thread
    printf("Initializing Wi-Fi sniffer...\n");
    sniff_packets();

    // Start the HTTP server to interact with survivors
    printf("Starting HTTP Captive Portal...\n");
    start_http_server();

    return 0;
}

// Function to sniff Wi-Fi packets
void sniff_packets() {
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle;
    
    // Open the Realtek Wi-Fi interface in monitor mode
    handle = pcap_open_live("wlan0", BUFSIZ, 1, 1000, errbuf);
    if (handle == NULL) {
        fprintf(stderr, "Could not open device wlan0: %s\n", errbuf);
        exit(1);
    }

    // Capture packets indefinitely and pass them to packet_handler
    pcap_loop(handle, 0, packet_handler, NULL);
}

// Function to handle sniffed packets
void packet_handler(u_char *args, const struct pcap_pkthdr *header, const u_char *packet) {
    struct ether_header *eth_header = (struct ether_header *)packet;

    // Check if it's a Wi-Fi data packet
    if (ntohs(eth_header->ether_type) == ETHERTYPE_IP) {
        char src_mac[18];
        snprintf(src_mac, sizeof(src_mac), "%02X:%02X:%02X:%02X:%02X:%02X",
                 eth_header->ether_shost[0], eth_header->ether_shost[1],
                 eth_header->ether_shost[2], eth_header->ether_shost[3],
                 eth_header->ether_shost[4], eth_header->ether_shost[5]);

        // Check if the MAC address is already in the survivor list
        for (int i = 0; i < survivor_count; i++) {
            if (strcmp(survivors[i].mac_address, src_mac) == 0) {
                return; // Device already detected
            }
        }

        // Add new survivor
        if (survivor_count < MAX_SURVIVORS) {
            strcpy(survivors[survivor_count].mac_address, src_mac);
            survivors[survivor_count].emergency_level = 0; // No response yet
            survivor_count++;

            printf("Detected Device: %s\n", src_mac);
        }
    }
}

// Function to start HTTP server for captive portal
int start_http_server() {
    struct MHD_Daemon *daemon;
    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL, &answer_request, NULL, MHD_OPTION_END);
    if (daemon == NULL) {
        printf("Failed to start HTTP server\n");
        return 1;
    }

    printf("HTTP server running on port %d...\n", PORT);
    getchar(); // Keeps the server running
    MHD_stop_daemon(daemon);
    return 0;
}

// Function to handle HTTP requests (captive portal)
int answer_request(void *cls, struct MHD_Connection *connection, const char *url,
                   const char *method, const char *version, const char *upload_data,
                   size_t *upload_data_size, void **con_cls) {
    const char *page =
        "<html><body>"
        "<h1>Emergency Assistance</h1>"
        "<p>Please select your emergency level:</p>"
        "<form action='/' method='POST'>"
        "<input type='radio' name='level' value='1'> Level 1 (Critical - Life Threatening)<br>"
        "<input type='radio' name='level' value='2'> Level 2 (Serious - Could become life-threatening)<br>"
        "<input type='radio' name='level' value='3'> Level 3 (Less Urgent - Requires immediate attention)<br>"
        "<input type='submit' value='Submit'>"
        "</form></body></html>";

    struct MHD_Response *response;
    int ret;

    // If receiving POST data (user response)
    if (strcmp(method, "POST") == 0) {
        char *level = MHD_lookup_connection_value(connection, MHD_POSTDATA_KIND, "level");

        if (level) {
            int level_num = atoi(level);

            // Assign emergency level to the survivor
            for (int i = 0; i < survivor_count; i++) {
                if (survivors[i].emergency_level == 0) { // First response
                    survivors[i].emergency_level = level_num;
                    printf("Survivor at MAC %s classified as Level %d\n", survivors[i].mac_address, level_num);
                    sort_survivors();
                    display_survivors();
                    break;
                }
            }
        }
    }

    response = MHD_create_response_from_buffer(strlen(page), (void *)page, MHD_RESPMEM_PERSISTENT);
    ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    return ret;
}

// Function to sort survivors based on emergency level (Bubble Sort for simplicity)
void sort_survivors() {
    for (int i = 0; i < survivor_count - 1; i++) {
        for (int j = 0; j < survivor_count - i - 1; j++) {
            if (survivors[j].emergency_level > survivors[j + 1].emergency_level) {
                Survivor temp = survivors[j];
                survivors[j] = survivors[j + 1];
                survivors[j + 1] = temp;
            }
        }
    }
}

// Function to display the sorted survivor list
void display_survivors() {
    printf("\n--- Sorted Survivors List ---\n");
    for (int i = 0; i < survivor_count; i++) {
        printf("MAC: %s | Emergency Level: %d\n", survivors[i].mac_address, survivors[i].emergency_level);
    }
}
