/* 
 * modifed: igorpauk 2017-18
 */

#include "common.h"
#include "io.h"


int       fs_ioMaxInPacketSize = 4096;
int       fs_ioMaxOutPacketSize = 32768;


// ========================================= packet ========================================= //

fs_packet_t *fs_packetCreate(void) {
	fs_packet_t *packet;

	packet = malloc(sizeof(fs_packet_t));
	if (!packet) {
		WARN("malloc() failed");
		return NULL;
	}
	memset(packet,0,sizeof(fs_packet_t));
	v_init(&(packet->params));
	return packet;
}

errno_t fs_packetDelete(fs_packet_t *packet, bool freeParams) {
	if (!packet) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	if (freeParams)	fs_freeParams(&(packet->params));
	else v_zero(&(packet->params));
	free(packet);
	return OK;
}

fs_packet_t *fs_packetCopy(fs_packet_t *packet) {
	fs_packet_t *packetCopy;
	if (!packet) {
		WARN("Invalid arguments");
		return NULL;
	}
	packetCopy = fs_packetCreate();
	if (!packetCopy) return NULL;
	fs_copyParams(&(packet->params),&(packetCopy->params));
	return packetCopy;
}

errno_t fs_packetRead(fs_packet_t *packet, int fd) {
	errno_t      io_status;
	char         *io_buf, tbuf[10];
	int          size;

	if (!packet || !fd) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	memset(tbuf,0,sizeof(tbuf));
	io_status = safeRead(fd,tbuf,4);
	if (io_status != OK) {
		if (io_status != ERR_EOF) WARN("Error reading packet size, status: %d",io_status);
		return io_status;
	}
	size = hexToInt(tbuf);
	if ((size <= 0) || (size > fs_ioMaxInPacketSize)) {
		WARN("Invalid incoming packet size (size: %d, max size: %d)",size,fs_ioMaxInPacketSize);
		return ERR_WRONG_DATA;
	}
	io_buf = malloc(size+1);
	if (!io_buf) {
		WARN("malloc() failed");
		return ERR_NO_MEM;
	}
	io_status = safeRead(fd,io_buf,size+1);	// Plus extra \0 at the end since we read C-strings
//	DEBUG("DATA IN: %s",io_buf);
//	DEBUG("INCOMING PACKET SIZE: %d bytes",size);
	if (io_status == OK) {
		*(io_buf + size) = 0;
		if (strlen(io_buf) == size) {	// We can check again, God damn it!
			fs_freeParams(&(packet->params));
			io_status = fs_unpackParams(&(packet->params),io_buf,size);
			if (io_status == OK) packet->size = size;
		} else {
			WARN("Packet size mismatch (size: %d, data: %s)",size,io_buf);
			io_status = ERR_WRONG_DATA;
		}
	} else if (io_status != ERR_EOF) WARN("Error reading packet data, status: %d",io_status);
	free(io_buf);
	return io_status;
}

errno_t fs_packetWrite(fs_packet_t *packet, int fd) {
	errno_t      io_status;
	char         *io_buf, tbuf[10];
	int          buf_size, size;

	if (!packet || !fd) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	buf_size = fs_getParamBufSize(&(packet->params));
	if ((buf_size <= 0) || (buf_size > fs_ioMaxOutPacketSize)) {
		WARN("Invalid outgoing packet size (size: %d, max size: %d)",buf_size,fs_ioMaxOutPacketSize);
		return ERR_WRONG_DATA;
	}
	io_buf = malloc(buf_size+5);
	if (!io_buf) {
		WARN("malloc() failed");
		return ERR_NO_MEM;
	}
	size = fs_packParams(&(packet->params),io_buf+4,buf_size);
	if (size == buf_size) {
		sprintf(tbuf,"%04x",size);
		memcpy(io_buf,tbuf,4);
		size += 4;
		*(io_buf + size) = 0;
//		DEBUG("DATA OUT: %s",io_buf+4);
//		DEBUG("OUTGOING PACKET SIZE: %d bytes",buf_size);
		io_status = safeWrite(fd,io_buf,size+1);	// Plus extra \0 at the end since we write C-strings
		if (io_status == OK) packet->size = size;
		else WARN("Error writing packet data, status: %d",io_status);
	} else {
		WARN("Packet size mismatch (size: %d, buf_size: %d)",size,buf_size);
		io_status = ERR_WRONG_DATA;
	}
	free(io_buf);
	return io_status;
}


// ========================================= packet params ========================================= //

int fs_getParamBufSize(vector_t *params) {
	int          size = 0;
	fs_param_t   *param;
	viter_t      vi;

	if (!params) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	v_reset(params,&vi);
	while ((param = v_each(params,&vi))) {
		size += 4;
		switch (param->type) {
			case PT_INT:
			case PT_NINT:
			case PT_FIXED:
			case PT_NFIXED:
				size += 8;
				break;
			case PT_SHORTINT:
			case PT_NSHORTINT:
				size += 4;
				break;
			case PT_BIGINT:
			case PT_NBIGINT:
				size += 16;
				break;
			case PT_STRING:
				param->size = strlen(param->val.ptr);
				size += 4 + param->size;
				break;
			case PT_RAW:
				size += 4 + param->size;
				break;
			default:
				WARN("Invalid param type: %d",param->type);
				return ERR_WRONG_DATA;
		}
	}
	return size;
}

int fs_packParams(vector_t *params, char *buf, int bufSize) {
	int          size = bufSize;
	fs_param_t   *param;
	viter_t      vi;

	if (!params || !buf) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	v_reset(params,&vi);
	while ((param = v_each(params,&vi))) {
		if ((PARAM_TYPE(param) == PT_INT) && (PARAM_INT(param) < 0)) PARAM_TYPE(param) = PT_NINT;
		if ((PARAM_TYPE(param) == PT_FIXED) && (PARAM_FIXED(param) < 0)) PARAM_TYPE(param) = PT_NFIXED;
		if ((PARAM_TYPE(param) == PT_BIGINT) && (PARAM_INT(param) < 0)) PARAM_TYPE(param) = PT_NBIGINT;
		if ((PARAM_TYPE(param) == PT_SHORTINT) && (PARAM_FIXED(param) < 0)) PARAM_TYPE(param) = PT_NSHORTINT;
		sprintf(buf,"%04x",(param->id << 8) + (param->type & 0xFF));
		buf += 4;
		size -= 4;
		switch (param->type) {
			case PT_INT:
			case PT_NINT:
				if (size < 8) {
					WARN("Insufficient buffer space (left: %d, need: %d)",size,8);
					return ERR_WRONG_DATA;
				}
				sprintf(buf,"%08x",(param->type == PT_NINT ? -param->val.i: param->val.i));
				buf += 8;
				size -= 8;
				break;
			case PT_SHORTINT:
			case PT_NSHORTINT:
				if (size < 4) {
					WARN("Insufficient buffer space (left: %d, need: %d)", size, 4);
					return ERR_WRONG_DATA;
				}
				sprintf(buf, "%04x", (param->type == PT_NSHORTINT ? -param->val.i : param->val.i));
				buf += 4;
				size -= 4;
				break;
			case PT_BIGINT:
			case PT_NBIGINT:
				if (size < 16) {
					WARN("Insufficient buffer space (left: %d, need: %d)", size, 16);
					return ERR_WRONG_DATA;
				}
				sprintf(buf, "%016x", (param->type == PT_NBIGINT ? -param->val.i : param->val.i));
				buf += 16;
				size -= 16;
				break;
			case PT_FIXED:
			case PT_NFIXED:
				if (size < 8) {
					WARN("Insufficient buffer space (left: %d, need: %d)",size,8);
					return ERR_WRONG_DATA;
				}
				sprintf(buf,"%08x",(int)((param->type == PT_NFIXED ? -param->val.d: param->val.d) * 10000));
				buf += 8;
				size -= 8;
				break;
			case PT_STRING:
			case PT_RAW:
				if (param->type == PT_STRING) param->size = strlen(param->val.ptr);
				if (size < (param->size + 4)) {
					WARN("Insufficient buffer space (left: %d, need: %d)",size,(param->size + 4));
					return ERR_WRONG_DATA;
				}
				sprintf(buf,"%04x",param->size);
				memcpy(buf + 4, param->val.ptr, param->size);
				buf += param->size + 4;
				size -= param->size + 4;
				break;
			default:
				WARN("Invalid param type: %d",param->type);
				return ERR_WRONG_DATA;
		}
	}
	return (bufSize - size);
}

errno_t fs_unpackParams(vector_t *params, char *data, int size) {
	int          i;
	char         tbuf[18], *tdata = data;
	fs_param_t   *param;

	if (!params || !data || (size < 0)) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	while (size > 0) {
		if (size < 4) {
			WARN("Insufficient data for reading type (left: %d, offset: %d, data: %s)",size,(tdata-data),data);
			return ERR_WRONG_DATA;
		}
		param = malloc(sizeof(fs_param_t));
		memset(tbuf,0,sizeof(tbuf));
		memcpy(tbuf,tdata,4);
		i = hexToInt(tbuf);	
		param->id = (i >> 8);		// got 'id' field
		param->type = i & 0xFF;	// got 'type' field
		tdata += 4;
		size -= 4;
		switch (param->type) {
			case PT_INT:
			case PT_NINT:
				if (size < 8) {
					WARN("Insufficient data for reading value (left: %d, offset: %d, data: %s)", size, (tdata - data), data);
					free(param);
					return ERR_WRONG_DATA;
				}
				memset(tbuf, 0, sizeof(tbuf));
				memcpy(tbuf, tdata, 8);
				param->val.i = hexToInt(tbuf);
				if (param->type == PT_NINT) param->val.i = -param->val.i;
				tdata += 8;
				size -= 8;
				break;
			case PT_SHORTINT:
			case PT_NSHORTINT:
				if (size < 4) {
					WARN("Insufficient data for reading value (left: %d, offset: %d, data: %s)", size, (tdata - data), data);
					free(param);
					return ERR_WRONG_DATA;
				}
				memset(tbuf, 0, sizeof(tbuf));
				memcpy(tbuf, tdata, 4);
				param->val.i = hexToShort(tbuf);
				if (param->type == PT_NSHORTINT) param->val.i = -param->val.i;
				tdata += 4;
				size -= 4;
				break;
			case PT_BIGINT:
			case PT_NBIGINT:
				if (size < 16) {
					WARN("Insufficient data for reading value (left: %d, offset: %d, data: %s)", size, (tdata - data), data);
					free(param);
					return ERR_WRONG_DATA;
				}
				memset(tbuf, 0, sizeof(tbuf));
				memcpy(tbuf, tdata, 16);
				param->val.i = hexToInt(tbuf);
				if (param->type == PT_NBIGINT) param->val.i = -param->val.i;
				tdata += 16;
				size -= 16;
				break;
			case PT_FIXED:
			case PT_NFIXED:
				if (size < 8) {
					WARN("Insufficient data for reading value (left: %d, offset: %d, data: %s)",size,(tdata-data),data);
					free(param);
					return ERR_WRONG_DATA;
				}
				memset(tbuf,0,sizeof(tbuf));
				memcpy(tbuf,tdata,8);
				param->val.d = (double)hexToInt(tbuf) / 10000;
				if (param->type == PT_NFIXED) param->val.d = -param->val.d;
				tdata += 8;
				size -= 8;
				break;
			case PT_STRING:
			case PT_RAW:
				if (size < 4) {
					WARN("Insufficient data for reading size (left: %d, offset: %d, data: %s)",size,(tdata-data),data);
					free(param);
					return ERR_WRONG_DATA;
				}
				memset(tbuf,0,sizeof(tbuf));
				memcpy(tbuf,tdata,4);
				param->size = hexToInt(tbuf);
				tdata += 4;
				size -= 4;
				if (param->size > size) {
					WARN("Invalid length: %d (offset: %d, data: %s)",param->size,(tdata-data-4),data);
					free(param);
					return ERR_WRONG_DATA;
				}
				param->val.ptr = malloc(param->size + 1);
				memcpy(param->val.ptr,tdata,param->size);
				*(param->val.ptr + param->size) = 0;
				tdata += param->size;
				size -= param->size;
				break;
			default:
				WARN("Invalid param type: %d (offset: %d, data: %s)",param->type,(tdata-data-4),data);
				free(param);
				return ERR_WRONG_DATA;
		}
		v_push(params,param);
	}
	return OK;
}

// fmt: "i" - PT_INT, "f" - PT_FIXED, "s" - PT_STRING
errno_t fs_addParamsVA(vector_t *params, char *fmt, va_list ap) {
	fs_param_t   *param;
	char         *s;

	if (!params || !fmt) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	for ( ; *fmt; fmt++ ) {
		PARAM_NEW(param);
		switch(*fmt) {
			case 'i':
			case 'I':
				PARAM_SETINT(param,va_arg(ap, int));
				break;
			case 'h':
			case 'H':
				PARAM_SETSORT(param, va_arg(ap, int));
				break;
			case 'l':
			case 'L':
				PARAM_SETLONG(param, va_arg(ap, int));
				break;
			case 'f':
			case 'F':
				PARAM_SETFIXED(param,va_arg(ap, double));
				break;
			case 's':
			case 'S':
				s = va_arg(ap, char *);
				if (!s) s = "";
				PARAM_SETSTRING(param,strdup(s));
				break;
			default:
				WARN("Wrong type specifier: '%c'",*fmt);
				free(param);
				return ERR_WRONG_ARGS;
		}
		v_push(params,param);
	}
	return OK;
}

errno_t fs_addParams(vector_t *params, char *fmt, ...) {
	va_list      ap;
	errno_t      status;

	va_start(ap,fmt);
	status = fs_addParamsVA(params,fmt,ap);
	va_end(ap);
	return status;
}

vector_t *fs_copyParams(vector_t *src, vector_t *dst) {
	fs_param_t   *srcParam, *dstParam;
	viter_t      vi;

	if (!src || !dst) {
		WARN("Invalid arguments");
		return NULL;
	}
	fs_freeParams(dst);
	v_reset(src,&vi);
	while ((srcParam = v_each(src,&vi))) {
		dstParam = malloc(sizeof(fs_param_t));
		memcpy(dstParam,srcParam,sizeof(fs_param_t));
		if ((srcParam->type == PT_STRING) || (srcParam->type == PT_RAW)) dstParam->val.ptr = strdup(srcParam->val.ptr);
		v_push(dst,dstParam);
	}
	return dst;
}

errno_t fs_freeParams(vector_t *params) {
	fs_param_t   *param;
	viter_t      vi;

	if (!params) {
		WARN("Invalid arguments");
		return ERR_WRONG_ARGS;
	}
	if (!v_size(params)) return OK;
	v_reset(params,&vi);
	while ((param = v_each(params,&vi))) {
		if ((param->type == PT_STRING) || (param->type == PT_RAW)) free(param->val.ptr);
		free(param);
	}
	v_zero(params);
	return OK;
}

void fs_debugParams(vector_t *params, char *prefix, bool shortForm) {
	fs_param_t   *param;
	viter_t      vi;
	int          n = 0;
	char         *buf;
	int          m, i, bufPos = 0;

	if (!params) {
		WARN("Invalid arguments");
		return;
	}

	if (shortForm) {
		buf = (char *)malloc(DEBUG_BUF_MAX);
		if (!buf) {
			WARN("malloc() failed");
			return;
		}
		bufPos = sprintf(buf,"%s[%d]",(prefix ? prefix: ""),v_size(params));
		v_reset(params,&vi);
		while ((param = v_each(params,&vi))) {
			m = DEBUG_BUF_MAX - bufPos - 6;
			switch (PARAM_TYPE(param)) {
				case PT_INT:
				case PT_NINT:
					i = snprintf(buf + bufPos, m, ", %d", PARAM_INT(param));
					break;
				case PT_SHORTINT:
				case PT_NSHORTINT:
					i = snprintf(buf + bufPos, m, ", %d", PARAM_SORT(param));
					break;
				case PT_BIGINT:
				case PT_NBIGINT:
					i = snprintf(buf + bufPos, m, ", %d", PARAM_INT(param));
					break;
				case PT_FIXED:
				case PT_NFIXED:
					i = snprintf(buf + bufPos, m, ", %.2f", PARAM_FIXED(param));
					break;
				case PT_STRING:
					i = snprintf(buf + bufPos, m, ", \"%s\"", PARAM_STRING(param));
					break;
				case PT_RAW:
					i = snprintf(buf + bufPos, m, ", RAW(%d bytes)", PARAM_SIZE(param));
					break;
				default:
					i = snprintf(buf + bufPos, m, ", UNKNOWN TYPE(%d)", PARAM_TYPE(param));
					break;
			}
			if (i >= m) {
				bufPos += m - 1;
				bufPos += sprintf(buf + bufPos, "[...]");
				break;
			} else bufPos += i;
		}
		DEBUG("%s",buf);
		free(buf);
		return;
	}

	DEBUG("%s--------------------------------",(prefix ? prefix: ""));
	DEBUG("SIZE (%d param(s))",v_size(params));
	v_reset(params,&vi);
	while ((param = v_each(params,&vi))) {
		switch (PARAM_TYPE(param)) {
			case PT_INT:
				DEBUG("#%-3d id=%-3d INT (%d)",n,PARAM_ID(param),PARAM_INT(param));
				break;
			case PT_NINT:
				DEBUG("#%-3d id=%-3d NINT (%d)",n,PARAM_ID(param),PARAM_INT(param));
				break;
			case PT_SHORTINT:
				DEBUG("#%-3d id=%-3d SORT (%d)", n, PARAM_ID(param), PARAM_SORT(param));
				break;
			case PT_NSHORTINT:
				DEBUG("#%-3d id=%-3d NSORT (%d)", n, PARAM_ID(param), PARAM_SORT(param));
				break;
			case PT_BIGINT:
				DEBUG("#%-3d id=%-3d BIG (%d)", n, PARAM_ID(param), PARAM_INT(param));
				break;
			case PT_NBIGINT:
				DEBUG("#%-3d id=%-3d NBIG (%d)", n, PARAM_ID(param), PARAM_INT(param));
				break;
			case PT_FIXED:
				DEBUG("#%-3d id=%-3d FIXED (%.2f)",n,PARAM_ID(param),PARAM_FIXED(param));
				break;
			case PT_NFIXED:
				DEBUG("#%-3d id=%-3d NFIXED (%.2f)",n,PARAM_ID(param),PARAM_FIXED(param));
				break;
			case PT_STRING:
				DEBUG("#%-3d id=%-3d STRING (\"%s\")",n,PARAM_ID(param),PARAM_STRING(param));
				break;
			case PT_RAW:
				DEBUG("#%-3d id=%-3d RAW (%d bytes)",n,PARAM_ID(param),PARAM_SIZE(param));
				break;
			default:
				DEBUG("#%-3d id=%-3d UNKNOWN TYPE (%d)",n,PARAM_ID(param),PARAM_TYPE(param));
				break;
		}
		n++;
	}
	DEBUG("--------------------------------");
}
