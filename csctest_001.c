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
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <unistd.h>

#define MAX_SURVIVORS 100

typedef struct {
    char mac_address[18];
    int emergency_level;
} Survivor;

Survivor survivors[MAX_SURVIVORS];
int survivor_count = 0;

void add_survivor(char *mac, int level) {
    if (survivor_count < MAX_SURVIVORS) {
        strcpy(survivors[survivor_count].mac_address, mac);
        survivors[survivor_count].emergency_level = level;
        survivor_count++;
    }
}

void sniff_devices() {
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle;
    struct pcap_pkthdr header;
    const u_char *packet;
    struct ether_header *eth_hdr;
    struct ip *ip_hdr;
    char mac_address[18];
    
    handle = pcap_open_live("wlan0", BUFSIZ, 1, 1000, errbuf);
    if (handle == NULL) {
        fprintf(stderr, "Couldn't open device: %s\n", errbuf);
        return;
    }
    
    while ((packet = pcap_next(handle, &header)) != NULL) {
        eth_hdr = (struct ether_header *)packet;
        snprintf(mac_address, sizeof(mac_address), "%02X:%02X:%02X:%02X:%02X:%02X",
                 eth_hdr->ether_shost[0], eth_hdr->ether_shost[1], eth_hdr->ether_shost[2],
                 eth_hdr->ether_shost[3], eth_hdr->ether_shost[4], eth_hdr->ether_shost[5]);
        
        printf("Detected device: %s\n", mac_address);
        add_survivor(mac_address, 0);
    }
    
    pcap_close(handle);
}

void send_message() {
    system("echo 'Emergency Network: Select Your Emergency Level' > /var/www/html/index.html");
}

void receive_response(char *mac, int level) {
    for (int i = 0; i < survivor_count; i++) {
        if (strcmp(survivors[i].mac_address, mac) == 0) {
            survivors[i].emergency_level = level;
            printf("Survivor %s updated to Level %d\n", mac, level);
            return;
        }
    }
}

void display_survivors() {
    printf("\nSurvivor List:\n");
    for (int i = 0; i < survivor_count; i++) {
        printf("MAC: %s, Level: %d\n", survivors[i].mac_address, survivors[i].emergency_level);
    }
}

int main() {
    printf("Starting Wi-Fi Sniffer for Survivor Detection...\n");
    sniff_devices();
    send_message();
    display_survivors();
    return 0;
}

