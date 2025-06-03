#include <stdio.h>
#include <dirent.h>
#include <string.h>

int main() {
    const char *dev_path = "/dev";
    struct dirent *entry;
    DIR *dp = opendir(dev_path);

    if (dp == NULL) {
        perror("opendir");
        return 1;
    }

    printf("Ports série détectés :\n");

    while ((entry = readdir(dp)) != NULL) {
        // On cherche les périphériques commençant par "ttyS", "ttyUSB" ou "ttyACM"
        if (strncmp(entry->d_name, "ttyS", 4) == 0 ||
            strncmp(entry->d_name, "ttyUSB", 6) == 0 ||
            strncmp(entry->d_name, "ttyACM", 6) == 0) {
            printf("/dev/%s\n", entry->d_name);
        }
    }

    closedir(dp);
    return 0;
}

