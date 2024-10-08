
#include <glob.h>

// Size of buffer for status
#define BUFFERSIZE 512
// Whether or not to precisely set the status every second on the second
#define TIMEINTERVALSECONDPRECISE
// How often to set the status (only applies if TIMEINTERVALSECONDPRECISE is not defined)
#define TIMEINTERVALSECONDS 10

int avgfiles(const char* glob_pattern, glob_t** glob_res) {
	if (*glob_res == NULL) {
		*glob_res = malloc(sizeof(glob_t));
		if (!glob_res) {
			perror("malloc");
			return -1;
		}
		no_files:
		if (glob(glob_pattern, GLOB_TILDE | GLOB_BRACE, NULL, *glob_res) != 0) {
			perror("glob");
			return -1;
		}
	} else if ((*glob_res)->gl_pathc == 0) goto no_files; // no files invalidates the glob (eg if a battery is removed)
	if ((*glob_res)->gl_pathc == 0) return -1; // no files
	static int num;
	static int sum;
	num = 0;
	sum = 0;
	static FILE* file;
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

char* pfcpu(char* ptr, char* end, char* format) {
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

char* pftime(char* ptr, char* end, char* format) {
	static struct tm timeinfo;
	localtime_r(&now.tv_sec, &timeinfo);
	return ptr + strftime(ptr, end - ptr, format, &timeinfo);
}

char* ppower(char* ptr, char* end, char* format) {
	static float power;
	static glob_t* glob_res_temp;
	power = avgfiles("/sys/class/power_supply/BAT*/capacity", &glob_res_temp);
	if (power == -1) return ptr;
	return ptr + snprintf(ptr, end - ptr, format, power);
}

char* ptext(char* ptr, char* end, char* str) {
	if (!str) return 0;
	int l = strlen(str);
	if (ptr + l > end) {
		memcpy(ptr, str, end - ptr);
		return end;
	}
	memcpy(ptr, str, l);
	return ptr + l;
}

char* pspace(char* ptr, char* end) {
	(void)end;
	*ptr = ' ';
	return ptr + 1;
}

#define PART(name, ...) ptr = (name)(ptr, end __VA_OPT__(,) __VA_ARGS__); if (ptr >= end) return end;
#define PARTS { \
	PART(ppower, "󰁹 %2.0f ") \
	PART(pfcpu, "%2.1f  %2.1f ") \
	PART(pftime, " %a %Y/%m/%d  %H:%M.%S") \
}
