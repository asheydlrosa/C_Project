#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#define MAX_DEVICES 1000
#define ASHLEY_MAC "00:15:5D:96:C0:1C"  // Replace with Ashley's actual MAC address

double detection_radius = 5.0;  // Default detection radius (user-defined)
char detected_macs[MAX_DEVICES][18];
int detected_count = 0;

// Function to execute system commands
void executeCommand(const char *command) {
    int result = system(command);
    if (result != 0) {
        fprintf(stderr, "Error executing command: %s\n", command);
    }
}

// Function to set up Wi-Fi Access Point and captive portal
void setupWiFiAP(const char *ssid, const char *portalMessage) {
    FILE *hostapdConfig = fopen("/etc/hostapd/hostapd.conf", "w");
    if (!hostapdConfig) {
        perror("Failed to open hostapd configuration file");
        return;
    }
    fprintf(hostapdConfig,
            "interface=wlan0mon\n"
            "driver=nl80211\n"
            "ssid=%s\n"
            "hw_mode=g\n"
            "channel=6\n"
            "macaddr_acl=0\n"
            "auth_algs=1\n"
            "ignore_broadcast_ssid=0\n",
            ssid);
    fclose(hostapdConfig);

    executeCommand("service hostapd restart");
    executeCommand("iptables -t nat -F");
    executeCommand("iptables -t nat -A PREROUTING -p tcp --dport 80 -j DNAT --to-destination 192.168.1.1:80");
    executeCommand("iptables -t nat -A POSTROUTING -j MASQUERADE");

    FILE *indexHtml = fopen("/var/www/html/index.html", "w");
    if (!indexHtml) {
        perror("Failed to open web server directory");
        return;
    }
    fprintf(indexHtml, "<html><head><title>Emergency Alert</title></head><body><h1>%s</h1></body></html>", portalMessage);
    fclose(indexHtml);

    executeCommand("service lighttpd restart");
    printf("Wi-Fi Access Point '%s' is now active with a captive portal.\n", ssid);
}

// Function to check if a MAC address is unique
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

    if (is_unique_mac(mac_addr)) {
        if (detected_count < MAX_DEVICES) {
            strcpy(detected_macs[detected_count], mac_addr);
            detected_count++;
            printf("New Device Detected: %s | Total Devices: %d\n", mac_addr, detected_count);

            // Send connection message
            printf("[INFO] Sending connection message to %s\n", mac_addr);

            // Check if the detected MAC address is Ashley's device
            if (strcmp(mac_addr, ASHLEY_MAC) == 0) {
                printf("[ALERT] Ashley's device has connected! MAC: %s\n", mac_addr);
            }
        }
    }
}

int start_packet_capture() {
    char* dev = "wlan0mon";
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t* handle;

    handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);
    if (handle == NULL) {
        fprintf(stderr, "Couldn't open device %s: %s\n", dev, errbuf);
        return 2;
    }

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

    printf("Scanning for Wi-Fi devices within %.2f meters...\n", detection_radius);
    pcap_loop(handle, 0, packet_handler, NULL);

    pcap_close(handle);
    return 0;
}

int main() {
    printf("Enter detection radius (meters): ");
    scanf("%lf", &detection_radius);

    printf("Setting up Emergency Wi-Fi Network...\n");
    setupWiFiAP("Emergency_Network", "This is an emergency alert. Connect here for assistance.");
    
    printf("Starting Wi-Fi sniffer...\n");
    return start_packet_capture();
}
