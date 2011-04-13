#include <sys/select.h>
#include <stdio.h>
#include <string.h>

int ret = 0;
int tmp = 0;

int proc_stat(char *filename, int *total, int *idle)
{
        int i;
        FILE *file = fopen(filename, "r");
        if(!file)
                return -1;
        if(fscanf(file, "cpu ") != 0) {
                fclose(file);
                return -1;
        }
        for(i=0; fscanf(file, " %d", &tmp) == 1 && i < 4; ++i) {
                *total += tmp;
                if(i == 3) *idle += tmp;
        }
        fclose(file);
        return 0;
}

int proc_meminfo(char *filename, int *total, int *free)
{
        FILE *file = fopen(filename, "r");
        if(!file) {
                *total = -1;
                *free = -1;
                return -1;
        }

        if(fscanf(file, "MemTotal: %d kB\n", total) != 1)
                *total = -1;

        *free = 0;
        if(fscanf(file, "MemFree: %d kB\n", &tmp) == 1)
                *free += tmp;
        if(fscanf(file, "Buffers: %d kB\n", &tmp) == 1)
                *free += tmp;
        if(fscanf(file, "Cached: %d kB\n", &tmp) == 1)
                *free += tmp;
        if(*free == 0)
                *free = -1;

        fclose(file);
        return (*total == -1 || *free == -1) ?-1 :0;
}

int get_int(char *filename, int *val)
{
        FILE *file = fopen(filename, "r");
        if(!file)
                return -1;
        ret = fscanf(file, "%d", val);
        fclose(file);
        if(ret == 1)
                return 0;
        else {
                *val = -1;
                return -1;
        }
}

int get_float(char *filename, float *val)
{
        FILE *file = fopen(filename, "r");
        if(!file)
                return -1;
        ret = fscanf(file, "%f", val);
        fclose(file);
        if(ret == 1)
                return 0;
        else {
                *val = -1;
                return -1;
        }
}

int get_string(char *filename, char *val, int n)
{
        FILE *file = fopen(filename, "r");
        if(!file)
                return -1;
        ret = fread(val, sizeof(*val), n, file);
        fclose(file);
        val[ret] = '\0';
        return 0;
}

char *style_null = "";
char *style_wmfs_head = "\\#FFFFFF\\";
char *style_wmfs_norm = "\\#D4D4D4\\";

int main(int argc, char **argv)
{
        int mem_total = 0;
        int mem_free = 0;
        float mem_percent = 0.0f;
        int cpu_temp = 0;
        int cpu_total = 0;
        int cpu_idle = 0;
        float cpu_percent = 0.0f;
        float cpu_freq = 0.0f;
        int bat_present = 0;
        int bat_now = 0;
        int bat_full = 0;
        int bat_rate = 0;
        char bat_status[sizeof("Discharging\n")];
        char bat_state = '~';
        int bat_hours = 0;
        int bat_minutes = 0;
        float bat_time = 0.0f;
        float bat_percent = 0.0f;
        int gpu_temp = 0;
        struct timeval tv = {1, 0};
        char *style_head;
        char *style_norm;
        if(argc > 1 && !strcmp(argv[1], "-w")) {
                style_head = style_wmfs_head;
                style_norm = style_wmfs_norm;
        } else {
                style_head = style_null;
                style_norm = style_null;
        }

        proc_stat("/proc/stat", &cpu_total, &cpu_idle);
        proc_meminfo("/proc/meminfo", &mem_total, &mem_free);
        get_int("/sys/class/thermal/thermal_zone0/temp", &cpu_temp);
        get_float("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq",
                        &cpu_freq);

        if(get_int("/sys/class/power_supply/BAT0/present",
                        &bat_present) != 0)
                bat_present = 0;
        if(bat_present) {
                get_int("/sys/class/power_supply/BAT0/energy_now",
                                &bat_now);
                get_int("/sys/class/power_supply/BAT0/energy_full",
                                &bat_full);
                get_string("/sys/class/power_supply/BAT0/status",
                                bat_status, sizeof(bat_status));
                if(!strcmp(bat_status, "Discharging\n")) {
                        get_int("/sys/class/power_supply/BAT0/power_now",
                                        &bat_rate);
                        bat_time = (float)bat_now / bat_rate;
                        bat_state = '-';
                } else if(!strcmp(bat_status, "Charging\n")) {
                        get_int("/sys/class/power_supply/BAT0/power_now",
                                        &bat_rate);
                        bat_time = (float)(bat_full - bat_now) / bat_rate;
                        bat_state = '+';
                }
        }

        get_int("/sys/devices/platform/thinkpad_hwmon/temp4_input", &gpu_temp);

        mem_percent = (mem_total-mem_free)*100.0f/mem_total;
        cpu_temp /= 1000;
        cpu_freq *= 0.000001;
        bat_percent = bat_now*100.0f/bat_full;
        bat_hours = bat_time;
        bat_minutes = (bat_time - bat_hours) * 60.0f;
        gpu_temp /= 1000;

        cpu_total = -cpu_total;
        cpu_idle = -cpu_idle;
        select(0, 0, 0, 0, &tv);
        proc_stat("/proc/stat", &cpu_total, &cpu_idle);
        cpu_percent = (cpu_total-cpu_idle)*100.0f/cpu_total;

        printf("%smem:%s %.1f%%"
               " %scpu:%s %d°C %.1f%% %.1fGHz",
               style_head, style_norm, mem_percent,
               style_head, style_norm, cpu_temp, cpu_percent, cpu_freq);
        if(bat_present) {
                printf(" %sbat:%s %.1f%%", style_head, style_norm, bat_percent);
                if(bat_state != '~' && bat_hours >= 0 && bat_minutes >= 0)
                        printf(" %c%02d:%02d", bat_state, bat_hours, bat_minutes);
        }
        printf(" %sgpu:%s %d°C\n", style_head, style_norm, gpu_temp);
        return 0;
}
