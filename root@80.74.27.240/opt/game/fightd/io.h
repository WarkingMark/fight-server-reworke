/* 
 * modifed: igorpauk 2017-18
 */

#ifndef __IO_H__
#define __IO_H__

#include "typedefs.h"


#define MAX_INPACKET_SIZE     4096
#define MAX_OUTPACKET_SIZE    32768


enum fs_paramType_e {
	PT_INVALID   = 0,
	PT_INT       = 1,
	PT_FIXED     = 2,
	PT_STRING    = 3,
	PT_RAW       = 4,
	PT_NINT      = 5,
	PT_NFIXED    = 6,
	PT_SHORTINT  = 7,
	PT_NSHORTINT = 8,
	PT_BIGINT    = 9,
	PT_NBIGINT   = 10
};


struct fs_packet_s {
	int          size, flags;
	vector_t     params;
};

struct fs_param_s {
	unsigned int      id;
	fs_paramType_t    type;
	union {
		int       i;		// PT_INT, PT_NINT
		double    d;		// PT_FIXED, PT_NFIXED
		short     h;        // PT_SHORTINT, PT_NSHORTINT
		long      l;        // PT_BIGINT, PT_NBIGINT
		char      *ptr;	// PT_STRING, PT_RAW
	}                 val;
	int               size;
};


#define PARAM_ADD(A, FMT, ...) fs_addParams(&(((fs_packet_t *)(A))->params), FMT, ## __VA_ARGS__)
#define PARAM_NEW(A) { A = malloc(sizeof(fs_param_t)); memset(A,0,sizeof(fs_param_t)); }
#define PARAM_PUSH(A, B) v_push(&(((fs_packet_t *)(A))->params),(B))
#define PARAM_POP(A) v_pop(&(((fs_packet_t *)(A))->params))
#define PARAM_COUNT(A) v_size(&(((fs_packet_t *)(A))->params))
#define PARAM_RESET(A) v_reset(&(((fs_packet_t *)(A))->params),0)
#define PARAM_CURRENT(A) v_current(&(((fs_packet_t *)(A))->params),0)
#define PARAM_NEXT(A) v_next(&(((fs_packet_t *)(A))->params),0)
#define PARAM_EACH(A) v_each(&(((fs_packet_t *)(A))->params),0)
#define PARAM_IDX(A) v_idx(&(((fs_packet_t *)(A))->params),0)

#define PARAM_ID(A) (((fs_param_t *)(A))->id)
#define PARAM_TYPE(A) (((fs_param_t *)(A))->type)
#define PARAM_SIZE(A) (((fs_param_t *)(A))->size)

#define PARAM_INT(A) (((fs_param_t *)(A))->val.i)
#define PARAM_BIGINT(A) (((fs_param_t *)(A))->val.i)
#define PARAM_SETINT(A, B) { PARAM_TYPE(A) = PT_INT; PARAM_INT(A) = B; }
#define PARAM_SORT(A) (((fs_param_t *)(A))->val.i)
#define PARAM_SETSORT(A, B) { PARAM_TYPE(A) = PT_SHORTINT; PARAM_SORT(A) = B; }
#define PARAM_LONG(A) (((fs_param_t *)(A))->val.l)
#define PARAM_SETLONG(A, B) { PARAM_TYPE(A) = PT_BIGINT; PARAM_LONG(A) = B; }
#define PARAM_FIXED(A) (((fs_param_t *)(A))->val.d)
#define PARAM_SETFIXED(A, B) { PARAM_TYPE(A) = PT_FIXED; PARAM_FIXED(A) = B; }
#define PARAM_STRING(A) (((fs_param_t *)(A))->val.ptr)
#define PARAM_SETSTRING(A, B) { PARAM_TYPE(A) = PT_STRING; PARAM_STRING(A) = B; }
#define PARAM_RAW(A) (((fs_param_t *)(A))->val.ptr)
#define PARAM_SETRAW(A, B, C) { PARAM_TYPE(A) = PT_RAW; PARAM_RAW(A) = B; PARAM_SIZE(A) = C; }


extern int     fs_ioMaxInPacketSize;
extern int     fs_ioMaxOutPacketSize;


fs_packet_t *fs_packetCreate(void);
errno_t fs_packetDelete(fs_packet_t *packet, bool freeParams);
fs_packet_t *fs_packetCopy(fs_packet_t *packet);
errno_t fs_packetRead(fs_packet_t *packet, int fd);
errno_t fs_packetWrite(fs_packet_t *packet, int fd);

int fs_getParamBufSize(vector_t *params);
int fs_packParams(vector_t *params, char *buf, int bufSize);
errno_t fs_unpackParams(vector_t *params, char *data, int size);
errno_t fs_addParamsVA(vector_t *params, char *fmt, va_list ap);
errno_t fs_addParams(vector_t *params, char *fmt, ...);
vector_t *fs_copyParams(vector_t *src, vector_t *dst);
errno_t fs_freeParams(vector_t *params);
void fs_debugParams(vector_t *params, char *prefix, bool shortForm);


#endif
