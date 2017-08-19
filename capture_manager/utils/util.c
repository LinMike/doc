#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include "../common.h"

char* pixfmt_string(const uint32_t fmt, char* str)
{
    str[0] =  (fmt & 0x000000ff);
    str[1] = ((fmt & 0x0000ff00) >> 8);
    str[2] = ((fmt & 0x00ff0000) >> 16);
    str[3] = ((fmt & 0xff000000) >> 24);
    str[4] = '\0';
    return str;
}

void dump_byte_array(const uint8_t* const data, const int size)
{
    int i;
    for (i = 0; i < size; i++)
    {
        if (i % 32 == 0)
        {
            printf("\n%4x  |  ", i);
        }

        printf("%02x ", data[i]);
    }
    printf("\n");
}

void dump_data_to_file(uint8_t* data, const int size, const char* filename)
{
    FILE *file = fopen(filename, "wb");
    if (file == NULL)
    {
        log_error("create file %s failed!\n", filename);
        return;
    }

    fwrite(data, 1, size, file);

    fflush(file);
    fclose(file);
}

void dump_data_to_file_autoname(uint8_t* data, int size, const char* suffix)
{
    char filename[64];
    get_local_time_str(filename, suffix);
    FILE *file = fopen(filename, "wb");
    if (file == NULL)
    {
        log_error("create file %s failed!\n", filename);
        return;
    }

    fwrite(data, 1, size, file);

    fflush(file);
    fclose(file);

    log_debug("dumpe to %s success!\n", filename);
}


long get_time_ms()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

void get_local_time_str(char* buf, const char* suffix)
{
    time_t my_time;
    struct tm * timeinfo;
    time (&my_time);
    timeinfo = localtime (&my_time);

    sprintf(buf, "%04d%02d%02d_%02d%02d%02d%s",
        timeinfo->tm_year + 1900,
        timeinfo->tm_mon + 1,
        timeinfo->tm_mday,
        timeinfo->tm_hour,
        timeinfo->tm_min,
        timeinfo->tm_sec,
        (suffix ? suffix : "")
    );
}

#define MOTOR_MAGIC 'L'
#define BUZZER_ON 		_IOW(MOTOR_MAGIC, 0,int)
#define BUZZER_OFF 		_IOW(MOTOR_MAGIC, 1,int)
#define IOCTL_BZR_BEEP		_IOW('Q', 0x31, int)

static int g_alarm_duration;
static long g_alarm_time_start;
static int g_alarm_enable = 0;

static int set_alarm_state(int on)
{
    unsigned int cmd = (on ? BUZZER_ON : BUZZER_OFF);
    int fd = open("/dev/qiyang_buzzer", O_RDWR);
    if (fd < 0)
    {
        log_error("open device buzzer failed!\n");
        return -1;
    }

    int retval = ioctl(fd, cmd, cmd);
    if (retval == -1)
    {
        log_error("buzzer ioctl on error\n");
        return -1;
    }

    close(fd);
    return 0;
}

void alarm_ctrl()
{
    if (!g_alarm_enable)
        return;

    long tcur = get_time_ms();
    if ((tcur - g_alarm_time_start) >= g_alarm_duration)
    {
        set_alarm_state(0);
        g_alarm_enable = 0;
    }
}

int alarm_on(int duration)
{
    g_alarm_duration = (duration < 0 ? 0 : duration);
    g_alarm_time_start = get_time_ms();
    g_alarm_enable = 1;
    set_alarm_state(1);
    return 0;
}


