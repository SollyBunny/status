#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <glob.h>
#include <unistd.h>

int avgfiles(const char *glob_pattern, glob_t **glob_res) {
	if (*glob_res == NULL) {
		*glob_res = malloc(sizeof(glob_t));
		if (!glob_res) {
			perror("malloc");
			return -1;
		}
		if (glob(glob_pattern, GLOB_TILDE, NULL, *glob_res) != 0) {
			perror("glob");
			return -1;
		}
	}
	static int num;
	static int sum;
	num = 0;
	sum = 0;
	static FILE *file;
	for (size_t i = 0; i < (*glob_res)->gl_pathc; ++i) {
		if (file) {
			fclose(file);
			file = NULL;
		}
		file = fopen((*glob_res)->gl_pathv[i], "r");
		if (!file) continue;
		static int cur;
		if (fscanf(file, "%d", &cur) != 1) continue;
		++num;
		sum += cur;
	}
	if (file) {
		fclose(file);
		file = NULL;
	}
	if (num == 0) return -1;
	return sum / num;
}

char *pfcpu(char *ptr, char *end, char *format) {
	static float temp;
	static glob_t *glob_res_temp;
	temp = avgfiles("/sys/class/thermal/thermal_zone*/temp", &glob_res_temp);
	if (temp > 0) temp /= 1000;
	static float freq;
	static glob_t *glob_res_freq;
	freq = avgfiles("/sys/devices/system/cpu/cpu*/cpufreq/scaling_cur_freq", &glob_res_freq);
	if (freq > 0) freq /= 1000000;
	return ptr + snprintf(ptr, end - ptr, format, temp, freq);
}

char *pftime(char *ptr, char *end, char *format) {
	time_t rawtime;
	struct tm *timeinfo = NULL;
	if (timeinfo == NULL) timeinfo = malloc(sizeof(struct tm));
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	return ptr + strftime(ptr, end - ptr, format, timeinfo);
}

char *ppower(char *ptr, char *end, char *format) {
	static float temp;
	static glob_t *glob_res_temp;
	temp = avgfiles("/sys/class/power_supply/BAT*/capacity", &glob_res_temp);
	return ptr + snprintf(ptr, end - ptr, format, temp);
}

char *ptext(char *ptr, char *end, char *str) {
	if (!str) return 0;
	int l = strlen(str);
	if (ptr + l > end) {
		memcpy(ptr, str, end - ptr);
		return end;
	}
	memcpy(ptr, str, l);
	return ptr + l;
}

char *psleep(char *ptr, char *end, int us) {
	(void)end;
	usleep(us);
	return ptr;
}

char *pspace(char *ptr, char *end) {
	(void)end;
	*ptr = ' ';
	return ptr + 1;
}

#define PART(name, ...) ptr = (name)(ptr, end __VA_OPT__(,) __VA_ARGS__); if (ptr >= end) return end;
#define PARTS { \
	PART(psleep, 1000000) \
	PART(ppower, "󰁹 %2.0f") \
	PART(pfcpu, " %2.1f   %2.1f") \
	PART(pspace) \
	PART(pftime, " %a %Y/%m/%d  %H:%M.%S") \
}
