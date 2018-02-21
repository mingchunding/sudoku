#include <stdio.h>
#include <stdlib.h>

#define NOT_SOLVED(x) ((x&(x-2))>1)
#define DBG_PRINT(i,j) 
//printf("Calculate [%d][%d] = 0x%.3x\n",i+1,j+1,v[i][j])

struct sudoku_cell_t {
	int row;
	int col;
};

int sudoku_init(int *v, struct sudoku_cell_t *cell)
{
	int i;
	int t = 0;
	for (i=0; i<81; i++) {
		if (!v[i]) {
			v[i] = 0x3fe;
			continue;
		}
		v[i] = 1<<v[i] | 1;
		cell[t].row = i/9;
		cell[t++].col = i%9;
	}

	return t;
}

int sudoku_print(int v[][9])
{
	int i, j, m, n;

	printf("=========================================================================\n");
	for (i=0; i<9*3; i++) {
		m = i % 3;
		printf("|");
		for (j=0; j<9; j++)
		{
			int t = v[i/3][j] & ~1;
			int c = 0;
			if (!t || NOT_SOLVED(t)) {
				//printf(" 0");
			//	continue;
			} else for (c=0; !(t&1); c++) {
				t >>= 1;
			}

			if (c) {
				if (v[i/3][j] & 1)
					printf(" \e[1;34m%d %d %d\e[m",c,c,c);
				else
					printf(" %d %d %d",c,c,c);
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

	//sleep(1);

	return 0;
}

int sudoku_filterout_cell(int sudoku[][9], int row, int col, struct sudoku_cell_t *step)
{
	int i,j,k,m,n;
	int solved = 0;

	i = row;
	j = col;
	for (k=0; k<9; k++) {
		if (!NOT_SOLVED(sudoku[i][k])) continue;
		if (sudoku[i][k] > sudoku[i][j]) {
			sudoku[i][k] &= ~sudoku[i][j];
			DBG_PRINT(i,k);
			if (!NOT_SOLVED(sudoku[i][k])) {
				step[solved].row = i;
				step[solved++].col = k;
			}
		}
	}
	for (k=0; k<9; k++) {
		if (!NOT_SOLVED(sudoku[k][j])) continue;
		if (sudoku[k][j] > sudoku[i][j]) {
			sudoku[k][j] &= ~sudoku[i][j];
			DBG_PRINT(k,j);
			if (!NOT_SOLVED(sudoku[k][j])) {
				step[solved].row = k;
				step[solved++].col = j;
			}
		}
	}
	for (m=(i-i%3); m<(i-i%3+3); m++) {
		if (m==i) continue;
		for (n=(j-j%3); n<(j-j%3+3); n++) {
			if (!NOT_SOLVED(sudoku[m][n])) continue;
			if (sudoku[m][n] <= sudoku[i][j]) continue;

			sudoku[m][n] &= ~sudoku[i][j];
			DBG_PRINT(m,n);
			if (!NOT_SOLVED(sudoku[m][n])) {
				step[solved].row = m;
				step[solved++].col = n;
			}
		}
	}

	if (solved)
		printf("%d is solved.\n",solved);

	return solved;
}

int sudoku_loopup_only(int sudoku[][9], struct sudoku_cell_t *step)
{
	int i,j,t,n;
	struct _group_t {
		int repeat;
		int row;
		int col;
	} times[3][9][9];

	int solved = 0;

	for (i=0; i<3*9*9; i++)
		((struct _group_t*)times)[i].repeat = 0;

	for (i=0; i<9; i++) {
		for (j=0; j<9; j++) {
			t = sudoku[i][j] & ~1;
			if (!NOT_SOLVED(t))
				continue;
			for (n=0; t; n++, t>>=1) {
				if (!(t&1))
					continue;
				times[0][i][n-1].repeat++;
				times[0][i][n-1].row = i;
				times[0][i][n-1].col = j;
				times[1][j][n-1].repeat++;
				times[1][j][n-1].row = i;
				times[1][j][n-1].col = j;
				times[2][i/3*3+j/3][n-1].repeat++;
				times[2][i/3*3+j/3][n-1].row = i;
				times[2][i/3*3+j/3][n-1].col = j;
			}

		}
	}

	for (n=0; n<3; n++) {
		for (i=0; i<9; i++) {
			for (j=0; j<9; j++) {
				if (times[n][i][j].repeat == 1) {
					t = 2<<j;
					step[solved].row = times[n][i][j].row;
					step[solved++].col = times[n][i][j].col;
					sudoku[times[n][i][j].row][times[n][i][j].col] = t;
				}
			}
		}

		if (solved >0)
			break;
	}

	return solved;
}

int sudoku_solve(int sudoku[][9], struct sudoku_cell_t *step, int solved)
{
	int s = 0;
	int t = 0;

	for (s=0; s < solved && solved<81; s++) {
		int i = step[s].row;
		int j = step[s].col;
		if (NOT_SOLVED(sudoku[i][j])) {
			printf(" ******* Error [%d][%d] = 0x%.3x ******\n",i+1,j+1,sudoku[i][j]);
		}

		t = sudoku_filterout_cell(sudoku, i, j, &step[solved]);
		solved += t;
		if (t)
			printf("step %2d: checked [%d][%d]=%d relative cells\n", s, i+1, j+1, sudoku[i][j]);

		//sudoku_print(sudoku);
		if (s == solved-1) {
			solved += sudoku_loopup_only(sudoku, &step[solved]);
		}
	}


	return solved;
}

#define sample 2
int main(int argc, char *argv[])
{
	int i,j,k,m,n;
	int sudoku[][9] = {
#if (sample==2)

		{9,0,2,0,0,0,0,0,8},
		{0,0,0,0,8,5,0,0,9},
		{4,0,0,2,0,0,0,0,0},
		{0,5,0,0,0,6,3,0,0},
		{0,1,0,0,3,0,0,2,0},
		{0,0,6,4,0,0,0,9,0},
		{0,0,0,0,0,2,0,0,3},
		{5,0,0,8,1,0,0,0,0},
		{6,0,0,0,0,0,7,0,2},
#elif (sample==1)
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
	struct sudoku_cell_t sudoku_step[81];
	int solved = 0;

	solved = sudoku_init((int*)sudoku, sudoku_step);
	sudoku_print(sudoku);
	printf("%d is solved\n", solved);

	solved = sudoku_solve(sudoku, sudoku_step, solved);

	printf("%d solved.\n", solved);
	sudoku_print(sudoku);
	return 0;
}
