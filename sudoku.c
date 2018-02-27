#include <stdio.h>
#include <stdlib.h>

#define NOT_SOLVED(x) ((x&(x-2))>1)
#define DBG_PRINT(i,j) 
//printf("Calculate [%d][%d] = 0x%.3x\n",i+1,j+1,v[i][j])

#define SUDOKU_METHOD_SCAN_DRAFT_FOR_ONLY_ONE 1
#define SUDOKU_METHOD_SCAN_MULTI_CELLS_IN_ROW 2
#define SUDOKU_METHOD_SCAN_MULTI_CELLS_IN_BLK 4
#define SUDOKU_METHOD_SCAN_DRAFT_FOR_MULTIPLE 8

struct sudoku_cell_pos_t {
	int row;
	int col;
};

struct s_sudoku_t {
	int cell[9][9];
	int pos[9][3][9];
	struct sudoku_cell_pos_t steps[81];
	int solved;
};

int sudoku_init(struct s_sudoku_t *sudoku)
{
	int row, col, blk;
	int t = 0;
	int num;
	int (*v)[9] = sudoku->cell;
	int (*pos)[3][9] = sudoku->pos;
	struct sudoku_cell_pos_t *cell = (struct sudoku_cell_pos_t *)sudoku->steps;

	for (row=0; row<9; row++) {
		for (col=0; col<9; col++) {
			if (!v[row][col]) {
				v[row][col] = 0x3fe;
				continue;
			}

			num = v[row][col];

			v[row][col] = (1<<num) | 1;
			cell[t].row = row;
			cell[t++].col = col;
		}
	}

	sudoku->solved = t;

	return t;
}

int sudoku_print(struct s_sudoku_t *sudoku)
{
	int i, j, m, n;
	int (*v)[9] = sudoku->cell;

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

int sudoku_show_locations(struct s_sudoku_t *sudoku)
{
	int i, j, n;

	for (i=0; i<9; i++) {
		for (n=0; n<9; n++) {
			printf(" 0x%.3x", sudoku->pos[n][0][i]);
		}
		printf("\n");
	}
	printf("\n\n");

	for (n=0; n<9; n++) {
		for (j=0; j<9; j++) {
			printf(" 0x%.3x", sudoku->pos[n][1][j]);
		}
		printf("\n");
	}
	printf("\n\n");

	for (n=0; n<9; n++) {
		for (j=0; j<9; j++) {
			printf(" 0x%.3x", sudoku->pos[n][2][j]);
		}
		printf("\n");
	}
	printf("\n\n");
	return 0;
}


int sudoku_filterout_cell(struct s_sudoku_t *sudoku, int row, int col)
{
	int i,j,k,m,n,blk;
	int solved = 0;
	int (*cell)[9] = sudoku->cell;
	struct sudoku_cell_pos_t *step = &sudoku->steps[sudoku->solved];


	m = sudoku->cell[row][col] & ~1;
	for (n=0; !(m&1); n++, m>>=1);
	blk = row/3*3+col/3;
	sudoku->pos[n-1][0][row] &= ~(1<<col);
	sudoku->pos[n-1][1][col] &= ~(1<<row);
	sudoku->pos[n-1][2][blk] &= ~(1<<((row%3)*3+(col%3)));

	i = row;
	j = col;
	for (k=0; k<9; k++) {
		if (!NOT_SOLVED(cell[i][k])) continue;
		if (cell[i][k] > cell[i][j]) {
			cell[i][k] &= ~cell[i][j];
			DBG_PRINT(i,k);
			if (!NOT_SOLVED(cell[i][k])) {
				step[solved].row = i;
				step[solved++].col = k;
			}
		}
	}
	for (k=0; k<9; k++) {
		if (!NOT_SOLVED(cell[k][j])) continue;
		if (cell[k][j] > cell[i][j]) {
			cell[k][j] &= ~cell[i][j];
			DBG_PRINT(k,j);
			if (!NOT_SOLVED(cell[k][j])) {
				step[solved].row = k;
				step[solved++].col = j;
			}
		}
	}
	for (m=(i-i%3); m<(i-i%3+3); m++) {
		if (m==i) continue;
		for (n=(j-j%3); n<(j-j%3+3); n++) {
			if (!NOT_SOLVED(cell[m][n])) continue;
			if (cell[m][n] <= cell[i][j]) continue;

			cell[m][n] &= ~cell[i][j];
			DBG_PRINT(m,n);
			if (!NOT_SOLVED(cell[m][n])) {
				step[solved].row = m;
				step[solved++].col = n;
			}
		}
	}

	if (solved)
		;//printf("%d is solved.\n",solved);

	sudoku->solved += solved;

	return solved;
}

int sudoku_clear_drafts_in_col_by_block(struct s_sudoku_t *sudoku, int blk, int col, int n)
{
	int row, solved = 0;
	int multi = 0;
	struct sudoku_cell_pos_t *step = &sudoku->steps[sudoku->solved];

	for (row = 0; row < blk/3*3; row++) {
		if (!NOT_SOLVED(sudoku->cell[row][col])) continue;
		sudoku->cell[row][col] &= ~(2<<n);
		if (sudoku->pos[n-1][0][row] & ~(1<<col) ||
			sudoku->pos[n-1][1][col] & ~(1<<row))
			multi++;

		sudoku->pos[n-1][0][row] &= ~(1<<col);
		sudoku->pos[n-1][1][col] &= ~(1<<row);
		if (NOT_SOLVED(sudoku->cell[row][col])) continue;

		step[solved].row = row;
		step[solved++].col = col;
	}
	for (row = blk/3*3+3; row < 9; row++) {
		if (!NOT_SOLVED(sudoku->cell[row][col])) continue;
		sudoku->cell[row][col] &= ~(2<<n);
		if (sudoku->pos[n-1][0][row] & ~(1<<col) ||
			sudoku->pos[n-1][1][col] & ~(1<<row))
			multi++;

		sudoku->pos[n-1][0][row] &= ~(1<<col);
		sudoku->pos[n-1][1][col] &= ~(1<<row);
		if (NOT_SOLVED(sudoku->cell[row][col])) continue;

		step[solved].row = row;
		step[solved++].col = col;
	}

	sudoku->solved += solved;
	return multi;
}

int sudoku_clear_drafts_in_row_by_block(struct s_sudoku_t *sudoku, int blk, int row, int n)
{
	int col, solved = 0;
	int multi = 0;
	struct sudoku_cell_pos_t *step = &sudoku->steps[sudoku->solved];

	for (col = 0; col < (blk%3)*3; col++) {
		if (!NOT_SOLVED(sudoku->cell[row][col])) continue;
		sudoku->cell[row][col] &= ~(2<<n);
		if (sudoku->pos[n][0][row] & ~(1<<col) ||
			sudoku->pos[n][1][col] & ~(1<<row))
			multi++;

		sudoku->pos[n][0][row] &= ~(1<<col);
		sudoku->pos[n][1][col] &= ~(1<<row);
		if (NOT_SOLVED(sudoku->cell[row][col])) continue;

		step[solved].row = row;
		step[solved++].col = col;
	}
	for (col = (blk%3)*3+3; col < 9; col++) {
		if (!NOT_SOLVED(sudoku->cell[row][col])) continue;
		sudoku->cell[row][col] &= ~(2<<n);
		if (sudoku->pos[n][0][row] & ~(1<<col) ||
			sudoku->pos[n][1][col] & ~(1<<row))
			multi++;

		sudoku->pos[n][0][row] &= ~(1<<col);
		sudoku->pos[n][1][col] &= ~(1<<row);
		if (NOT_SOLVED(sudoku->cell[row][col])) continue;

		step[solved].row = row;
		step[solved++].col = col;
	}

	sudoku->solved += solved;
	return multi;
}

void sudoku_clear_loc_by_cell(struct s_sudoku_t *sudoku, int row, int col, int n)
{
	int blk = row/3*3 + col/3;
	int idx = (row%3)*3 + (col%3);

	sudoku->pos[n][0][row] &= ~(1<<col);
	sudoku->pos[n][1][col] &= ~(1<<row);
	sudoku->pos[n][2][blk] &= ~(1<<idx);
}

int num_of_one(int v) {
	int c;

	for (c=0; v; v>>=1)
		if (v&1) c++;

	return c;
}

int sudoku_lookup_only(struct s_sudoku_t *sudoku, int rules)
{
	int i,j,k,t,n,m,s;
	int solved = 0;

	for (i=0; i<3*9*9; i++) {
		((int*)sudoku->pos)[i] = 0;
	}

	for (i=0; i<9; i++) {
		for (j=0; j<9; j++) {
			t = sudoku->cell[i][j] & ~1;
			if (!NOT_SOLVED(t))
				continue;
			for (n=0; t; n++, t>>=1) {
				if (!(t&1))
					continue;
				//mark number n as draft in possible locations of row, col and block
				k = i/3*3+j/3;
				m = (i%3)*3+(j%3);
				sudoku->pos[n-1][0][i] |= (1<<j);
				sudoku->pos[n-1][1][j] |= (1<<i);
				sudoku->pos[n-1][2][k] |= (1<<m);
			}

		}
	}

	int *row[3] = {&i, &j, &m};
	int *col[3] = {&j, &i, &k};
	int multi = 0;
	for (t=0; t<3; t++) {
		solved = sudoku->solved;
		for (i=0; i<9; i++) {
			for (n=0; n<9; n++) {
				int c = 0;
				s = sudoku->pos[n][t][i];
				for (j=-1; s; j++,s>>=1)
					if (s&1) c++;

				s = sudoku->pos[n][t][i];
				m = i/3*3 + j/3;
				k = (i%3)*3 + j%3;
				switch (c) {
				case 1:
				{
					// exist in only one cell
					printf("num %d only be in cells of row #%d, col #%d\n", n+1, *row[t]+1, *col[t]+1);
					sudoku->steps[sudoku->solved].row = *row[t];
					sudoku->steps[sudoku->solved++].col = *col[t];
					sudoku->cell[*row[t]][*col[t]] = 2<<n;
					sudoku_clear_loc_by_cell(sudoku, *row[t], *col[t], n+1);

					if (t<2) break;

					multi += sudoku_clear_drafts_in_row_by_block(sudoku, i, *row[t], n);
					multi += sudoku_clear_drafts_in_col_by_block(sudoku, i, *col[t], n);
					break;
				}
				case 2:
				case 3:
					if (t<2) break;
					if (~rules & SUDOKU_METHOD_SCAN_DRAFT_FOR_MULTIPLE) break;
					if (num_of_one(sudoku->pos[n][0][*row[t]]) > num_of_one(s) &&
							((s & 0x007) == s ||
							(s & 0x038) == s ||
							(s & 0x1c0) == s)) {

						printf("num %d only be in %d cells in blk #%d of row %d\n", n+1, c, i, *row[t]);
						multi += sudoku_clear_drafts_in_row_by_block(sudoku, i, *row[t], n);
					} else if (num_of_one(sudoku->pos[n][1][*col[t]]) > num_of_one(s) &&
							((s & 0x049) == s ||
							(s & 0x092) == s ||
							(s & 0x124) == s)) {
						printf("num %d only be in %d cells in blk #%d of col %d\n", n+1, c, i, *col[t]);
						multi += sudoku_clear_drafts_in_col_by_block(sudoku, i, *col[t], n);
					}
					break;
				default:
					break;
				}
			}
		}

		if (multi > 0)
			printf("Clear %d more drafts for some numbers are only in blocks.\n", multi);

		if (sudoku->solved > solved) {
			// break immediately to caculate pos
			return 0;
		}
	}

	return multi;
}

int sudoku_lookup_multi_in_row(struct s_sudoku_t *sudoku, int row)
{
	int i,j,k,l,v[4];

	for (i=0; i<9; i++) {
		v[0] = sudoku->cell[row][i];
		if (!NOT_SOLVED(v[0])) continue;

		for (j=i+1; j<9; j++) {
			v[1] = sudoku->cell[row][j];
			if (!NOT_SOLVED(v[1])) continue;
			if (num_of_one(v[0]|v[1]) == 2)
			       return v[0]|v[1];

			for (k=j+1; k<9; k++) {
				v[2] = sudoku->cell[row][k];
				if (!NOT_SOLVED(v[2])) continue;
				if (num_of_one(v[0]|v[1]|v[2]) == 3)
				       return v[0]|v[1]|v[2];

				for (l=k+1; l<9; l++) {
					v[3] = sudoku->cell[row][l];
					if (!NOT_SOLVED(v[3])) continue;
					if (num_of_one(v[0]|v[1]|v[2]|v[3]) == 4)
					       return v[0]|v[1]|v[2]|v[3];
				}
			}
		}
	}

	return 0;
}

int sudoku_filterout_multi_in_row(struct s_sudoku_t *sudoku)
{
	int row, col, v;
	int solved = 0;
	struct sudoku_cell_pos_t *step = &sudoku->steps[sudoku->solved];
	int multi = 0, a;

	for (row=0; row<9; row++) {
		v = sudoku_lookup_multi_in_row(sudoku, row);
		if (!v) continue;

		a = 0;
		for (col=0; col<9; col++) {
			if (!NOT_SOLVED(sudoku->cell[row][col])) continue;
			if (!(sudoku->cell[row][col] & ~v))
				continue;

			if (!(sudoku->cell[row][col] & v))
				continue;

			a++;
			sudoku->cell[row][col] &= ~v;
			if (!NOT_SOLVED(sudoku->cell[row][col])) {
				step[solved].row = row;
				step[solved++].col = col;
			}
		}
		multi += a;
		if (a > 0)
			printf("multi number 0x%.3x are found in row #%d.\n", v, row);
	}

	sudoku->solved += solved;

	return multi;
}

int sudoku_lookup_multi_in_blk(struct s_sudoku_t *sudoku, int blk)
{
	int i,j,k,l;
	int row[5], col[5], v[5];

	row[0] = blk/3*3;
	col[0] = blk%3*3;

	for (i=0; i<9; i++) {
		row[1] = row[0] + i/3;
		col[1] = col[0] + i%3;
		v[1] = sudoku->cell[row[1]][col[1]];
		if (!NOT_SOLVED(v[1])) continue;

		for (j=i+1; j<9; j++) {
			row[2] = row[0] + j/3;
			col[2] = col[0] + j%3;
			v[2] = sudoku->cell[row[2]][col[2]];
			if (!NOT_SOLVED(v[2])) continue;
			if (num_of_one(v[1]|v[2]) == 2)
			       return v[1]|v[2];

			for (k=j+1; k<9; k++) {
				row[3] = row[0] + k/3;
				col[3] = col[0] + k%3;
				v[3] = sudoku->cell[row[3]][col[3]];
				if (!NOT_SOLVED(v[3])) continue;
				if (num_of_one(v[1]|v[2]|v[3]) == 3)
				       return v[1]|v[2]|v[3];

				for (l=k+1; l<9; l++) {
					row[4] = row[0] + l/3;
					col[4] = col[0] + l%3;
					v[4] = sudoku->cell[row[4]][col[4]];
					if (!NOT_SOLVED(v[4])) continue;
					if (num_of_one(v[1]|v[2]|v[3]|v[4]) == 4)
					       return v[1]|v[2]|v[3]|v[4];
				}
			}
		}
	}

	return 0;
}

int sudoku_filterout_multi_in_blk(struct s_sudoku_t *sudoku)
{
	int i, j, row, col, v;
	int solved = 0;
	struct sudoku_cell_pos_t *step = &sudoku->steps[sudoku->solved];
	int multi = 0, a;

	for (i=0; i<9; i++) {
		v = sudoku_lookup_multi_in_blk(sudoku, i);
		if (!v) continue;

		a = 0;
		for (j=0; j<9; j++) {
			row = i/3*3 + j/3;
			col = (i%3)*3 + (j%3);
			if (!NOT_SOLVED(sudoku->cell[row][col])) continue;
			if (!(sudoku->cell[row][col] & ~v))
				continue;

			if (!(sudoku->cell[row][col] & v))
				continue;

			sudoku->cell[row][col] &= ~v;
			a++;
			if (!NOT_SOLVED(sudoku->cell[row][col])) {
				step[solved].row = row;
				step[solved++].col = col;
			}
		}
		multi += a;
		if (a > 0)
			printf("multi number 0x%.3x are found in block #%d.\n", v, i);
	}

	sudoku->solved += solved;

	return multi;
}

int sudoku_solve(struct s_sudoku_t *sudoku, int rules)
{
	int s = 0;
	int t = 0;
	int multi = 0;

	for (s=0; s < sudoku->solved && sudoku->solved<81; s++) {
		int i = sudoku->steps[s].row;
		int j = sudoku->steps[s].col;
		if (NOT_SOLVED(sudoku->cell[i][j])) {
			printf(" ******* Error [%d][%d] = 0x%.3x ******\n",i+1,j+1,sudoku->cell[i][j]);
		}

		if (sudoku_filterout_cell(sudoku, i, j))
			;//printf("step %2d: checked [%d][%d]=%d relative cells\n", s, i+1, j+1, sudoku->cell[i][j]);

		//sudoku_print(sudoku);
		do {
			if (s+1 == sudoku->solved && (rules & SUDOKU_METHOD_SCAN_DRAFT_FOR_ONLY_ONE)) {
				while (0 < sudoku_lookup_only(sudoku, rules))
				       printf("loop lookup only\n");
			}

			if (s+1 == sudoku->solved && (rules & SUDOKU_METHOD_SCAN_MULTI_CELLS_IN_ROW)) {
				multi = sudoku_filterout_multi_in_row(sudoku);
			} else
				multi = 0;

			if (s+1 == sudoku->solved && (rules & SUDOKU_METHOD_SCAN_MULTI_CELLS_IN_BLK)) {
				multi += sudoku_filterout_multi_in_blk(sudoku);
			}
		} while (s+1 == sudoku->solved && multi);
	}

	return sudoku->solved;
}

int main(int argc, char *argv[])
{
	struct s_sudoku_t sudoku[] = {
		{
			.cell = {
		{0,8,4,6,0,0,0,0,0},
		{5,3,0,2,0,4,0,9,0},
		{6,0,0,0,9,0,0,0,4},
		{2,9,0,0,3,0,0,6,8},
		{0,0,3,0,0,0,2,0,0},
		{8,7,0,0,5,0,0,1,9},
		{4,0,0,0,7,0,0,0,3},
		{0,5,0,3,0,8,0,4,1},
		{0,0,0,0,0,5,8,2,0}
			}
		},
		{
			.cell = {
		{4,0,0,0,6,5,0,0,7},
		{1,7,0,0,0,0,0,9,0},
		{0,0,0,0,0,4,0,8,0},
		{0,0,2,0,3,0,0,0,9},
		{0,0,0,4,0,1,0,0,0},
		{6,0,0,0,9,0,3,0,0},
		{0,9,0,1,0,0,0,0,0},
		{0,1,0,0,0,0,0,3,8},
		{2,0,0,5,8,0,0,0,1}
			}
		},
		{
			.cell = {
		{9,0,2,0,0,0,0,0,8},
		{0,0,0,0,8,5,0,0,9},
		{4,0,0,2,0,0,0,0,0},
		{0,5,0,0,0,6,3,0,0},
		{0,1,0,0,3,0,0,2,0},
		{0,0,6,4,0,0,0,9,0},
		{0,0,0,0,0,2,0,0,3},
		{5,0,0,8,1,0,0,0,0},
		{6,0,0,0,0,0,7,0,2}
			}
		},
		{
			.cell = {
		{0,0,0,0,3,4,0,0,0},
		{4,0,2,0,0,0,3,1,0},
		{0,0,0,1,0,0,5,0,0},
		{8,0,0,6,0,0,0,3,0},
		{2,0,0,0,1,0,0,0,9},
		{0,3,0,0,0,7,0,0,2},
		{0,0,3,0,0,6,0,0,0},
		{0,7,4,0,0,0,2,0,8},
		{0,0,0,8,9,0,0,0,0}
			}
		},
	};
	int i = 1;
	int rules = 0;

	if (argc > 1)
		i = atoi(argv[1]);
	if (argc > 2)
		rules = atoi(argv[2]);

	//for (i=0; i<argc; i++)
	//	printf("argv[%d]=%s\n", i, argv[i]);

	sudoku_init(&sudoku[i]);
	//sudoku_print(&sudoku[i]);
	printf("%d is solved\n", sudoku[i].solved);

	sudoku_solve(&sudoku[i], rules);

	printf("%d solved.\n", sudoku[i].solved);
	sudoku_print(&sudoku[i]);

	return 0;
}
