#include <stdio.h>
#include <unistd.h>

int mem(char *filename, int *free, int *total)
{
        int ret = 0;
        int tmp;
        FILE *file = fopen(filename, "r");
        if(!file)
                return -1;
        ret = fscanf(file, "MemTotal: %d kB\n", total);
        ret = fscanf(file, "MemFree: %d kB\n", &tmp);
        if(ret > 0)
                *free += tmp;
        ret = fscanf(file, "Buffers: %d kB\n", &tmp);
        if(ret > 0)
                *free += tmp;
        ret = fscanf(file, "Cached: %d kB\n", &tmp);
        if(ret > 0)
                *free += tmp;
        fclose(file);

        return 0;
}

int main(int argc, char **argv)
{
        FILE *file = NULL;
        int ret = 0;
        int i;
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
        
        file = fopen("/proc/stat", "r");
        if(!file)
                return -1;
        ret = fscanf(file, "cpu ") + 1;
        for(i=0; ret > 0 && i < 4; ++i) {
                ret = fscanf(file, " %d", &tmp);
                cpu_total += tmp;
                if(i == 3) cpu_idle = tmp;
        }
        fclose(file);

        mem("/proc/meminfo", &mem_free, &mem_total);

        file = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
        if(!file)
                return -1;
        ret = fscanf(file, "%d", &cpu_temp);
        fclose(file);

        file = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq", "r");
        if(!file)
                return -1;
        ret = fscanf(file, "%f", &cpu_freq);
        fclose(file);

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
        cpu_temp *= 0.001;
        cpu_freq *= 0.000001;
        bat_percent = bat_now*100.0f/bat_full;

        usleep(1000000);
        file = fopen("/proc/stat", "r");
        if(!file)
                return -1;
        ret = fscanf(file, "cpu ") + 1;
        for(i=0; ret > 0 && i < 4; ++i) {
                ret = fscanf(file, " %d", &tmp);
                cpu_total -= tmp;
                if(i == 3) cpu_idle = tmp - cpu_idle;
        }
        fclose(file);
        cpu_total = -cpu_total;
        cpu_percent = (cpu_total-cpu_idle)*100.0f/cpu_total;

        printf("mem: %.1f%% "
               "cpu: %d°C %.1f%% %.1fGHz "
               "bat: %.1f%%\n",
               mem_percent,
               cpu_temp, cpu_percent, cpu_freq,
               bat_percent);
        return 0;
}
