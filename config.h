#include <glob.h>

// Size of buffer for status
#define BUFFERSIZE 512
// Whether or not to precisely set the status every second on the second
#define TIMEINTERVALSECONDPRECISE
// How often to set the status (only applies if TIMEINTERVALSECONDPRECISE is not defined)
#define TIMEINTERVALSECONDS 10

static glob_t glob_res;
static int glob_code;
#define MAP_FILES(glob_pattern) \
	glob_code = glob(glob_pattern, GLOB_TILDE | GLOB_BRACE | GLOB_MARK | GLOB_NOSORT, NULL, &glob_res); \
	if (glob_code != 0) { \
		if (glob_code != GLOB_NOMATCH) \
			perror("glob"); \
		glob_res.gl_pathc = 0; \
	} \
	for (int i = 0; i < glob_res.gl_pathc; ++i)
#define MAP_FILE (glob_res.gl_pathv[i])
#define MAP_FILES_COUNT (glob_res.gl_pathc)

static float readFileAsFloat(const char* file) {
	FILE* fp = fopen(file, "r");
	if (!fp) return -1;
	float val;
	fscanf(fp, "%f", &val);
	fclose(fp);
	return val;
}

static char* pfcpu(char* ptr, char* end, char* format) {
	float temp = -1.0f;
	MAP_FILES("/sys/class/thermal/thermal_zone*/temp") {
		float val = readFileAsFloat(MAP_FILE);
		if (val > temp)
			temp = val;
	}
	if (temp > 0.0f) temp /= 1.0e3f;

	float freq = -1.0f;
	MAP_FILES("/sys/devices/system/cpu/cpu*/cpufreq/scaling_cur_freq") {
		float val = readFileAsFloat(MAP_FILE);
		if (val > freq)
			freq = val;
	}
	if (freq > 0.0f) freq /= 1.0e6f;

	return ptr + snprintf(ptr, end - ptr, format, temp, freq);
}

static char* pftime(char* ptr, char* end, char* format) {
	struct tm timeinfo;
	localtime_r(&now.tv_sec, &timeinfo);
	return ptr + strftime(ptr, end - ptr, format, &timeinfo);
}

static char* ppower(char* ptr, char* end, char* format) {
	float power = 100.0f;
	MAP_FILES("/sys/class/power_supply/BAT*/capacity") {
		float val = readFileAsFloat(MAP_FILE);
		if (val < power)
			power = val;
	}
	if (MAP_FILES_COUNT == 0)
		return ptr;
	return ptr + snprintf(ptr, end - ptr, format, power);
}

static char* ptext(char* ptr, char* end, char* str) {
	if (!str) return 0;
	int l = strlen(str);
	if (ptr + l > end) {
		memcpy(ptr, str, end - ptr);
		return end;
	}
	memcpy(ptr, str, l);
	return ptr + l;
}

static char* pspace(char* ptr, char* end) {
	(void)end;
	*ptr = ' ';
	return ptr + 1;
}

#define PARTS { \
	PART(ppower, "󰁹 %2.0f") \
	PART(pspace) \
	PART(pfcpu, "%2.1f  %2.1f") \
	PART(pspace) \
	PART(pftime, " %a %Y/%m/%d  %H:%M.%S") \
}
