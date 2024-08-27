#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <termios.h>
#include <time.h>
#include <sys/time.h>
#include <sys/select.h>

#define KEY_LEFT   'A'
#define KEY_RIGHT  'D'
#define KEY_DOWN   'S'
#define KEY_ROTATE 'W'

#define  _CASE(c, n) \
	case KEY_##c: \
	case KEY_##c + 0x20 * n:

#if KEY_LEFT >= 'A' && KEY_LEFT <= 'Z'
#define CASE_LEFT _CASE(LEFT, 1)
#elif KEY_LEFT >= 'a' && KEY_LEFT <= 'z'
#define CASE_LEFT _CASE(LEFT, -1)
#else
#define CASE_LEFT case KEY_LEFT:
#endif

#if KEY_RIGHT >= 'A' && KEY_RIGHT <= 'Z'
#define CASE_RIGHT _CASE(RIGHT, 1)
#elif KEY_RIGHT >= 'a' && KEY_RIGHT <= 'z'
#define CASE_RIGHT _CASE(RIGHT, -1)
#else
#define CASE_RIGHT case KEY_RIGHT:
#endif

#if KEY_DOWN >= 'A' && KEY_DOWN <= 'Z'
#define CASE_DOWN _CASE(DOWN, 1)
#elif KEY_DOWN >= 'a' && KEY_DOWN <= 'z'
#define CASE_DOWN _CASE(DOWN, -1)
#else
#define CASE_DOWN case KEY_DOWN:
#endif

#if KEY_ROTATE >= 'A' && KEY_ROTATE <= 'Z'
#define CASE_ROTATE _CASE(ROTATE, 1)
#elif KEY_ROTATE >= 'a' && KEY_ROTATE <= 'z'
#define CASE_ROTATE _CASE(ROTATE, -1)
#else
#define CASE_ROTATE case KEY_ROTATE:
#endif

#define X 0
#define Y 1
#define FOR_BLOCKS for(i = 0; i < 4; i++)
#define ANY_BLOCKS BLOCK(0) | BLOCK(1) | BLOCK(2) | BLOCK(3)
#define ALL_BLOCKS BLOCK(0) & BLOCK(1) & BLOCK(2) & BLOCK(3)

fd_set input_set;
struct termios oldtc, newtc;
struct timeval timeout;
struct timespec tstart={0, 0}, tend={0, 0};
int speed, score;
char i, j, next, lvl, line, type, status, end, min, max, tmp, frame, sc, star, quit = 0, piece[4][2], buf[10][22];

char shape[7][2][9] = {
	{
		"  [][][]",
		"    []  "
	},
	{
		"  [][][]",
		"      []"
	},
	{
		"  [][]  ",
		"    [][]"
	},
	{
		"    [][]",
		"    [][]"
	},
	{
		"    [][]",
		"  [][]  ",
	},
	{
		"  [][][]",
		"  []    "
	},
	{
		"[][][][]",
		"        "
	}
};

char gen[7][4][2] = {
	{{5, 2}, {4, 2}, {5, 3}, {6, 2}},
	{{5, 2}, {4, 2}, {6, 2}, {6, 3}},
	{{5, 2}, {5, 3}, {4, 2}, {6, 3}},
	{{4, 2}, {5, 2}, {4, 3}, {5, 3}},
	{{5, 2}, {6, 2}, {4, 3}, {5, 3}},
	{{5, 2}, {4, 2}, {6, 2}, {4, 3}},
	{{5, 2}, {4, 2}, {6, 2}, {3, 2}}
};

char getch() {
	char ch, ready = 0;
	FD_ZERO(&input_set);
	FD_SET(STDIN_FILENO, &input_set);
	ready = select(1, &input_set, NULL, NULL, &timeout);
	if (ready == EOF) return 0;
	if (ready) {
		read(0, &ch, 1);
		return ch;
	}
	else return 0;
}

void title() {
	printf("\x1b[H\x1b[J\x1b[7;32H[ ]\x1b[8;32HТ Е Т Р И С\x1b[9;40H[ ]");
	do {
		printf("\x1b[20;17H\x1b[KВАШ УРОВЕНЬ? (0-9) - ");
		scanf("%c", &lvl);
		if(lvl >= '0' && lvl <= '9') break;
		if(lvl == 'q' || lvl == 'Q') {
			quit = 1;
			break;
		}
	} while(1);
	while(getchar() != '\n');
	lvl -= '0';
}

void init() {
	end = frame = status = score = star = line = 0;
	speed = 47 - (lvl * 5);
	next = rand() % 7;
	for(i = 0; i < 10; i++) for(j = 0; j < 22; j++) buf[i][j] = 0;
	printf("\x1b[?25l\x1b[H\x1b[J\x1b[2HПОЛНЫХ СТРОК:  0\x1b[3HУРОВЕНЬ:       %d\x1b[4;3HСЧЕТ:    0\x1b[2;29H", lvl);
	for(i = 0; i < 20; i++) printf("<! . . . . . . . . . .!>\x1b[1B\x1b[24D");
	printf("<!====================!>\x1b[1B\x1b[22D\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\n");
}

void new() {
	sc = 19;
	type = next;
	next = rand() % 7;
	for(int i = 0; i < 4; i++) {
		piece[i][X] = gen[type][i][X];
		piece[i][Y] = gen[type][i][Y];
	}
#define BLOCK(n) (buf[piece[n][X]][piece[n][Y]])
	if(ANY_BLOCKS) end = 1;
#undef  BLOCK
	printf("\x1b[11;19H%s\x1b[12;19H%s\n", shape[next][X], shape[next][Y]);
	status = 1;
}

void eliminate() {
	score += sc;
	if(lvl) score += 3 * (lvl + 1);
	if(score >= 1000) {
		score -= 1000;
		star++;
		printf("\x1b[5H");
		for(j = 0; j < star; j++) {
			if(!(j % 5)) putchar('\n');
			printf(" ¤");
		}
	}
	FOR_BLOCKS buf[piece[i][X]][piece[i][Y]] = 1;
	min = max = piece[0][1];
	printf("\x1b[4;9H%4d", score);
	for(i = 0; i < 4; i++) min > piece[i][1] ? min = piece[i][1] : 0, max < piece[i][1] ? max = piece[i][1] : 0;
	for(i = min; i <= max; i++) {
		tmp = 0;
		for(j = 0; j < 10; j++) tmp += !buf[j][i];
		if(!tmp) {
			for(j = i; j > 0; j--) for(int k = 0; k < 10; k++) buf[k][j] = i ? buf[k][j - 1] : 0;
			line++;

			if(line == lvl * 10 + 6 && lvl < 9) lvl++, speed -= 5;
			printf("\x1b[2;14H%3d\x1b[3;16H%d", line, lvl);
		}
	}
	status = 0;
}

void fall() {
	if(frame == 0) {
#define BLOCK(n) (!buf[piece[n][X]][piece[n][Y] + 1] & piece[n][Y] < 21)
		if(ALL_BLOCKS) {
			sc--;
			FOR_BLOCKS piece[i][Y]++;
		}
#undef  BLOCK
		else eliminate();
	}
}

void left() {
#define BLOCK(n) (!buf[piece[n][X] - 1][piece[n][Y]] & piece[n][X] > 0)
	if(ALL_BLOCKS) FOR_BLOCKS piece[i][X]--;
#undef  BLOCK
}

void right() {
#define BLOCK(n) (!buf[piece[n][X] + 1][piece[n][Y]] & piece[n][X] < 9)
	if(ALL_BLOCKS) FOR_BLOCKS piece[i][X]++;
#undef  BLOCK
}

void down() {
#define BLOCK(n) (!buf[piece[n][X]][piece[n][Y] + 1] & piece[n][Y] < 21)
	while(ALL_BLOCKS) FOR_BLOCKS piece[i][Y]++;
#undef  BLOCK
	eliminate();
}

void rotate() {
	switch(type) {
		case 0:
			switch(status) {
				case 1:
					if(!buf[piece[0][X]][piece[0][Y] - 1]) {
						piece[1][X]++;
						piece[1][Y]++;
						piece[2][X]++;
						piece[2][Y]--;
						piece[3][X]--;
						piece[3][Y]--;
						status++;
					}
					break;

				case 2:
					if(!buf[piece[0][X] - 1][piece[0][Y]] & piece[0][X] > 0) {
						piece[1][X]++;
						piece[1][Y]--;
						piece[2][X]--;
						piece[2][Y]--;
						piece[3][X]--;
						piece[3][Y]++;
						status++;
					}
					break;

				case 3:
					if(!buf[piece[0][X]][piece[0][Y] + 1] & piece[0][Y] < 19) {
						piece[1][X]--;
						piece[1][Y]--;
						piece[2][X]--;
						piece[2][Y]++;
						piece[3][X]++;
						piece[3][Y]++;
						status++;
					}
					break;

				case 4:
					if(!buf[piece[0][X] + 1][piece[0][Y]] & piece[0][X] < 9) {
						piece[1][X]--;
						piece[1][Y]++;
						piece[2][X]++;
						piece[2][Y]++;
						piece[3][X]++;
						piece[3][Y]--;
						status = 1;
					}
					break;
			}
			break;

		case 1:
			switch(status) {
				case 1:
					if(!buf[piece[0][X]][piece[0][Y] + 1] & !buf[piece[0][X]][piece[0][Y] - 1] & !buf[piece[0][X] + 1][piece[0][Y] + 1]) {
						piece[1][X]++;
						piece[1][Y]--;
						piece[2][X]--;
						piece[2][Y]++;
						piece[3][X] -= 2;
						status++;
					}
					break;

				case 2:
					if(!buf[piece[0][X] + 1][piece[0][Y]] & !buf[piece[0][X] - 1][piece[0][Y]] & !buf[piece[0][X] - 1][piece[0][Y] - 1] & piece[0][X] < 9) {
						piece[1][X]++;
						piece[1][Y]++;
						piece[2][X]--;
						piece[2][Y]--;
						piece[3][Y] -= 2;
						status++;
					}
					break;

				case 3:
					if(!buf[piece[0][X]][piece[0][Y] + 1] & !buf[piece[0][X]][piece[0][Y] - 1]  & !buf[piece[0][X] + 1][piece[0][Y] + 1] & piece[0][Y] < 19) {
						piece[1][X]--;
						piece[1][Y]++;
						piece[2][X]++;
						piece[2][Y]--;
						piece[3][X] += 2;
						status++;
					}
					break;

				case 4:
					if(!buf[piece[0][X] + 1][piece[0][Y]] & !buf[piece[0][X] - 1][piece[0][Y]] & !buf[piece[0][X] + 1][piece[0][Y] + 1] & piece[0][X] > 0) {
						piece[1][X]--;
						piece[1][Y]--;
						piece[2][X]++;
						piece[2][Y]++;
						piece[3][Y] += 2;
						status = 1;
					}
					break;
			}
			break;

		case 2:
		case 4:
			switch(status) {
				case 1:
					if(!buf[piece[1][X] + 1][piece[0][Y]] & !buf[piece[0][X] + 1][piece[0][Y] - 1]) {
						piece[2][X] += 2;
						piece[3][Y] -= 2;
						status++;
					}
					break;
				case 2:
					if(!buf[piece[1][X] - 1][piece[0][Y]] & !buf[piece[0][X] + 1][piece[0][Y] + 1] & piece[0][X] > 0) {
						piece[2][X] -= 2;
						piece[3][Y] += 2;
						status = 1;
					}
					break;
			}
			break;

		case 5:
			switch(status) {
				case 1:
					if(!buf[piece[0][X]][piece[0][Y] + 1] & !buf[piece[0][X]][piece[0][Y] - 1]) {
						piece[1][X]++;
						piece[1][Y]--;
						piece[2][X]--;
						piece[2][Y]++;
						piece[3][Y] -= 2;
						status++;
					}
					break;
				case 2:
					if(!buf[piece[0][X] + 1][piece[0][Y]] & !buf[piece[0][X] - 1][piece[0][Y]] & piece[0][X] < 9) {
						piece[1][X]++;
						piece[1][Y]++;
						piece[2][X]--;
						piece[2][Y]--;
						piece[3][X] += 2;
						status++;
					}
					break;
				case 3:
					if(!buf[piece[0][X]][piece[0][Y] + 1] & !buf[piece[0][X]][piece[0][Y] - 1] & piece[0][Y] < 19) {
						piece[1][X]--;
						piece[1][Y]++;
						piece[2][X]++;
						piece[2][Y]--;
						piece[3][Y] += 2;
						status++;
					}
					break;
				case 4:
					if(!buf[piece[0][X] + 1][piece[0][Y]] & !buf[piece[0][X] - 1][piece[0][Y]] & piece[0][X] > 0) {
						piece[1][X]--;
						piece[1][Y]--;
						piece[2][X]++;
						piece[2][Y]++;
						piece[3][X] -= 2;
						status = 1;
					}
					break;
			}
			break;

		case 6:
			switch(status) {
				case 1:
					if(!buf[piece[0][X]][piece[0][Y] + 1] & !buf[piece[0][X]][piece[0][Y] - 2] & !buf[piece[0][X]][piece[0][Y] - 1] & piece[0][Y] < 19) {
						piece[1][X]++;
						piece[1][Y]--;
						piece[2][X]--;
						piece[2][Y]++;
						piece[3][X] += 2;
						piece[3][Y] -= 2;
						status++;
					}
					break;
				case 2:
					if(!buf[piece[0][X] + 1][piece[0][Y]] & !buf[piece[0][X] - 2][piece[0][Y]] & !buf[piece[0][X] - 1][piece[0][Y]] & piece[0][X] > 1 & piece[0][X] < 9) {
						piece[1][X]--;
						piece[1][Y]++;
						piece[2][X]++;
						piece[2][Y]--;
						piece[3][X] -= 2;
						piece[3][Y] += 2;
						status = 1;
					}
					break;
			}
			break;
	}
}

void loop() {
	frame++;
	if(frame >= speed) frame = 0;
	if(status == 0) new();
	else fall();
	switch(getch()) {
		CASE_LEFT
			left();
			break;

		CASE_RIGHT
			right();
			break;

		CASE_DOWN
			down();
			break;

		CASE_ROTATE
			rotate();
			break;

		case '\x1b':
			end = 1;
			break;
	}
}

void refresh() {
	printf("\x1b[2;31H");
	for(i = 2; i < 22; i++) {
		for(j = 0; j < 10; j++) {
#define BLOCK(n) (piece[n][Y] == i & piece[n][X] == j)
			if (buf[j][i] | ANY_BLOCKS) {
#undef  BLOCK
				printf("[]");
			} else {
				printf(" .");
			}
		}
		printf("\x1b[1B\x1b[20D");
	}
	putchar('\n');
}

void Quit() {
    quit = 1;
}

int main() {
	printf("\x1b[32m");
	signal(SIGINT, Quit);
	tcgetattr(STDIN_FILENO, &oldtc);
	newtc = oldtc;
	newtc.c_lflag &= ~(ICANON | ECHO);
	while(!quit) {
		srand(time(0));
		title();
		init();
		tcsetattr(STDIN_FILENO, TCSANOW, &newtc);
		while(!quit && !end) {
			clock_gettime(CLOCK_MONOTONIC, &tstart);
			loop();
			refresh();
			do {
				clock_gettime(CLOCK_MONOTONIC, &tend);
				if(((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec) >= 0.033) break;
			} while(1);
		}
		tcsetattr(STDIN_FILENO, TCSANOW, &oldtc);
		printf("\x1b[?25h");
	}
	printf("\x1b[H\x1b[J\x1b[0m");
}