#include <stdio.h>
#include <stdlib.h>

#define NOT_READY(x) (x&(x-1))
#define DBG_PRINT(i,j) 
//printf("Calculate [%d][%d] = 0x%.3x\n",i+1,j+1,v[i][j])

struct sudoku_cell_t {
	int row;
	int col;
};

int init(int *v, struct sudoku_cell_t *cell)
{
	int i;
	int t = 0;
	for (i=0; i<81; i++) {
		if (!v[i]) {
			v[i] = 0x3fe;
			continue;
		}
		v[i] = 1<<v[i];
		cell[t].row = i/9;
		cell[t++].col = i%9;
	}

	return t;
}

int show_sudoku(int v[][9])
{
	int i, j, m, n;

	printf("=========================================================================\n");
	for (i=0; i<9*3; i++) {
		m = i % 3;
		printf("|");
		for (j=0; j<9; j++)
		{
			int t = v[i/3][j];
			int c = 0;
			if (!t || NOT_READY(t)) {
				//printf(" 0");
			//	continue;
			} else for (c=0; !(t&1); c++) {
				t >>= 1;
			}

			if (c) {
				printf(" %d %d %d",c,c,c);
				//printf(" \e[5;32m%d %d %d\e[m",c,c,c);
			} else for (n=0; n<3; n++) {
				c = m*3+n+1;
				if (t & (1<<c))
					printf(" \e[1;31m%d\e[m",c);
				else
					printf("  ");
			}
			if (j%3==2)
				printf(" |");
			else
				printf(" |");
		}
		printf("\n");
		if (i%9==8)
			printf("=========================================================================\n");
		else if (i%3==2)
			printf("-------------------------------------------------------------------------\n");
	}
	printf("\n");	

	return 0;
}

int main(int argc, char *argv[])
{
	int i,j,k,m,n;
	int v[][9] = {
#if 0
		{4,0,0,0,6,5,0,0,7},
		{1,7,0,0,0,0,0,9,0},
		{0,0,0,0,0,4,0,8,0},
		{0,0,2,0,3,0,0,0,9},
		{0,0,0,4,0,1,0,0,0},
		{6,0,0,0,9,0,3,0,0},
		{0,9,0,1,0,0,0,0,0},
		{0,1,0,0,0,0,0,3,8},
		{2,0,0,5,8,0,0,0,1},
#else
		{0,8,4,6,0,0,0,0,0},
		{5,3,0,2,0,4,0,9,0},
		{6,0,0,0,9,0,0,0,4},
		{2,9,0,0,3,0,0,6,8},
		{0,0,3,0,0,0,2,0,0},
		{8,7,0,0,5,0,0,1,9},
		{4,0,0,0,7,0,0,0,3},
		{0,5,0,3,0,8,0,4,1},
		{0,0,0,0,0,5,8,2,0},
#endif
	};
	int done = 1;
	struct sudoku_cell_t sudoku_solve[81];
	int sudoku_solved = 0;
	int sudoku_solving = 0;

	if (argc < 1) return 0;

	//for (i=0; i<argc; i++)
	//	printf("argv[%d]=%s\n", i, argv[i]);

	sudoku_solving = init((int*)v, sudoku_solve);
	show_sudoku(v);
	printf("%d is solved\n", sudoku_solving);

	for (sudoku_solved=0; sudoku_solved < sudoku_solving; sudoku_solved++) {
		i = sudoku_solve[sudoku_solved].row;
		j = sudoku_solve[sudoku_solved].col;
	//	printf("step %2d: checking [%d][%d]=%d relative cells\n", sudoku_solved, i+1, j+1, v[i][j]);
	}

	for (sudoku_solved=0; sudoku_solved < sudoku_solving && sudoku_solving<81; sudoku_solved++) {
		done = 0;
		i = sudoku_solve[sudoku_solved].row;
		j = sudoku_solve[sudoku_solved].col;
		if (NOT_READY(v[i][j])) {
			printf(" ******* Error [%d][%d] = 0x%.3x ******\n",i+1,j+1,v[i][j]);
		}
		printf("step %2d: checking [%d][%d]=%d relative cells\n", sudoku_solved, i+1, j+1, v[i][j]);
		for (k=0; k<9; k++) {
			if (k==j || !NOT_READY(v[i][k])) continue;
			v[i][k] &= ~v[i][j];
			DBG_PRINT(i,k);
			if (!NOT_READY(v[i][k])) {
				done++;
				sudoku_solve[sudoku_solving].row = i;
				sudoku_solve[sudoku_solving++].col = k;
			}

		}
		for (k=0; k<9; k++) {
			if (k==i || !NOT_READY(v[k][j])) continue;
			v[k][j] &= ~v[i][j];
			DBG_PRINT(k,j);
			if (!NOT_READY(v[k][j])) {
				done++;
				sudoku_solve[sudoku_solving].row = k;
				sudoku_solve[sudoku_solving++].col = j;
			}
		}
		for (m=(i-i%3); m<(i-i%3+3); m++) {
			if (m==i) continue;
			for (n=(j-j%3); n<(j-j%3+3); n++) {
				if (n==j || !NOT_READY(v[m][n])) continue;
				v[m][n] &= ~v[i][j];
				DBG_PRINT(m,n);
				if (!NOT_READY(v[m][n])) {
					done++;
					sudoku_solve[sudoku_solving].row = m;
					sudoku_solve[sudoku_solving++].col = n;
				}
			}
		}
		printf("%d is solved.\n",done);
	}

	printf("solved=%d, solving=%d\n",sudoku_solved,sudoku_solving);
	show_sudoku(v);
	return 0;
}
