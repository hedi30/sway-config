#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

void run_cmd(const char *cmd, char *result, size_t maxlen) {
    FILE *fp = popen(cmd, "r");
    if (fp) {
        fgets(result, maxlen, fp);
        result[strcspn(result, "\n")] = 0;
        pclose(fp);
    } else {
        strncpy(result, "?", maxlen);
    }
}

void get_volume(char *result, size_t maxlen) {
    run_cmd("amixer get Master | grep -o '[0-9]*%' | head -1 | tr -d '%'", result, maxlen);
}

void get_brightness(char *result, size_t maxlen) {
    char current[16], max[16];
    run_cmd("brightnessctl get", current, sizeof(current));
    run_cmd("brightnessctl max", max, sizeof(max));

    int cur = atoi(current);
    int maxv = atoi(max);
    if (maxv > 0) {
        snprintf(result, maxlen, "%d", cur * 100 / maxv);
    } else {
        strncpy(result, "?", maxlen);
    }
}


void get_battery(char *result, size_t maxlen) {
    FILE *fp = popen("upower --enumerate | grep 'BAT'", "r");
    if (!fp) {
        strncpy(result, "?", maxlen);
        return;
    }

    char line[128], battery1[8] = "?", battery2[8] = "?";
    int count = 0;

    while (fgets(line, sizeof(line), fp) && count < 2) {
        line[strcspn(line, "\n")] = 0; 
        char cmd[256], percentage[8] = "?";
        snprintf(cmd, sizeof(cmd),
                 "upower -i %s | grep percentage | awk '{print $2}' | tr -d '%%'", line);
        run_cmd(cmd, percentage, sizeof(percentage));
        if (count == 0) strncpy(battery1, percentage, sizeof(battery1));
        else if (count == 1) strncpy(battery2, percentage, sizeof(battery2));
        count++;
    }
    pclose(fp);

    if (count == 1) snprintf(result, maxlen, "%s%%", battery1);
    else if (count == 2) snprintf(result, maxlen, " %s%%  %s%%", battery1, battery2);
    else strncpy(result, "?", maxlen);
}

void get_headset(char *result, size_t maxlen) {
    char path[128];
    run_cmd("upower --enumerate | grep 'headset' | head -n 1", path, sizeof(path));
    path[strcspn(path, "\n")] = 0; 

    if (strlen(path) == 0) {
        result[0] = '\0';
        return;
    }

    char cmd[256], percentage[8] = "?";
    snprintf(cmd, sizeof(cmd),
             "upower -i %s | grep percentage | awk '{print $2}' | tr -d '%%'", path);
    run_cmd(cmd, percentage, sizeof(percentage));
    snprintf(result, maxlen, "🎧 %s%%", percentage);
}

void get_datetime(char *date_str, size_t maxlen) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(date_str, maxlen, "%Y/%m/%d %H:%M", tm_info);
}

int main() {
    while (1) {
        char volume[8], brightness[8], battery[64], headset[32], datetime[32];
        get_volume(volume, sizeof(volume));
        get_brightness(brightness, sizeof(brightness));
        get_battery(battery, sizeof(battery));
        get_headset(headset, sizeof(headset));
        get_datetime(datetime, sizeof(datetime));

        printf("|  %s%% |  %s%% | %s | %s | %s\n",
               volume, brightness, battery,
               headset[0] ? headset : "", datetime);

        fflush(stdout);
//        usleep(500000);
        sleep(1);
    }

    return 0;
}

