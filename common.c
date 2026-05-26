/* 
 * modifed: igorpauk 2017-18
 */

#include "common.h"


sigFlags_t     signals = 0;
char           __ip_buf[16];
char           __b64_enc[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";


errno_t safeRead(int fd, char *buf, int size) {
	int i;

	if (!fd || !buf) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	while (size > 0) {
		do {
			i = read(fd,buf,size);
		} while ((i == -1) && (errno == EINTR));
		if (i > 0) {
			size -= i;
			buf += i;
		} else if (i < 0) {
			if (errno == ECONNRESET) return ERR_EOF;
			WARN("read(%d) failed: %s",fd,strerror(errno));
			return ERR_SYS_ERROR;
		} else return ERR_EOF;
	}
	return OK;
}

errno_t safeWrite(int fd, char *buf, int size) {
	int i;
  
	if (!fd || !buf) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	while (size > 0) {
		do {
			i = write(fd,buf,size);
		} while ((i == -1) && (errno == EINTR));
		if (i > 0) {
			size -= i;
			buf += i;
		} else if (i < 0) {
			if (errno == ECONNRESET) return ERR_EOF;
			WARN("write(%d) failed: %s",fd,strerror(errno));
			return ERR_SYS_ERROR;
		} else return ERR_EOF;
	}
	return OK;
}

int safeReadNB(int fd, char *buf, int size_max) {
	int i, size = 0;

	if (!fd || !buf) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	while (size_max > 0) {
		do {
			i = read(fd,buf,size_max);
		} while ((i == -1) && (errno == EINTR));
		if (i > 0) {
			size_max -= i;
			buf += i;
			size += i;
		} else if (i < 0) {	// error
			break;
		} else {	// eof
			if (size > 0) break;
			return ERR_EOF;
		}
	}
	return size;
}

int safeWriteNB(int fd, char *buf, int size_max) {
	int i, size = 0;

	if (!fd || !buf) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	while (size_max > 0) {
		do {
			i = write(fd,buf,size_max);
		} while ((i == -1) && (errno == EINTR));
		if (i > 0) {
			size_max -= i;
			buf += i;
			size += i;
		} else if (i < 0) {	// error
			break;
		} else {	// eof
			if (size > 0) break;
			return ERR_EOF;
		}
	}
	return size;
}

errno_t blockSignal(int signum) {
	sigset_t ss1, ss2;
	int      i;
  
	sigemptyset(&ss1);
	sigaddset(&ss1,signum);
	do {
		i = sigprocmask(SIG_BLOCK,&ss1,&ss2);
	} while ((i == -1) && (errno == EINTR));
	if (i == -1) {
		WARN("Can't block signal %d: %s",signum,strerror(errno));
		return ERR_SYS_ERROR;
	}
	return OK;
}

errno_t unblockSignal(int signum) {
	sigset_t ss1, ss2;
	int      i;
  
	sigemptyset(&ss1);
	sigaddset(&ss1,signum);
	do {
		i = sigprocmask(SIG_UNBLOCK,&ss1,&ss2);
	} while ((i == -1) && (errno == EINTR));
	if (i == -1) {
		WARN("Can't unblock signal %d: %s",signum,strerror(errno));
		return ERR_SYS_ERROR;
	}
	return OK;
}

void sigHandler_SIGINT(int signal) {
	signals |= CSF_SIGINT;
}

void sigHandler_SIGTERM(int signal) {
	signals |= CSF_SIGTERM;
}

void sigHandler_SIGCHLD(int signal) {
	signals |= CSF_SIGCHLD;
}

void sigHandler_SIGUSR1(int signal) {
	signals |= CSF_SIGUSR1;
}

void sigHandler_SIGPIPE(int signal, siginfo_t *info, void *data) {
//	fprintf(stderr,"SIGPIPE received, fd: %d",info->si_fd);
//	close(info->si_fd);
	signals |= CSF_SIGPIPE;
}

errno_t sigHandlerInstall() {
	struct sigaction sa1, sa2;
	int              i;

	sigemptyset(&sa1.sa_mask);
	sigaddset(&sa1.sa_mask,SIGPIPE);
	sa1.sa_handler = sigHandler_SIGCHLD;
	sa1.sa_flags = SA_NOCLDSTOP;
	do { 
		i = sigaction(SIGCHLD,&sa1,&sa2);
	} while ((i == -1) && (errno == EINTR));
	if (i == -1) {
		WARN("Can't install SIGCHLD handler");
		return ERR_SYS_ERROR;
	}
	sigemptyset(&sa1.sa_mask);
	sigaddset(&sa1.sa_mask,SIGCHLD);
	sa1.sa_sigaction = sigHandler_SIGPIPE;
	sa1.sa_flags = SA_SIGINFO;
	do { 
		i = sigaction(SIGPIPE,&sa1,&sa2);
	} while ((i == -1) && (errno == EINTR));
	if (i == -1) {
		WARN("Can't install SIGPIPE handler");
		return ERR_SYS_ERROR;
	}
	sigemptyset(&sa1.sa_mask);
	sa1.sa_handler = sigHandler_SIGINT;
	sa1.sa_flags = 0;
	do { 
		i = sigaction(SIGINT,&sa1,&sa2);
	} while ((i == -1) && (errno == EINTR));
	if (i == -1) {
		WARN("Can't install SIGINT handler");
		return ERR_SYS_ERROR;
	}
	sigemptyset(&sa1.sa_mask);
	sa1.sa_handler = sigHandler_SIGTERM;
	sa1.sa_flags = 0;
	do { 
		i = sigaction(SIGTERM,&sa1,&sa2);
	} while ((i == -1) && (errno == EINTR));
	if (i == -1) {
		WARN("Can't install SIGTERM handler");
		return ERR_SYS_ERROR;
	}
	sigemptyset(&sa1.sa_mask);
	sa1.sa_handler = sigHandler_SIGUSR1;
	sa1.sa_flags = 0;
	do { 
		i = sigaction(SIGUSR1,&sa1,&sa2);
	} while ((i == -1) && (errno == EINTR));
	if (i == -1) {
		WARN("Can't install SIGUSR1 handler");
		return ERR_SYS_ERROR;
	}
	return OK;
}

void rtrimStr(char *s) {
	int i = 0;
	if (!s) return;
	i = strlen(s) - 1;
	while ((i >= 0) && (s[i] >= 0x00) && (s[i] <= 0x20)) s[i--] = 0;
}

int hexToInt(char *s) {
	if (!s) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	return strtol(s,(char **)NULL,16);
}

char *strIpAddr(unsigned long ipAddr, char *buf) {	// Converts a 32-bit address to a string. If buf=NULL, uses a static buf
	if (!buf) buf = __ip_buf;
	sprintf(buf, "%d.%d.%d.%d", (unsigned int)((ipAddr >> 24) & 0xFF), (unsigned int)((ipAddr >> 16) & 0xFF), (unsigned int)((ipAddr >> 8) & 0xFF), (unsigned int)(ipAddr & 0xFF));
	return buf;
}


int randInt(int min, int max, unsigned int *seed) {
	unsigned rnd;
	rnd = seed ? rand_r(seed): rand();
	if (max < min) max = min;
	return min + (unsigned)((rnd/(RAND_MAX + 1.0))*(max - min + 1));
}

double randDouble(double min, double max, unsigned int *seed) {
	unsigned rnd;
	rnd = seed ? rand_r(seed): rand();
	if (max < min) max = min;
	return min + (rnd/(RAND_MAX + 1.0))*(max - min);
}

bool randRoll(double p, unsigned int *seed) {
	unsigned rnd;
	rnd = seed ? rand_r(seed): rand();
	p = MIN(MAX(p,0),1);
	return rnd < p*(RAND_MAX + 1.0);
}

int b64encode(char *dst, char *src, int size) {	// base64 encoding, Sen (it-territory)
	int               len = 0;
	unsigned char     s, d0, d1, d2, d3;

	while (size > 0) {
		s = src[0];
		d0 = s >> 2;
		d1 = (s & 0x03) << 4;
		if (size > 1) {
			s = src[1];
			d1 |= s >> 4;
			d2 = (s & 0x0F) << 2;
		} else d2 = 64;
		if (size > 2) {
			s = src[2];
			d2 |= s >> 6;
			d3 = s & 0x3F;
		} else d3 = 64;
		dst[0] = __b64_enc[d0];
		dst[1] = __b64_enc[d1];
		dst[2] = __b64_enc[d2];
		dst[3] = __b64_enc[d3];
		src += 3;
		dst += 4;
		len += 4;
		size -= 3;
	}
	return len;
}

int nod2(int v1, int v2) {
	int x1, x2, x;

	x2 = MIN(abs(v1), abs(v2));
	if (x2 == 0) return 0;
	x1 = MAX(abs(v1), abs(v2));
	while (1) {
		x = x1 % x2;
		if (x==0) break;
		x1 = x2;
		x2 = x;
	}
	return x2;
}

// misc functions ===========================================================

int equalDoze(int *v) {
	int intr;
	if (*v == 0) return 60; 
	intr = 60 / nod2(*v, 60);
	intr = intr==1 ? 3 : (intr==2 ? 4 : intr);
	*v = *v * intr / 60;
	return intr;	
}

short hexToShort(char *s) {
	if (!s) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	return strtol(s, (char **)NULL, 8);
}

long hexToLong(char *s) {
	if (!s) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	return strtol(s, (char **)NULL, 32);
}
