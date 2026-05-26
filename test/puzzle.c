/*

N-puzzle optimal solution depth-first search.
---------------------------------------------

Taken time:
	About 3-4 hours to formalize the chosen solution.
	Additionally about 4-5 hours of thinking on other possible strategies.

Note:
	I didn't implement any depth adaptive politics leaving some brain space for the future.

*/


#include <stdio.h>
#include <string.h>

#define  SIZE       3
#define  LEN        SIZE*SIZE
#define  MAX_DEPTH  25


typedef  int field_t[LEN];
typedef  int seq_t;


// prints a given field matrix
void print_field(field_t *fld) {
	int     i, j, k;

	if (!fld) return;
	for (j=0; j<SIZE; j++) {
		for (i=0; i<SIZE; i++) {
			k = (*fld)[j*SIZE+i];
			if (k) printf("%2d",k);
			else printf("  ");
		}
		printf("\n");
	}
	printf("\n");
}

// sets a field up
int input_field(field_t *fld) {
	int     i, j, k, n, l;
	int hh;

	if (!fld) return 0;
	printf("Please define the numbers. Enter \"0\" to mark an empty position.\n");
	for (j=0; j<SIZE; j++) {
		for (i=0; i<SIZE; i++) {
			l = j*SIZE + i;
			printf("[%d, %d]: ",i+1,j+1);
			if (!scanf("%d",&k)) {
				printf("Wrong input!\n");
				return 0;
			}
			if ((k < 0) || (k >= LEN)) {
				printf("Number should be between %d and %d!\n",0,LEN-1);
				return 0;
			}
			for (n=0; n<l; n++) {
				if ((*fld)[n] == k) {
					printf("You've already used this number!\n");
					return 0;
				}
			}
			(*fld)[l] = k;
		}
	}
	printf("\n");
	return 1;
}

// checks whether a field is in the final state
int check(field_t *fld) {
	int     i;

	if (!fld) return 0;
	for (i=0; i<LEN-1; i++) {
		if ((*fld)[i] != i+1) return 0;
	}
	return 1;
}

// moves a given empty position
// dir = (0 - up, 1 - down, 2 - left, 3 - right) 
int move(field_t *src, field_t *dst, int pos, int dir) {
	if (!src || !dst) return 0;
	if (src != dst) memcpy(dst,src,sizeof(field_t));
	if ((dir == 0) && ((pos - SIZE) >= 0)) {	// up
		(*dst)[pos] = (*dst)[pos - SIZE];
		(*dst)[pos - SIZE] = 0;
	} else 
	if ((dir == 1) && ((pos + SIZE) < LEN)) {	// down
		(*dst)[pos] = (*dst)[pos + SIZE];
		(*dst)[pos + SIZE] = 0;
	} else
	if ((dir == 2) && ((pos % SIZE - 1) >= 0)) {	// left
		(*dst)[pos] = (*dst)[pos - 1];
		(*dst)[pos - 1] = 0;
	} else
	if ((dir == 3) && ((pos % SIZE + 1) < SIZE)) {	// right
		(*dst)[pos] = (*dst)[pos + 1];
		(*dst)[pos + 1] = 0;
	} else return 0;
	return 1;
}

// calculates the optimal sequence for a given state
// returns the length of the sequence
// prevDir = (0 - up, 1 - down, 2 - left, 3 - right) - helps to avoid simple cycles and reduce the amount of childs
int get_seq(seq_t *seq, field_t *fld, int n, int prevDir) {
	int     pos, dir, l, minL, minDir;
	field_t moveFlds[4];
	seq_t   moveSeqs[4][MAX_DEPTH];

	if (!seq || !fld) return -1;
	if (check(fld)) return 0;
	if (n >= MAX_DEPTH) return -1;
	for (pos=0; pos<LEN; pos++) {
		if ((*fld)[pos] == 0) break;
	}

	// moving the empty position
	minL = minDir = -1;
	for (dir=0; dir<4; dir++) {
		if ((prevDir >= 0) && ((!(prevDir % 2) ? prevDir + 1 : prevDir - 1) == dir)) continue;	// skipping the opposite direction
		if (!move(fld,&(moveFlds[dir]),pos,dir)) continue;
		l = get_seq(moveSeqs[dir],&(moveFlds[dir]),n+1,dir);
		if (l < 0) continue;
		if ((minL >= 0) && (minL <= l)) continue;
		minL = l;
		minDir = dir;
	}
	if ((minL >= 0) && (minDir >= 0)) {
		seq[0] = minDir;
		for (l=0; l<minL; l++) seq[l+1] = moveSeqs[minDir][l];
		return minL + 1;
	}
	return -1;
}

// --------------------------------------------------------------------------------------------------------------

int main() {
	int     i, l, pos;
	field_t fld;
	seq_t   seq[MAX_DEPTH];
	char    *moveStr[] = {
		"DOWN",
		"UP",
		"RIGHT",
		"LEFT"
	};

	if (!input_field(&fld)) return 0;
	printf("GIVEN:\n");
	print_field(&fld);
	printf("Searching...\n");
	l = get_seq(seq,&fld,0,-1);
	for (i=0; i<l; i++) {
		printf("#%d: move %s\n",i+1,moveStr[seq[i]]);
		for (pos=0; pos<LEN; pos++) {
			if (fld[pos] == 0) break;
		}
		move(&fld,&fld,pos,seq[i]);
		print_field(&fld);
	}
	if (l >= 0) {
		printf("Move count: %d\n",l);
	} else {
		printf("Can't find the solution with the given search depth (%d)!\n",MAX_DEPTH);
	}
	return 0;
}
