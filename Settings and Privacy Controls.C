/* settings_privacy.c
 *
 * Simple console app that simulates Settings & Privacy Controls:
 * - profile visibility
 * - notification preferences
 * - connected apps management
 * - sessions management
 * - data export (writes export_<username>.json)
 * - account deletion (simulated)
 *
 * Build:
 *   gcc -o settings_privacy settings_privacy.c
 * Run:
 *   ./settings_privacy
 *
 * This is a demonstration program for local use only.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_APPS 10
#define MAX_SESSIONS 10
#define MAX_NAME 64
#define STORAGE_FILE "user_settings.txt"

typedef enum { VIS_PUBLIC = 0, VIS_FRIENDS = 1, VIS_PRIVATE = 2 } Visibility;

typedef struct {
    char name[MAX_NAME];
} ConnectedApp;

typedef struct {
    int id;
    char device[MAX_NAME];
    char last_active[32];
} Session;

typedef struct {
    char username[MAX_NAME];
    Visibility visibility;
    int notifications_enabled; /* 0 false, 1 true */
    ConnectedApp apps[MAX_APPS];
    int app_count;
    Session sessions[MAX_SESSIONS];
    int session_count;
    int deleted; /* 0 = active, 1 = deleted */
} UserSettings;

/* Utility: current timestamp string */
void timestamp_now(char *buf, size_t len) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    strftime(buf, len, "%Y-%m-%d %H:%M:%S", tm);
}

/* Load settings from STORAGE_FILE if present */
void load_settings(UserSettings *u) {
    FILE *f = fopen(STORAGE_FILE, "r");
    if (!f) {
        /* defaults */
        u->visibility = VIS_PUBLIC;
        u->notifications_enabled = 1;
        u->app_count = 0;
        u->session_count = 0;
        u->deleted = 0;
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        char key[64], val[192];
        if (sscanf(line, "%63[^=]=%191[^\n]\n", key, val) < 2) continue;
        if (strcmp(key, "username") == 0) {
            strncpy(u->username, val, MAX_NAME-1);
        } else if (strcmp(key, "visibility") == 0) {
            u->visibility = atoi(val);
        } else if (strcmp(key, "notifications") == 0) {
            u->notifications_enabled = atoi(val);
        } else if (strcmp(key, "app_count") == 0) {
            u->app_count = atoi(val);
        } else if (strncmp(key, "app_", 4) == 0 && u->app_count < MAX_APPS) {
            int idx = atoi(key + 4);
            if (idx >= 0 && idx < MAX_APPS) {
                strncpy(u->apps[idx].name, val, MAX_NAME-1);
            }
        } else if (strcmp(key, "session_count") == 0) {
            u->session_count = atoi(val);
        } else if (strncmp(key, "session_", 8) == 0) {
            /* session_0 = id|device|last_active */
            int idx = atoi(key + 8);
            if (idx >= 0 && idx < MAX_SESSIONS) {
                int id;
                char device[MAX_NAME], last_active[32];
                if (sscanf(val, "%d|%63[^|]|%31[^\n]", &id, device, last_active) == 3) {
                    u->sessions[idx].id = id;
                    strncpy(u->sessions[idx].device, device, MAX_NAME-1);
                    strncpy(u->sessions[idx].last_active, last_active, 31);
                }
            }
        } else if (strcmp(key, "deleted") == 0) {
            u->deleted = atoi(val);
        }
    }
    fclose(f);
}

/* Save settings to STORAGE_FILE */
void save_settings(const UserSettings *u) {
    FILE *f = fopen(STORAGE_FILE, "w");
    if (!f) {
        printf("Error: cannot write settings file.\n");
        return;
    }
    fprintf(f, "username=%s\n", u->username);
    fprintf(f, "visibility=%d\n", u->visibility);
    fprintf(f, "notifications=%d\n", u->notifications_enabled);
    fprintf(f, "app_count=%d\n", u->app_count);
    for (int i = 0; i < u->app_count; ++i) {
        fprintf(f, "app_%d=%s\n", i, u->apps[i].name);
    }
    fprintf(f, "session_count=%d\n", u->session_count);
    for (int i = 0; i < u->session_count; ++i) {
        fprintf(f, "session_%d=%d|%s|%s\n", i, u->sessions[i].id, u->sessions[i].device, u->sessions[i].last_active);
    }
    fprintf(f, "deleted=%d\n", u->deleted);
    fclose(f);
}

/* Helper: pretty-print visibility */
const char* visibility_name(Visibility v) {
    switch (v) {
        case VIS_PUBLIC: return "Public";
        case VIS_FRIENDS: return "Friends-only";
        case VIS_PRIVATE: return "Only me (Private)";
        default: return "Unknown";
    }
}

/* Data export: write JSON-like file */
void export_data(const UserSettings *u) {
    char filename[128];
    snprintf(filename, sizeof(filename), "export_%s.json", u->username);
    FILE *f = fopen(filename, "w");
    if (!f) {
        printf("Failed to create export file.\n");
        return;
    }
    fprintf(f, "{\n");
    fprintf(f, "  \"username\": \"%s\",\n", u->username);
    fprintf(f, "  \"visibility\": \"%s\",\n", visibility_name(u->visibility));
    fprintf(f, "  \"notifications_enabled\": %d,\n", u->notifications_enabled);
    fprintf(f, "  \"connected_apps\": [\n");
    for (int i = 0; i < u->app_count; ++i) {
        fprintf(f, "    \"%s\"%s\n", u->apps[i].name, (i+1==u->app_count) ? "" : ",");
    }
    fprintf(f, "  ],\n");
    fprintf(f, "  \"sessions\": [\n");
    for (int i = 0; i < u->session_count; ++i) {
        fprintf(f, "    { \"id\": %d, \"device\": \"%s\", \"last_active\": \"%s\" }%s\n",
                u->sessions[i].id, u->sessions[i].device, u->sessions[i].last_active,
                (i+1==u->session_count) ? "" : ",");
    }
    fprintf(f, "  ]\n");
    fprintf(f, "}\n");
    fclose(f);
    printf("Data exported to %s\n", filename);
}

/* Account deletion: simulate by clearing storage and writing deletion marker */
void delete_account(UserSettings *u) {
    char confirm[8];
    printf("Are you sure you want to DELETE your account and data? This cannot be undone. (yes/no): ");
    if (!fgets(confirm, sizeof(confirm), stdin)) return;
    if (strncmp(confirm, "yes", 3) != 0) {
        printf("Account deletion cancelled.\n");
        return;
    }

    /* mark deleted, clear data in memory and save */
    u->deleted = 1;
    u->app_count = 0;
    u->session_count = 0;
    u->notifications_enabled = 0;
    u->visibility = VIS_PRIVATE;
    save_settings(u);

    /* optional: write deletion marker file */
    FILE *f = fopen("account_deleted.marker", "w");
    if (f) {
        fprintf(f, "User %s deleted at ", u->username);
        char t[64]; timestamp_now(t, sizeof(t));
        fprintf(f, "%s\n", t);
        fclose(f);
    }
    printf("Account deletion simulated and settings cleared locally.\n");
}

/* Add a sample session to simulate active sessions */
void add_sample_session(UserSettings *u) {
    if (u->session_count >= MAX_SESSIONS) return;
    int id = (u->session_count == 0) ? 1 : (u->sessions[u->session_count-1].id + 1);
    u->sessions[u->session_count].id = id;
    snprintf(u->sessions[u->session_count].device, MAX_NAME, "Device-%d", id);
    timestamp_now(u->sessions[u->session_count].last_active, sizeof(u->sessions[u->session_count].last_active));
    u->session_count++;
}

/* Menu operations */
void show_settings(const UserSettings *u) {
    printf("\n--- Settings for %s ---\n", u->username);
    if (u->deleted) {
        printf("Account status: DELETED (local simulation)\n");
        return;
    }
    printf("Profile visibility: %s\n", visibility_name(u->visibility));
    printf("Notifications enabled: %s\n", u->notifications_enabled ? "Yes" : "No");
    printf("Connected apps (%d):\n", u->app_count);
    for (int i = 0; i < u->app_count; ++i) printf("  %d. %s\n", i+1, u->apps[i].name);
    printf("Active sessions (%d):\n", u->session_count);
    for (int i = 0; i < u->session_count; ++i) {
        printf("  %d. id=%d device=%s last_active=%s\n",
               i+1, u->sessions[i].id, u->sessions[i].device, u->sessions[i].last_active);
    }
}

void change_visibility(UserSettings *u) {
    printf("Choose visibility:\n1) Public\n2) Friends-only\n3) Only me (Private)\nSelect (1-3): ");
    char line[8];
    if (!fgets(line, sizeof(line), stdin)) return;
    int choice = atoi(line);
    if (choice >=1 && choice <=3) {
        u->visibility = (Visibility)(choice-1);
        printf("Visibility set to %s\n", visibility_name(u->visibility));
    } else {
        printf("Invalid choice.\n");
    }
}

void toggle_notifications(UserSettings *u) {
    u->notifications_enabled = !u->notifications_enabled;
    printf("Notifications %s\n", u->notifications_enabled ? "enabled" : "disabled");
}

void list_apps(const UserSettings *u) {
    printf("Connected apps (%d):\n", u->app_count);
    for (int i = 0; i < u->app_count; ++i) {
        printf("  %d) %s\n", i+1, u->apps[i].name);
    }
}

void connect_app(UserSettings *u) {
    if (u->app_count >= MAX_APPS) {
        printf("Max connected apps reached.\n");
        return;
    }
    char name[MAX_NAME];
    printf("Enter name of app to connect: ");
    if (!fgets(name, sizeof(name), stdin)) return;
    name[strcspn(name, "\n")] = '\0';
    if (strlen(name) == 0) { printf("Empty name, cancelled.\n"); return; }
    /* simple duplicate check */
    for (int i = 0; i < u->app_count; ++i) if (strcmp(u->apps[i].name, name) == 0) {
        printf("App already connected.\n");
        return;
    }
    strncpy(u->apps[u->app_count].name, name, MAX_NAME-1);
    u->app_count++;
    printf("App '%s' connected.\n", name);
}

void disconnect_app(UserSettings *u) {
    list_apps(u);
    if (u->app_count == 0) return;
    printf("Enter app number to disconnect: ");
    char line[8];
    if (!fgets(line, sizeof(line), stdin)) return;
    int idx = atoi(line) - 1;
    if (idx < 0 || idx >= u->app_count) { printf("Invalid index.\n"); return; }
    printf("Disconnected '%s'\n", u->apps[idx].name);
    for (int i = idx; i < u->app_count - 1; ++i) u->apps[i] = u->apps[i+1];
    u->app_count--;
}

void list_sessions(const UserSettings *u) {
    printf("Active sessions (%d):\n", u->session_count);
    for (int i = 0; i < u->session_count; ++i) {
        printf("  %d) id=%d device=%s last_active=%s\n", i+1, u->sessions[i].id, u->sessions[i].device, u->sessions[i].last_active);
    }
}

void end_session(UserSettings *u) {
    list_sessions(u);
    if (u->session_count == 0) return;
    printf("Enter session number to end: ");
    char line[8];
    if (!fgets(line, sizeof(line), stdin)) return;
    int idx = atoi(line) - 1;
    if (idx < 0 || idx >= u->session_count) { printf("Invalid index.\n"); return; }
    printf("Ended session id=%d device=%s\n", u->sessions[idx].id, u->sessions[idx].device);
    for (int i = idx; i < u->session_count - 1; ++i) u->sessions[i] = u->sessions[i+1];
    u->session_count--;
}

/* Main interactive loop */
int main(void) {
    UserSettings user;
    memset(&user, 0, sizeof(user));

    printf("Welcome to Settings & Privacy Controls simulator.\n");
    printf("Enter your username: ");
    if (!fgets(user.username, sizeof(user.username), stdin)) return 1;
    user.username[strcspn(user.username, "\n")] = '\0';
    if (strlen(user.username) == 0) {
        printf("Username required.\n");
        return 1;
    }

    /* Try load existing settings (if username matches stored username) */
    load_settings(&user);
    if (strcmp(user.username, "") == 0) {
        strncpy(user.username, user.username, MAX_NAME-1);
    }

    /* If there were no sessions, add a couple of sample sessions for demo */
    if (user.session_count == 0) {
        add_sample_session(&user);
        add_sample_session(&user);
    }

    int running = 1;
    while (running) {
        printf("\n--- Menu ---\n");
        printf("1) Show settings\n");
        printf("2) Change profile visibility\n");
        printf("3) Toggle notifications\n");
        printf("4) Manage connected apps\n");
        printf("5) Manage active sessions\n");
        printf("6) Export my data\n");
        printf("7) Delete my account (simulate)\n");
        printf("8) Save settings\n");
        printf("9) Exit\n");
        printf("Select option: ");

        char choice[8];
        if (!fgets(choice, sizeof(choice), stdin)) break;
        int opt = atoi(choice);

        switch (opt) {
            case 1:
                show_settings(&user);
                break;
            case 2:
                change_visibility(&user);
                break;
            case 3:
                toggle_notifications(&user);
                break;
            case 4: {
                int sub = 0;
                while (1) {
                    printf("\nConnected Apps - 1) List 2) Connect 3) Disconnect 4) Back\nChoose: ");
                    if (!fgets(choice, sizeof(choice), stdin)) break;
                    sub = atoi(choice);
                    if (sub == 1) list_apps(&user);
                    else if (sub == 2) connect_app(&user);
                    else if (sub == 3) disconnect_app(&user);
                    else break;
                }
            } break;
            case 5: {
                int sub = 0;
                while (1) {
                    printf("\nSessions - 1) List 2) End session 3) Back\nChoose: ");
                    if (!fgets(choice, sizeof(choice), stdin)) break;
                    sub = atoi(choice);
                    if (sub == 1) list_sessions(&user);
                    else if (sub == 2) end_session(&user);
                    else break;
                }
            } break;
            case 6:
                export_data(&user);
                break;
            case 7:
                delete_account(&user);
                break;
            case 8:
                save_settings(&user);
                printf("Settings saved.\n");
                break;
            case 9:
                running = 0;
                save_settings(&user);
                printf("Goodbye!\n");
                break;
            default:
                printf("Invalid option.\n");
        }
    }

    return 0;
}

