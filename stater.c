#include <stdio.h>
#include <unistd.h>

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

int main(int argc, char **argv)
{
        FILE *file = NULL;
        int ret = 0;
        int tmp;
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
        float bat_percent = 0.0f;
        

        proc_stat("/proc/stat", &cpu_total, &cpu_idle);
        proc_meminfo("/proc/meminfo", &mem_total, &mem_free);
        get_int("/sys/class/thermal/thermal_zone0/temp", &cpu_temp);
        get_float("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq",
                        &cpu_freq);

        file = fopen("/sys/class/power_supply/BAT0/present", "r");
        if(file) {
                tmp = 0;
                ret = fscanf(file, "%d", &tmp);
                if(tmp && ret > 0) {
                        fclose(file);
                        file = fopen("/sys/class/power_supply/BAT0/energy_now",
                                        "r");
                        if(!file)
                                return -1;
                        ret = fscanf(file, "%d", &bat_now);
                        fclose(file);

                        file = fopen("/sys/class/power_supply/BAT0/energy_full",
                                        "r");
                        if(!file)
                                return -1;
                        ret = fscanf(file, "%d", &bat_full);
                }
                fclose(file);
        }

        mem_percent = (mem_total-mem_free)*100.0f/mem_total;
        cpu_temp /= 1000;
        cpu_freq *= 0.000001;
        bat_percent = bat_now*100.0f/bat_full;

        cpu_total = -cpu_total;
        cpu_idle = -cpu_idle;
        usleep(1000000);
        proc_stat("/proc/stat", &cpu_total, &cpu_idle);
        cpu_percent = (cpu_total-cpu_idle)*100.0f/cpu_total;

        printf("mem: %.1f%% "
               "cpu: %d°C %.1f%% %.1fGHz "
               "bat: %.1f%%\n",
               mem_percent,
               cpu_temp, cpu_percent, cpu_freq,
               bat_percent);
        return 0;
}
