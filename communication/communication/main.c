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

// Define structures for survivor queues
typedef struct Survivor {
    char name[50];
    int people_count;
    int severe_injury; // 1 = Severe injury, 0 = Minor/no injury
    struct Survivor *next;
} Survivor;

typedef struct Queue {
    Survivor *front;
    Survivor *rear;
} Queue;

// Initialize Immediate and Non-Immediate queues
Queue immediateHelpQueue = {NULL, NULL};
Queue nonImmediateQueue = {NULL, NULL};

// Function to enqueue a survivor
void enqueue(Queue *queue, const char *name, int people_count, int severe_injury) {
    Survivor *newSurvivor = (Survivor *)malloc(sizeof(Survivor));
    if (!newSurvivor) {
        fprintf(stderr, "Memory allocation failed.\n");
        return;
    }
    strcpy(newSurvivor->name, name);
    newSurvivor->people_count = people_count;
    newSurvivor->severe_injury = severe_injury;
    newSurvivor->next = NULL;

    if (queue->rear == NULL) {
        queue->front = queue->rear = newSurvivor;
    } else {
        queue->rear->next = newSurvivor;
        queue->rear = newSurvivor;
    }
}

// Function to execute system commands
void executeCommand(const char *command) {
    int result = system(command);
    if (result != 0) {
        fprintf(stderr, "Error executing command: %s\n", command);
    }
}

// Function to set up Wi-Fi Access Point and captive portal
void setupWiFiAP(const char *ssid) {
    // Configure hostapd for Wi-Fi Access Point
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

    // Set up iptables for captive portal redirection
    executeCommand("iptables -t nat -F");
    executeCommand("iptables -t nat -A PREROUTING -p tcp --dport 80 -j DNAT --to-destination 192.168.4.1:80");
    executeCommand("iptables -t nat -A POSTROUTING -j MASQUERADE");

    // Create the assistance request web page
    FILE *indexHtml = fopen("/var/www/html/index.html", "w");
    if (!indexHtml) {
        perror("Failed to open web server directory");
        return;
    }

    fprintf(indexHtml,
            "<html>"
            "<head><title>Emergency Assistance</title></head>"
            "<body>"
            "<h2>Do you need immediate assistance?</h2>"
            "<form action='/assistance' method='POST'>"
            "<input type='radio' name='assistance' value='yes'> Yes<br>"
            "<input type='radio' name='assistance' value='no'> No<br>"
            "<input type='submit' value='Submit'>"
            "</form>"
            "</body>"
            "</html>");
    fclose(indexHtml);

    // Assistance processing page
    FILE *assistHtml = fopen("/var/www/html/assistance.php", "w");
    if (!assistHtml) {
        perror("Failed to open assistance page.");
        return;
    }

    fprintf(assistHtml,
            "<?php\n"
            "if ($_SERVER['REQUEST_METHOD'] == 'POST') {\n"
            "    $assistance = $_POST['assistance'];\n"
            "    if ($assistance == 'yes') {\n"
            "        echo '<h2>Is someone severely injured?</h2>';\n"
            "        echo '<form action=\"/severe.php\" method=\"POST\">';\n"
            "        echo '<input type=\"radio\" name=\"severe\" value=\"yes\"> Yes<br>';\n"
            "        echo '<input type=\"radio\" name=\"severe\" value=\"no\"> No<br>';\n"
            "        echo '<input type=\"submit\" value=\"Submit\">';\n"
            "        echo '</form>';\n"
            "    } else {\n"
            "        echo '<h2>Please provide more information:</h2>';\n"
            "        echo '<form action=\"/nonimmediate.php\" method=\"POST\">';\n"
            "        echo 'Number of people: <input type=\"number\" name=\"people\" required><br>';\n"
            "        echo 'Minor injuries (yes/no): <input type=\"text\" name=\"injuries\" required><br>';\n"
            "        echo '<input type=\"submit\" value=\"Submit\">';\n"
            "        echo '</form>';\n"
            "    }\n"
            "}\n"
            "?>");
    fclose(assistHtml);

    // Process Severe Injury Response
    FILE *severePhp = fopen("/var/www/html/severe.php", "w");
    if (!severePhp) {
        perror("Failed to create severe injury processing page.");
        return;
    }

    fprintf(severePhp,
            "<?php\n"
            "if ($_SERVER['REQUEST_METHOD'] == 'POST') {\n"
            "    $severe = $_POST['severe'];\n"
            "    if ($severe == 'yes') {\n"
            "        echo '<h2>You have been placed at the top of the queue for emergency rescue.</h2>';\n"
            "    } else {\n"
            "        echo '<h2>You are in the queue for immediate assistance.</h2>';\n"
            "    }\n"
            "}\n"
            "?>");
    fclose(severePhp);

    // Process Non-Immediate Response
    FILE *nonImmediatePhp = fopen("/var/www/html/nonimmediate.php", "w");
    if (!nonImmediatePhp) {
        perror("Failed to create non-immediate processing page.");
        return;
    }

    fprintf(nonImmediatePhp,
            "<?php\n"
            "if ($_SERVER['REQUEST_METHOD'] == 'POST') {\n"
            "    $people = $_POST['people'];\n"
            "    $injuries = $_POST['injuries'];\n"
            "    echo '<h2>Thank you. We have recorded that there are ' . $people . ' people here.</h2>';\n"
            "    echo '<h2>Minor injuries reported: ' . $injuries . '</h2>';\n"
            "}\n"
            "?>");
    fclose(nonImmediatePhp);

    // Restart web server
    executeCommand("service lighttpd restart");

    printf("Wi-Fi AP '%s' active. Assistance request system is running.\n", ssid);
}


int main(int argc, const char * argv[]) {
    setupWiFiAP("Emergency_Network", "This is an emergency alert. Connect here for assistance.");
    return 0;
}
