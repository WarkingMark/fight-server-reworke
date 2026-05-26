/* 
 * modifed: igorpauk 2017-18
 */

#ifndef __VEC_H__
#define __VEC_H__


#define VECTOR_CHUNK_SIZE	20

typedef struct vector_s vector_t;
typedef struct viter_s viter_t;

struct viter_s {
	int     idx;
	void    **ptr;
};

struct vector_s {
	int	    size;
	void    *chunk[VECTOR_CHUNK_SIZE+1];
	viter_t vi;
};

vector_t *v_init(vector_t *vec);
vector_t *v_copy(vector_t *vec);
int v_zero(vector_t *vec);
int v_free(vector_t *vec);
int v_freeData(vector_t *vec);

int v_push(vector_t *vec, void *data);
void *v_pop(vector_t *vec);
void *v_pop_back(vector_t *vec, viter_t* vi);
int v_size(vector_t *vec);

void *v_jump(vector_t *vec, int idx, viter_t* vi);
void *v_elem(vector_t *vec, int idx);
void *v_reset(vector_t *vec, viter_t* vi);
void *v_current(vector_t *vec, viter_t* vi);
int v_idx(vector_t *vec, viter_t* vi);
void *v_next(vector_t *vec, viter_t* vi);
void *v_each(vector_t *vec, viter_t* vi);
int v_remove_at(vector_t *vec, int idx, viter_t* vi);
int v_remove(vector_t *vec, void *data);
int v_search(vector_t *vec, void *data);


#endif
