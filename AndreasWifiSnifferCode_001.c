//This version of the Alexis code will prompt us to 
//select the radius of the sniffer and then it will
//look for ashleys laptop
//Test 001
//Done on 02/18/2025 @ 16:58PM

#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#define MAX_DEVICES 1000  

// Target MAC addresses to detect
#define TARGET_MAC1 "00:15:5D:96:C0:1C"  // Physical MAC address
#define TARGET_MAC2 "C4:03:A8:F2:E4:F5"  // Wireless LAN adapter MAC address

char detected_macs[MAX_DEVICES][18];  
int detected_count = 0;  
double detection_radius; 

// Function to check if a MAC address is already recorded
int is_unique_mac(const char* mac_addr) {
    for (int i = 0; i < detected_count; i++) {
        if (strcmp(detected_macs[i], mac_addr) == 0) {
            return 0;  
        }
    }
    return 1;  
}

// Packet handler function
void packet_handler(unsigned char* user, const struct pcap_pkthdr* header, const unsigned char* packetData) {
    char mac_addr[18];

    snprintf(mac_addr, sizeof(mac_addr), "%02X:%02X:%02X:%02X:%02X:%02X",
        packetData[10], packetData[11], packetData[12],
        packetData[13], packetData[14], packetData[15]);

    // Check for Ashley's computer
    if (strcmp(mac_addr, TARGET_MAC1) == 0 || strcmp(mac_addr, TARGET_MAC2) == 0) {
        printf("\n🔍 We detected Ashley's computer! MAC: %s\n\n", mac_addr);
    }

    // If MAC is unique, add it to the list
    if (is_unique_mac(mac_addr)) {
        if (detected_count < MAX_DEVICES) {
            strcpy(detected_macs[detected_count], mac_addr);
            detected_count++;
            printf("New Wi-Fi Device Detected: %s | Total Devices: %d\n", mac_addr, detected_count);
        }
    }
}

int start_packet_capture() {
    char* dev = "wlan0mon";  // Change wlan1 to wlan0 
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t* handle;

    // Open the device for live capture
    handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);
    if (handle == NULL) {
        fprintf(stderr, "Couldn't open device %s: %s\n", dev, errbuf);
        return 2;
    }

    // Apply a filter for probe request frames
    struct bpf_program compiled_filter;
    char filter_name[] = "type mgt subtype probe-req";  
    if (pcap_compile(handle, &compiled_filter, filter_name, 0, PCAP_NETMASK_UNKNOWN) == -1) {
        fprintf(stderr, "Couldn't parse filter %s: %s\n", filter_name, pcap_geterr(handle));
        return 2;
    }
    if (pcap_setfilter(handle, &compiled_filter) == -1) {
        fprintf(stderr, "Couldn't install filter %s: %s\n", filter_name, pcap_geterr(handle));
        return 2;
    }

    // Start capturing packets
    printf("Scanning for Wi-Fi devices within %.2f meters...\n", detection_radius);
    pcap_loop(handle, 0, packet_handler, NULL);

    // Close the handle
    pcap_close(handle);
    return 0;
}

int main() {
    printf("Enter detection radius (meters): ");
    scanf("%lf", &detection_radius);

    printf("Starting Wi-Fi sniffer...\n");
    return start_packet_capture();
}
