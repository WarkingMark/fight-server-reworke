/* 
 * modifed: igorpauk 2017-18
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vec.h"


vector_t *v_init(vector_t *vec) {
	if (!vec) vec = malloc(sizeof(vector_t));
	if (!vec) return NULL;
	memset(vec,0,sizeof(vector_t));
	return vec;
}

vector_t *v_copy(vector_t *vec) {
	vector_t *next = vec, *vecCopy = NULL, *nextCopy, *prevCopy;
	int      i = 0;

	if (!vec) return NULL;
	do {
		nextCopy = malloc(sizeof(vector_t));
		memcpy(nextCopy,next,sizeof(vector_t));
		if (!vecCopy) vecCopy = prevCopy = nextCopy;
		else prevCopy->chunk[VECTOR_CHUNK_SIZE] = nextCopy;
		i += VECTOR_CHUNK_SIZE;
		if (i >= vec->size) break;
		prevCopy = nextCopy;
		next = next->chunk[VECTOR_CHUNK_SIZE];
	} while (1);
	return vecCopy;
}

int v_zero(vector_t *vec) {
	vector_t *prev, *next = vec;
	int      i;

	if (!vec) return -1;
	for (i = 0; i < vec->size; i += VECTOR_CHUNK_SIZE) {
		prev = next;
		next = next->chunk[VECTOR_CHUNK_SIZE];
		if (prev != vec) free(prev);
	}
	memset(vec,0,sizeof(vector_t));
	return 0;
}

int v_free(vector_t *vec) {
	if (!v_zero(vec) == -1) return -1;
	free(vec);
	return 0;
}

int v_freeData(vector_t *vec) {
	viter_t vi;
	void *data;

	if (!vec) return -1;
	v_reset(vec,&vi);
	while ((data = v_each(vec,&vi))) free(data);
	v_zero(vec);
	return 0;
}

int v_push(vector_t *vec, void *data) {
	vector_t *prev, *last = vec;
	int      i, chunk, offset;

	if (!vec) return -1;
	chunk = vec->size / VECTOR_CHUNK_SIZE;
	offset = vec->size % VECTOR_CHUNK_SIZE;
	vec->size++;
	for (i = chunk; i > 0; i--) {
		prev = last;
		last = last->chunk[VECTOR_CHUNK_SIZE];
	}
	if (!offset && chunk) {
		last = v_init(NULL);
		prev->chunk[VECTOR_CHUNK_SIZE] = last;
	}
	last->chunk[offset] = data;
	return vec->size;
}

void *v_pop(vector_t *vec) {
	vector_t *prev, *last = vec;
	int      i, chunk, offset;
	void     *data;

	if (!vec || (vec->size <= 0)) return NULL;
	vec->size--;
	chunk = vec->size / VECTOR_CHUNK_SIZE;
	offset = vec->size % VECTOR_CHUNK_SIZE;
	for (i = chunk; i > 0; i--) {
		prev = last;
		last = last->chunk[VECTOR_CHUNK_SIZE];
	}
	data = last->chunk[offset];
	if (!offset && chunk) {
		v_free(last);
		prev->chunk[VECTOR_CHUNK_SIZE] = NULL;
	}
	return data;
}

void *v_pop_back(vector_t *vec, viter_t* vi) {
	void *data;
	data = v_reset(vec,vi);
	v_remove_at(vec,0,vi);
	return data;
}

int v_size(vector_t *vec) {
	return vec ? vec->size: 0;
}

void *v_jump(vector_t *vec, int idx, viter_t* vi) {
	vector_t *next = vec;
	int      chunk, offset;

	if (!vec || (idx < 0) || (idx >= vec->size)) return NULL;
	chunk = idx / VECTOR_CHUNK_SIZE;
	offset = idx % VECTOR_CHUNK_SIZE;
	for (; chunk > 0; chunk--) next = next->chunk[VECTOR_CHUNK_SIZE];
	if (vi) {
		vi->idx = idx;
		vi->ptr = next->chunk + offset;
	}
	return next->chunk[offset];
}

void *v_elem(vector_t *vec, int idx) {
	return v_jump(vec,idx,NULL);
}


void *v_reset(vector_t *vec, viter_t* vi) {
	if (!vec) return NULL;
	if (!vi) vi = &(vec->vi);
	vi->idx = 0;
	vi->ptr = vec->chunk;
	return vec->size > 0 ? *vi->ptr: NULL;
}

void *v_current(vector_t *vec, viter_t* vi) {
	if (!vec) return NULL;
	if (!vi) vi = &(vec->vi);
	if ((vi->idx >= vec->size) || !vi->ptr) return NULL;
	return *vi->ptr;
}

int v_idx(vector_t *vec, viter_t* vi) {
	if (!vec) return -1;
	if (!vi) vi = &(vec->vi);
	if ((vi->idx >= vec->size) || !vi->ptr) return -1;
	return vi->idx;
}

void *v_next(vector_t *vec, viter_t* vi) {
	if (!vec) return NULL;
	if (!vi) vi = &(vec->vi);
	if ((vi->idx >= vec->size) || !vi->ptr) return NULL;
	vi->idx++;
	vi->ptr++;
	if (vi->idx >= vec->size) return NULL;
	if (!(vi->idx % VECTOR_CHUNK_SIZE)) vi->ptr = ((vector_t *)(*vi->ptr))->chunk;
	return *vi->ptr;
}

void *v_each(vector_t *vec, viter_t* vi) {
	void *data;
	data = v_current(vec,vi);
	v_next(vec,vi);
	return data;
}

int v_remove_at(vector_t *vec, int idx, viter_t* vi) {
	vector_t *v1, *v2;
	int      chunk, offset;
	int      o1, o2;

	if (!vec || (idx < 0) || (idx >= vec->size)) return -1;
	if (!vi) vi = &(vec->vi);
	chunk = idx / VECTOR_CHUNK_SIZE;
	offset = idx % VECTOR_CHUNK_SIZE;
	v1 = vec;
	for (; chunk > 0; chunk--) v1 = v1->chunk[VECTOR_CHUNK_SIZE];
	v2 = v1;
	o1 = o2 = offset;
	for ( idx++, o2++; idx < vec->size; idx++, o1++, o2++) {
		if (o1 >= VECTOR_CHUNK_SIZE) {
			o1 = 0;
			v1 = v1->chunk[VECTOR_CHUNK_SIZE];
		}
		if (o2 >= VECTOR_CHUNK_SIZE) {
			o2 = 0;
			v2 = v2->chunk[VECTOR_CHUNK_SIZE];
		}
		v1->chunk[o1] = v2->chunk[o2];
		if (vi->idx == idx) {	// validating the iterator
			vi->idx = idx - 1;
			vi->ptr = v1->chunk + o1;
		}
	}
	vec->size--;
	if (!(vec->size % VECTOR_CHUNK_SIZE) && (v2 != vec)) v_free(v2);
	return 0;
}	

int v_search(vector_t *vec, void *data) {
	viter_t vi;
	void    *p;
	if (!vec || (vec->size <= 0)) return -1;
	for (v_reset(vec,&vi); (p = v_current(vec,&vi)); v_next(vec,&vi)) {
		if (p == data) return v_idx(vec,&vi);
	}
	return -1;
}

int v_remove(vector_t *vec, void *data) {
	return v_remove_at(vec,v_search(vec,data),NULL);
}

/*
// =======================================================================
void v_debug(vector_t *v) {
	int i;
	void **vp;


	printf("debug:\n");
	vp = v->chunk;
	for (i=0; i < v->size; ) {
		printf(*vp ? "%d - NOT NULL\n": "%d - NULL\n",i);
		if (!vp) break;
		vp++;
		i++;
		if (!(i % VECTOR_CHUNK_SIZE)) vp = ((vector_t *)(*vp))->chunk;
	}
}
*/
