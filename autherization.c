#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Function to register a new user
void register_user(const char *username, const char *password) {
    FILE *file = fopen("users.txt", "a");
    if (file == NULL) {
        perror("Error opening file");
        exit(1);
    }

    fprintf(file, "%s %s\n", username, password);
    fclose(file);
    printf("User %s registered successfully.\n", username);
}

// Function to authenticate a user
int authenticate_user(const char *username, const char *password) {
    char file_username[100], file_password[100];

    FILE *file = fopen("users.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(1);
    }

    // Check credentials line by line
    while (fscanf(file, "%s %s", file_username, file_password) != EOF) {
        if (strcmp(username, file_username) == 0 && strcmp(password, file_password) == 0) {
            fclose(file);
            return 1; // Authentication successful
        }
    }

    fclose(file);
    return 0; // Authentication failed
}

// Admin function to grant access
void grant_access() {
    char new_username[100], new_password[100];
    printf("Enter new username: ");
    scanf("%s", new_username);
    printf("Enter new password: ");
    scanf("%s", new_password);

    register_user(new_username, new_password);
}

// Main function
int main() {
    char username[100], password[100];
    int choice;

    printf("1. Admin Login\n2. User Login\nChoose an option: ");
    scanf("%d", &choice);

    if (choice == 1) {
        // Admin login
        printf("Enter admin username: ");
        scanf("%s", username);
        printf("Enter admin password: ");
        scanf("%s", password);

        if (strcmp(username, "admin") == 0 && strcmp(password, "adminpass") == 0) {
            printf("Admin login successful!\n");
            grant_access();
        } else {
            printf("Invalid admin credentials.\n");
        }
    } else if (choice == 2) {
        // User login
        printf("Enter username: ");
        scanf("%s", username);
        printf("Enter password: ");
        scanf("%s", password);

        if (authenticate_user(username, password)) {
            printf("User login successful!\n");
        } else {
            printf("Invalid username or password.\n");
        }
    } else {
        printf("Invalid choice.\n");
    }

    return 0;
}
