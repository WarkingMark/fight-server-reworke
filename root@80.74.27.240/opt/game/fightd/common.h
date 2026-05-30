/* 
 * modifed: igorpauk 2017-18
 */

#ifndef __COMMON_H__
#define __COMMON_H__


#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>


#define false	0
#define true	1

#define True true
#define False false

#define MIN(A, B) ((A) < (B) ? (A): (B))
#define MAX(A, B) ((A) > (B) ? (A): (B))


typedef int bool;
typedef enum errorCode_e errno_t;
typedef enum sigFlags_e sigFlags_t;


enum errorCode_e {
	OK                =    0,
	ERR_WRONG_ARGS    =   -1,
	ERR_INIT          =   -2,
	ERR_CONFIG        =   -3,
	ERR_NO_MEM        =   -4, 
	ERR_SYS_ERROR     =   -5, 
	ERR_IO_ERROR      =   -6,
	ERR_WRONG_STATE   =   -7,
	ERR_WRONG_DATA    =   -8,
	ERR_EOF           =   -9,
	ERR_GENERAL       = -100
};

enum sigFlags_e {
	CSF_CLEAN    = 0x00,
	CSF_SIGPIPE  = 0x01,
	CSF_SIGINT   = 0x02,
	CSF_SIGTERM  = 0x04,
	CSF_SIGCHLD  = 0x08,
	CSF_SIGUSR1  = 0x10,
};


extern sigFlags_t   signals;


errno_t safeRead(int fd, char *buf, int size);
errno_t safeWrite(int fd, char *buf, int size);
int safeReadNB(int fd, char *buf, int size_max);
int safeWriteNB(int fd, char *buf, int size_max);

errno_t blockSignal(int signum);
errno_t unblockSignal(int signum);
errno_t sigHandlerInstall();

void rtrimStr(char *s);
int hexToInt(char *s);
char *strIpAddr(unsigned long ipAddr, char *buf);
int randInt(int min, int max, unsigned int *seed);
double randDouble(double min, double max, unsigned int *seed);
bool randRoll(double p, unsigned int *seed);

int b64encode(char *dst, char *src, int size);
int nod2(int v1, int v2);
int equalDoze(int *v);

double round(double);	// gcc 4.4.1 warning fix

short hexToShort(char *s);
long hexToLong(char *s);

#include "vec.h"
#include "debug.h"


#endif
