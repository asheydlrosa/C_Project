//
//  main.c
//  communication
//
//This code only has the communication function and must be added to the main code.
//Please keep in mind that the followng must be somewhere in the main code for function to be called:
//
//setupWiFiAP("Emergency_Network", "This is an emergency alert. Connect here for assistance.");
//
//  Created by Andrea Garcia on 2/4/25.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function to execute system commands
void executeCommand(const char *command) {
    int result = system(command);
    if (result != 0) {
        fprintf(stderr, "Error executing command: %s\n", command);
    }
}

// Function to set up Wi-Fi Access Point and captive portal
void setupWiFiAP(const char *ssid, const char *portalMessage) {
    // 1. Configure hostapd for Wi-Fi Access Point
    FILE *hostapdConfig = fopen("/etc/hostapd/hostapd.conf", "w");
    if (!hostapdConfig) {
        perror("Failed to open hostapd configuration file");
        return;
    }

    fprintf(hostapdConfig,
            "interface=wlan0\n"
            "driver=nl80211\n"
            "ssid=%s\n"
            "hw_mode=g\n"
            "channel=6\n"
            "macaddr_acl=0\n"
            "auth_algs=1\n"
            "ignore_broadcast_ssid=0\n",
            ssid);
    fclose(hostapdConfig);

    // Restart hostapd
    executeCommand("service hostapd restart");

    // 2. Set up iptables for captive portal redirection
    executeCommand("iptables -t nat -F");
    executeCommand("iptables -t nat -A PREROUTING -p tcp --dport 80 -j DNAT --to-destination 192.168.1.1:80");
    executeCommand("iptables -t nat -A POSTROUTING -j MASQUERADE");

    // 3. Start a lightweight web server and display portal message
    FILE *indexHtml = fopen("/var/www/html/index.html", "w");
    if (!indexHtml) {
        perror("Failed to open web server directory");
        return;
    }

    fprintf(indexHtml,
            "<html>"
            "<head><title>Emergency Alert</title></head>"
            "<body><h1>%s</h1></body>"
            "</html>",
            portalMessage);
    fclose(indexHtml);

    executeCommand("service lighttpd restart");

    printf("Wi-Fi Access Point '%s' is now active with a captive portal.\n", ssid);
}


int main(int argc, const char * argv[]) {
    setupWiFiAP("Emergency_Network", "This is an emergency alert. Connect here for assistance.");
    return 0;
}
