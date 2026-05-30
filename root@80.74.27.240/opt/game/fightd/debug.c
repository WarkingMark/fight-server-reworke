/* 
 * modifed: igorpauk 2017-18
 */

#include "debug.h"


pid_t          debugPid;
pthread_t      debugThid;
debugLevel_t   debugLevel = DL_SILENT;
FILE           *debugFile = NULL;
int            debugStrNum = 0;

char           *debugLevelStr[] = {
	"SILENT",
	"MSG   ",
	"FATAL ",
	"WARN  ",
	"INFO  ",
	"DEBUG ",
	"DEBUGXXX"
};


int debugInit(debugLevel_t level, const char *fn) {
	debugPid = getpid();
	debugThid = pthread_self();
	debugLevel = level;
	if (!fn) return 0;
	debugFile = fopen(fn,"a+");
	if (!debugFile) {
		WARN("Can't open %s",fn);
		return -1;
	}
	return 0;
}

void debugDone(void) {
	debugFlush();
	debugLevel = DL_SILENT;
	if (!debugFile) return;
	fclose(debugFile);
}

void debugMsg(debugLevel_t level, char *file, int line, const char *func, char *msg, ...) {
	va_list      ap;
	time_t       stime;
	pthread_t    thid;
	char         *buf;
	int          bufPos, m, i;

	if (level > debugLevel) return;
	buf = (char *)malloc(DEBUG_BUF_MAX);
	if (!buf) return;
	va_start(ap,msg);
	stime = time(NULL);
	thid = pthread_self();
	bufPos = strftime(buf, DEBUG_BUF_MAX - 7, "%Y-%m-%d %H:%M:%S", localtime(&stime));
	bufPos += sprintf(buf + bufPos, "%8d  %4x  %s %s:%d(%s)\t", debugPid, (unsigned int)(thid != debugThid ? ((thid >> 12) & 0xFFFF): 0), debugLevelStr[level], file, line, func);
	m = DEBUG_BUF_MAX - bufPos - 7;
	i = vsnprintf(buf + bufPos, m, msg, ap);
	if (i >= m) {	// string too long
		bufPos += m - 1;
		bufPos += sprintf(buf + bufPos, "[...]");
	} else bufPos += i;
	debugStrNum++;
	if (debugFile) {
		bufPos += sprintf(buf + bufPos, "\n");
		fwrite(buf,bufPos,1,debugFile);
		if (debugStrNum >= DEBUG_FLUSH_SIZE) {
			fflush(debugFile);
			debugStrNum = 0;
		}
	} else {
		*(buf + bufPos) = 0;
		puts(buf);
	}
	va_end(ap);
	free(buf);
}

void debugFlush(void) {
	if (!debugFile) return;
	fflush(debugFile);
	debugStrNum = 0;
}
