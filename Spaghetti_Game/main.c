# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <unistd.h>
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <math.h>
# include "mlx.h"

//Non standard library for solong games
#include <dirent.h>
#include <sys/time.h>
#include <ctype.h>

# define ESC 65307
# define RX_ARROW 65363
# define LX_ARROW 65361
# define UP_ARROW 65362
# define DW_ARROW 65364
# define T 116

#define MAX_LINE_LEN 2048  // lunghezza massima di una riga
#define MIN_SIZE 4         // dimensione minima (4x4)
#define MAX_NAME_LEN 20   // massimo 20 caratteri per nome file xpm
#define MAX_IMAGES 50
#define MAX_TIME 1*60	// tempo massimo per vincere
#define TILE_SIZE 64

enum e_bool {
	false,
	true,
};

enum e_io {
	in,
	out,
	err,
};

typedef struct s_pos {
    int x;
    int y;
} t_pos;

typedef struct s_data {
	void	*image;
	char	*addr;
	int		bits_per_pixel;
	int		line_length;
	int		endian;
}	t_data;

typedef struct s_images {
	char name[MAX_NAME_LEN];
	t_data img;
} t_images;

typedef struct s_env {
	void	*mlx;
	void	*mlx_win;

	//Game status variables
	char 	**M;
	char	old_tile;
	int		row;
	int		column;
	int		coin;
	int		score;
	char	body_paint;
	t_pos	pos_p;
	t_pos	pos_e;
    t_images images[MAX_IMAGES];
	struct timeval start_time; 
	int		frame;
	int		timer;
	int		is_over;
	int		is_transformed;
	int		moves;
}	t_env;

// 0-9 and : represented as an array of bits (1 = pixel on)
const int digits[11][7] = {
    // 0
    {0b111,0b101,0b101,0b101,0b101,0b101,0b111},
    // 1
    {0b010,0b110,0b010,0b010,0b010,0b010,0b111},
    // 2
    {0b111,0b001,0b001,0b111,0b100,0b100,0b111},
    // 3
    {0b111,0b001,0b001,0b111,0b001,0b001,0b111},
    // 4
    {0b101,0b101,0b101,0b111,0b001,0b001,0b001},
    // 5
    {0b111,0b100,0b100,0b111,0b001,0b001,0b111},
    // 6
    {0b111,0b100,0b100,0b111,0b101,0b101,0b111},
    // 7
    {0b111,0b001,0b001,0b010,0b010,0b010,0b010},
    // 8
    {0b111,0b101,0b101,0b111,0b101,0b101,0b111},
    // 9
    {0b111,0b101,0b101,0b111,0b001,0b001,0b111},
    // :
    {0b000,0b010,0b000,0b000,0b010,0b000,0b000}
};

// Functions declarations
int	moving_items(t_env *env);
t_data chose_exit(t_env *env);
t_data chose_player(t_env *env);

/*
key: Value of button pressed
e: Structure with all the data
Captures a keyboard keystroke by mapping it to the
key (the how is hidden).
*/
int	keyboard_manage(int key, t_env *env)
{
//	The comment below can be removed to print the code of the keys that are pressed on the keyboard. In this way you can configure the references present in global.h
	// printf("%d\n",key);
	if (key == ESC)
	{
		for (int i = 0; i < env->row; i++) free(env->M[i]);
		free(env->M);

		for (int i = 0; i < MAX_IMAGES; i++) {
			if (env->images[i].img.image == NULL)
				break;
			mlx_destroy_image(env->mlx, env->images[i].img.image);
		}

		if(env->mlx_win)
			mlx_destroy_window(env->mlx, env->mlx_win);
		if(env->mlx)
			mlx_destroy_display(env->mlx);
		free(env->mlx);

		write(out, "This seems to be the ESC\n", 26);
		exit(0);
	}
	if (key == RX_ARROW)
	{
		t_pos next_pos;
		next_pos.y = env->pos_p.y; 
		next_pos.x = env->pos_p.x +1;
		env->moves++;
		printf("Mossa n.%d\n", env->moves);
		fflush(stdout);

		if (env->M[next_pos.y][next_pos.x] == '0') {	//se ho uno spazio bianco
			env->M[env->pos_p.y][env->pos_p.x] = env->old_tile;
			env->old_tile = env->M[next_pos.y][next_pos.x];
			env->M[next_pos.y][next_pos.x] = 'P';
			env->pos_p.x = next_pos.x;
			env->pos_p.y = next_pos.y;
		}

		else if (env->M[next_pos.y][next_pos.x] == 'C') {	//se ho una caramella
			system("aplay ./music/level_up.wav >/dev/null 2>&1 &");
			env->M[env->pos_p.y][env->pos_p.x] = env->old_tile;
			env->old_tile = '0';
			env->score++;
			env->M[next_pos.y][next_pos.x] = 'P';
			env->pos_p.x = next_pos.x;
			env->pos_p.y = next_pos.y;
		}

		else if (env->M[next_pos.y][next_pos.x] == 'r' || env->M[next_pos.y][next_pos.x] == 'g' || env->M[next_pos.y][next_pos.x] == 'b') {	//se ho una pozza di colore
			system("aplay ./music/throw_splash.wav >/dev/null 2>&1 &");
			env->M[env->pos_p.y][env->pos_p.x] = env->old_tile;
			env->old_tile = env->M[next_pos.y][next_pos.x];
			env->body_paint = env->M[next_pos.y][next_pos.x];
			env->M[next_pos.y][next_pos.x] = 'P';
			env->pos_p.x = next_pos.x;
			env->pos_p.y = next_pos.y;
		}

		else if (env->M[next_pos.y][next_pos.x] == 'R' || env->M[next_pos.y][next_pos.x] == 'G' || env->M[next_pos.y][next_pos.x] == 'B') {	//se ho una tile colorata
			if (env->M[next_pos.y][next_pos.x] == toupper(env->body_paint)) {
				env->M[env->pos_p.y][env->pos_p.x] = env->old_tile;
				env->old_tile = env->M[next_pos.y][next_pos.x];
				env->M[next_pos.y][next_pos.x] = 'P';
				env->pos_p.x = next_pos.x;
				env->pos_p.y = next_pos.y;
			}
		}

		else if (env->M[next_pos.y][next_pos.x] == 'E') {	//se ho l'uscita
			env->M[env->pos_p.y][env->pos_p.x] = env->old_tile;
			env->old_tile = env->M[next_pos.y][next_pos.x];
			if (env->score == env->coin)
				env->is_over = true;
			env->M[next_pos.y][next_pos.x] = 'P';
			env->pos_p.x = next_pos.x;
			env->pos_p.y = next_pos.y;
		}

	}
	if (key == LX_ARROW)
	{
		t_pos next_pos;
		next_pos.y = env->pos_p.y; 
		next_pos.x = env->pos_p.x -1;
		env->moves++;
		printf("Mossa n.%d\n", env->moves);
		fflush(stdout);

		if (env->M[next_pos.y][next_pos.x] == '0') {	//se ho uno spazio bianco
			env->M[env->pos_p.y][env->pos_p.x] = env->old_tile;
			env->old_tile = env->M[next_pos.y][next_pos.x];
			env->M[next_pos.y][next_pos.x] = 'P';
			env->pos_p.x = next_pos.x;
			env->pos_p.y = next_pos.y;
		}

		else if (env->M[next_pos.y][next_pos.x] == 'C') {	//se ho una caramella
			system("aplay ./music/level_up.wav >/dev/null 2>&1 &");
			env->M[env->pos_p.y][env->pos_p.x] = env->old_tile;
			env->old_tile = '0';
			env->score++;
			env->M[next_pos.y][next_pos.x] = 'P';
			env->pos_p.x = next_pos.x;
			env->pos_p.y = next_pos.y;
		}

		else if (env->M[next_pos.y][next_pos.x] == 'r' || env->M[next_pos.y][next_pos.x] == 'g' || env->M[next_pos.y][next_pos.x] == 'b') {	//se ho una pozza di colore
			system("aplay ./music/throw_splash.wav >/dev/null 2>&1 &");
			env->M[env->pos_p.y][env->pos_p.x] = env->old_tile;
			env->old_tile = env->M[next_pos.y][next_pos.x];
			env->body_paint = env->M[next_pos.y][next_pos.x];
			env->M[next_pos.y][next_pos.x] = 'P';
			env->pos_p.x = next_pos.x;
			env->pos_p.y = next_pos.y;
		}

		else if (env->M[next_pos.y][next_pos.x] == 'R' || env->M[next_pos.y][next_pos.x] == 'G' || env->M[next_pos.y][next_pos.x] == 'B') {	//se ho una tile colorata
			if (env->M[next_pos.y][next_pos.x] == toupper(env->body_paint)) {
				env->M[env->pos_p.y][env->pos_p.x] = env->old_tile;
				env->old_tile = env->M[next_pos.y][next_pos.x];
				env->M[next_pos.y][next_pos.x] = 'P';
				env->pos_p.x = next_pos.x;
				env->pos_p.y = next_pos.y;
			}
		}

		else if (env->M[next_pos.y][next_pos.x] == 'E') {	//se ho l'uscita
			env->M[env->pos_p.y][env->pos_p.x] = env->old_tile;
			env->old_tile = env->M[next_pos.y][next_pos.x];
			if (env->score == env->coin)
				env->is_over = true;
			env->M[next_pos.y][next_pos.x] = 'P';
			env->pos_p.x = next_pos.x;
			env->pos_p.y = next_pos.y;
		}

	}
	if (key == UP_ARROW)
	{
		t_pos next_pos;
		next_pos.y = env->pos_p.y -1; 
		next_pos.x = env->pos_p.x;
		env->moves++;
		printf("Mossa n.%d\n", env->moves);
		fflush(stdout);

		if (env->M[next_pos.y][next_pos.x] == '0') {	//se ho uno spazio bianco
			env->M[env->pos_p.y][env->pos_p.x] = env->old_tile;
			env->old_tile = env->M[next_pos.y][next_pos.x];
			env->M[next_pos.y][next_pos.x] = 'P';
			env->pos_p.x = next_pos.x;
			env->pos_p.y = next_pos.y;
		}

		else if (env->M[next_pos.y][next_pos.x] == 'C') {	//se ho una caramella
			system("aplay ./music/level_up.wav >/dev/null 2>&1 &");
			env->M[env->pos_p.y][env->pos_p.x] = env->old_tile;
			env->old_tile = '0';
			env->score++;
			env->M[next_pos.y][next_pos.x] = 'P';
			env->pos_p.x = next_pos.x;
			env->pos_p.y = next_pos.y;
		}

		else if (env->M[next_pos.y][next_pos.x] == 'r' || env->M[next_pos.y][next_pos.x] == 'g' || env->M[next_pos.y][next_pos.x] == 'b') {	//se ho una pozza di colore
			system("aplay ./music/throw_splash.wav >/dev/null 2>&1 &");
			env->M[env->pos_p.y][env->pos_p.x] = env->old_tile;
			env->old_tile = env->M[next_pos.y][next_pos.x];
			env->body_paint = env->M[next_pos.y][next_pos.x];
			env->M[next_pos.y][next_pos.x] = 'P';
			env->pos_p.x = next_pos.x;
			env->pos_p.y = next_pos.y;
		}

		else if (env->M[next_pos.y][next_pos.x] == 'R' || env->M[next_pos.y][next_pos.x] == 'G' || env->M[next_pos.y][next_pos.x] == 'B') {	//se ho una tile colorata
			if (env->M[next_pos.y][next_pos.x] == toupper(env->body_paint)) {
				env->M[env->pos_p.y][env->pos_p.x] = env->old_tile;
				env->old_tile = env->M[next_pos.y][next_pos.x];
				env->M[next_pos.y][next_pos.x] = 'P';
				env->pos_p.x = next_pos.x;
				env->pos_p.y = next_pos.y;
			}
		}

		else if (env->M[next_pos.y][next_pos.x] == 'E') {	//se ho l'uscita
			env->M[env->pos_p.y][env->pos_p.x] = env->old_tile;
			env->old_tile = env->M[next_pos.y][next_pos.x];
			if (env->score == env->coin)
				env->is_over = true;
			env->M[next_pos.y][next_pos.x] = 'P';
			env->pos_p.x = next_pos.x;
			env->pos_p.y = next_pos.y;
		}

	}
	if (key == DW_ARROW)
	{
		t_pos next_pos;
		next_pos.y = env->pos_p.y +1; 
		next_pos.x = env->pos_p.x;
		env->moves++;
		printf("Mossa n.%d\n", env->moves);
		fflush(stdout);

		if (env->M[next_pos.y][next_pos.x] == '0') {	//se ho uno spazio bianco
			env->M[env->pos_p.y][env->pos_p.x] = env->old_tile;
			env->old_tile = env->M[next_pos.y][next_pos.x];
			env->M[next_pos.y][next_pos.x] = 'P';
			env->pos_p.x = next_pos.x;
			env->pos_p.y = next_pos.y;
		}

		else if (env->M[next_pos.y][next_pos.x] == 'C') {	//se ho una caramella
			system("aplay ./music/level_up.wav >/dev/null 2>&1 &");
			env->M[env->pos_p.y][env->pos_p.x] = env->old_tile;
			env->old_tile = '0';
			env->score++;
			env->M[next_pos.y][next_pos.x] = 'P';
			env->pos_p.x = next_pos.x;
			env->pos_p.y = next_pos.y;
		}

		else if (env->M[next_pos.y][next_pos.x] == 'r' || env->M[next_pos.y][next_pos.x] == 'g' || env->M[next_pos.y][next_pos.x] == 'b') {	//se ho una pozza di colore
			system("aplay ./music/throw_splash.wav >/dev/null 2>&1 &");
			env->M[env->pos_p.y][env->pos_p.x] = env->old_tile;
			env->old_tile = env->M[next_pos.y][next_pos.x];
			env->body_paint = env->M[next_pos.y][next_pos.x];
			env->M[next_pos.y][next_pos.x] = 'P';
			env->pos_p.x = next_pos.x;
			env->pos_p.y = next_pos.y;
		}

		else if (env->M[next_pos.y][next_pos.x] == 'R' || env->M[next_pos.y][next_pos.x] == 'G' || env->M[next_pos.y][next_pos.x] == 'B') {	//se ho una tile colorata
			if (env->M[next_pos.y][next_pos.x] == toupper(env->body_paint)) {
				env->M[env->pos_p.y][env->pos_p.x] = env->old_tile;
				env->old_tile = env->M[next_pos.y][next_pos.x];
				env->M[next_pos.y][next_pos.x] = 'P';
				env->pos_p.x = next_pos.x;
				env->pos_p.y = next_pos.y;
			}
		}

		else if (env->M[next_pos.y][next_pos.x] == 'E') {	//se ho l'uscita
			env->M[env->pos_p.y][env->pos_p.x] = env->old_tile;
			env->old_tile = env->M[next_pos.y][next_pos.x];
			if (env->score == env->coin)
				env->is_over = true;
			env->M[next_pos.y][next_pos.x] = 'P';
			env->pos_p.x = next_pos.x;
			env->pos_p.y = next_pos.y;
		}

	}
	if (key == T)
	{
		env->timer = true;
	}
	return (0);
}

/*
e: Structure with all the data
Implements closing the program when the
cross is pressed in the window.
*/
int	window_cross(t_env *env)
{
	for (int i = 0; i < env->row; i++) free(env->M[i]);
	free(env->M);
	
	for (int i = 0; i < MAX_IMAGES; i++) {
		if (env->images[i].img.image == NULL)
		break;
		mlx_destroy_image(env->mlx, env->images[i].img.image);
	}

	if(env->mlx_win)
		mlx_destroy_window(env->mlx, env->mlx_win);
	if(env->mlx)
		mlx_destroy_display(env->mlx);
	free(env->mlx);
		
	write(out, "Just put a CROSS on it\n", 23);
	exit(0);
}

/*
Comparator function for qsort to sort an array of strings (char *).
qsort passes pointers to the array elements, so the arguments are
actually (const char **). We cast the void pointers accordingly,
dereference them to get the actual char *, and then use strcmp.
*/
int cmpstring(const void *p1, const void *p2) {
    return strcmp(*(const char **)p1, *(const char **)p2);
}

/*
*/
char *get_elapsed_time_str(struct timeval start)
{
    static char buffer[16];
    struct timeval now;
    gettimeofday(&now, NULL);

    int elapsed = (now.tv_sec - start.tv_sec);
    int minutes = elapsed / 60;
    int seconds = elapsed % 60;

    snprintf(buffer, sizeof(buffer), "%02d:%02d", minutes, seconds);
    return buffer;
}

/*
*/
char *get_countdown_str(struct timeval start)
{
    static char buffer[16];
    struct timeval now;
    gettimeofday(&now, NULL);

    int elapsed = now.tv_sec - start.tv_sec;
    int remaining = MAX_TIME - elapsed;

    if (remaining < 0) remaining = 0; // don't go negative

    int minutes = remaining / 60;
    int seconds = remaining % 60;

    snprintf(buffer, sizeof(buffer), "%02d:%02d", minutes, seconds);
    return buffer;
}

/*
*/
int is_valid_char(char c) {
    return (c == 'C' || c == '1' || c == '0' || c == 'E' || c == 'P' || c == 'R' || c == 'G' || c == 'B' || c == 'r' || c == 'g' || c == 'b');
}

/*
*/
int	main(int argc, char *argv[])
{
	//Sanity Check
	if (argc != 2) {
		printf("Usage:\n%s a_map.ber\n", argv[0]);
		fflush(stdout);
		return 1;
	}
    char *filename = argv[1];
    if (strlen(filename) < 4 || strcmp(filename + strlen(filename) - 4, ".ber") != 0) {
        printf("Error: file must have .ber extension\n");
		fflush(stdout);
		return 1;
    }
	if (access(filename, F_OK) != 0) {
        printf("Error: can't open this file\n");
		fflush(stdout);
        return 1;
		//TODO controlla tutti i return con memoria allocata memoria allocata
    }

	//Dichiarazione variabili iniziali
	t_env	env;
	env.M = NULL;
	env.old_tile = '0';
	env.row = 0;
	env.column = -1;
	env.coin = 0;
	env.score = 0;
	env.body_paint = 'r';
	for (int i = 0; i < MAX_IMAGES; i++) {
		env.images[i].img.image = NULL;
	}
	gettimeofday(&env.start_time, NULL);
	env.frame = 0;
	env.timer = false;
	env.is_over = false;
	env.is_transformed = false;
	env.moves = 0;

	//Inizializzo variabili + Sanity check mappa
	FILE *fp = fopen(filename, "r");
    if (!fp) {
		printf("Error: can't open this file\n");
		fflush(stdout);
        return 1;
    }
	char tmp_buffer[MAX_LINE_LEN];
    while (fgets(tmp_buffer, MAX_LINE_LEN, fp)) {
        int len = strlen(tmp_buffer);
        if (tmp_buffer[len - 1] == '\n') {
            tmp_buffer[len - 1] = '\0';
            len--;
        }
        if (env.column == -1)
            env.column = len;
        else if (len != env.column) {
            printf("Error: map is not squared\n");
			fflush(stdout);
            fclose(fp);
            return 1;
        }
        env.row++;
    }
	if (env.row < MIN_SIZE || env.column < MIN_SIZE) {
        printf("Error: Map must be at least %dx%d (found %dx%d)\n", MIN_SIZE, MIN_SIZE, env.row, env.column);
		fflush(stdout);
        fclose(fp);
        return 1;
    }
	// Inizzializzo mappa + Sanity check mappa
    env.M = malloc(env.row * sizeof(char *));
    if (!env.M) {
        printf("Error: Memory allocation\n");
		fflush(stdout);
        fclose(fp);
        return 1;
    }
    for (int i = 0; i < env.row; i++) {
        env.M[i] = malloc((env.column + 1) * sizeof(char));
        if (!env.M[i]) {
            printf("Error: Memory allocation\n");
			fflush(stdout);
            for (int j = 0; j < i; j++) free(env.M[j]);
            free(env.M);
            fclose(fp);
            return 1;
        }
    }
	rewind(fp);
	int countE = 0, countP = 0;
    for (int r = 0; r < env.row; r++) {
        fgets(tmp_buffer, MAX_LINE_LEN, fp);
        tmp_buffer[strcspn(tmp_buffer, "\n")] = '\0';
        strcpy(env.M[r], tmp_buffer);		//Riempie la env.Mappa-Matrice
        for (int c = 0; c < env.column; c++) {
            char ch = env.M[r][c];
            if (!is_valid_char(ch)) {
                printf("Error: Invalid character '%c' at position (%d,%d)\n", ch, r, c);
				fflush(stdout);
				for (int i = 0; i < env.row; i++) free(env.M[i]);
				free(env.M);
				fclose(fp);
				return 1;
			}
            if (ch == 'C') env.coin++;
            else if (ch == 'E') {
				countE++;
				env.pos_e.x = c;
				env.pos_e.y = r;
			}
			else if (ch == 'P') {
				countP++;
				env.pos_p.x = c;
				env.pos_p.y = r;
			}
            if (r == 0 || r == env.row - 1 || c == 0 || c == env.column - 1) {
                if (ch != '1') {
                    printf("Error: Invalid border at position (%d,%d), found '%c'\n", r, c, ch);
					fflush(stdout);
					for (int i = 0; i < env.row; i++) free(env.M[i]);
					free(env.M);
					fclose(fp);
					return 1;
				}
            }
        }
    }
	if (countE != 1) {
		printf("Error: There must be exactly 1 'E' (%d found)\n", countE);
		fflush(stdout);
		for (int i = 0; i < env.row; i++) free(env.M[i]);
		free(env.M);
		fclose(fp);
		return 1;
	}
    if (countP != 1) {
		printf("Error: There must be exactly 1 'P' (%d found)\n", countP);
		fflush(stdout);
		for (int i = 0; i < env.row; i++) free(env.M[i]);
		free(env.M);
		fclose(fp);
		return 1;
	}
	fclose(fp);
	//// DEBUG ////
    // printf("Matrice (%dx%d):\n", env.row, env.column);
    // for (int i = 0; i < env.row; i++) {
    //     for (int j = 0; j < env.column; j++) {
    //         putchar(env.M[i][j]);
    //     }
    //     putchar('\n');
    // }
    // printf("coin-score (%d-%d):\n", env.coin,env.score);
    // printf("Sei in (%d-%d):\n", env.pos_p.x,env.pos_p.y);
	// fflush(stdout);

	// TODO Availability check degli item collezzionabili e uscita sulla mappa

	//Inizializzo mlx
	env.mlx = mlx_init();
	env.mlx_win = mlx_new_window(env.mlx, env.column * TILE_SIZE, env.row * TILE_SIZE, "Sp(aghetti)lash");

	//Carico i file xpm dalla memoria
    int countI = 0;
	struct dirent *entry;
    DIR *dp = opendir("./textures");
    if (!dp) {
        printf("Error: Opening images folder fail");
		fflush(stdout);
		for (int i = 0; i < env.row; i++) free(env.M[i]);
		free(env.M);
		if(env.mlx_win)
			mlx_destroy_window(env.mlx, env.mlx_win);
		if(env.mlx)
			mlx_destroy_display(env.mlx);
		free(env.mlx);
		return 1;
    }

	char *file_names[MAX_IMAGES];
	while ((entry = readdir(dp)) != NULL) {
		if (strstr(entry->d_name, ".xpm")) {  // considera solo file che contengono '.xpm'
			//// DEBUG ////
			// printf("Leggo %s\n",entry->d_name);
			// fflush(stdout);

			char path[512];
			snprintf(path, sizeof(path), "%s/%s", "./textures", entry->d_name);
			if (access(path, R_OK) == 0) {
				file_names[countI] = strdup(entry->d_name);
				countI++;
			} else {
				printf("Error: Opening images file: %s\n", path);
				fflush(stdout);
				for (int i = 0; i < countI; i++) {
					free(file_names[i]); // libero la strdup
				}
				closedir(dp);
				for (int i = 0; i < env.row; i++) free(env.M[i]);
				free(env.M);
				if(env.mlx_win)
					mlx_destroy_window(env.mlx, env.mlx_win);
				if(env.mlx)
					mlx_destroy_display(env.mlx);
				free(env.mlx);
				return 1;
			}
		}
	}
	closedir(dp);

	qsort(file_names, countI, sizeof(char *), cmpstring);
	for (int i = 0; i < countI; i++) {
		char path[512];
		snprintf(path, sizeof(path), "%s/%s", "./textures", file_names[i]);

		t_data img;
		int width, height;
		img.image = mlx_xpm_file_to_image(env.mlx, (char *)path, &width, &height);
		img.addr = mlx_get_data_addr(img.image, &img.bits_per_pixel, &img.line_length, &img.endian);
		//TODO dovrei controllare che non ci siano errori durante la lettura del file .xpm
		strncpy(env.images[i].name, file_names[i], MAX_NAME_LEN - 1);
		env.images[i].name[MAX_NAME_LEN - 1] = '\0';
		env.images[i].img = img;

		free(file_names[i]); // libero la strdup
	}
	//// DEBUG ////
	// printf("Numero di immagini caricate: %d\n", countI);
	// for (int i = 0; i < countI; i++) {
	// 	printf("Immagine %d: %s\n", i + 1, env.images[i].name);
	// }
	// fflush(stdout);


	mlx_hook(env.mlx_win, 17, 0, window_cross, &env);			//Capture events from window
	mlx_key_hook(env.mlx_win, keyboard_manage, &env);			//Capture events from keyboard
	mlx_loop_hook(env.mlx, moving_items, &env);

	mlx_loop(env.mlx);
	return (0);
}

/*
*/
int	moving_items(t_env *env) {
	if(env->frame >= 300) env->frame=0;
	env->frame++;
	char *time_string;
	if(env->timer){
		time_string = get_countdown_str(env->start_time);
		if ((strcmp(time_string, "00:00") == 0 && env->is_over) || env->is_transformed) {
			sleep(4);	// lascio un attimo contemplare la sconfitta

			for (int i = 0; i < env->row; i++) free(env->M[i]);
			free(env->M);

			for (int i = 0; i < MAX_IMAGES; i++) {
				if (env->images[i].img.image == NULL)
					break;
				mlx_destroy_image(env->mlx, env->images[i].img.image);
			}

			if(env->mlx_win)
				mlx_destroy_window(env->mlx, env->mlx_win);
			if(env->mlx)
				mlx_destroy_display(env->mlx);
			free(env->mlx);
			exit(0);
		}
		if (strcmp(time_string, "00:00") == 0){
			env->is_over = true;
			write(out, "GAME OVER!!!\n", 13);
		}
		if (env->is_over == true){
			write(out, "YOU WIN!!!\n", 11);
			system("aplay ./music/gyarados.wav >/dev/null 2>&1 &");
			env->is_transformed = true;
		}
	}
	else {
		time_string = get_elapsed_time_str(env->start_time);
		if (env->is_transformed) {
			sleep(4);	// lascio un attimo contemplare la sconfitta

			for (int i = 0; i < env->row; i++) free(env->M[i]);
			free(env->M);

			for (int i = 0; i < MAX_IMAGES; i++) {
				if (env->images[i].img.image == NULL)
					break;
				mlx_destroy_image(env->mlx, env->images[i].img.image);
			}

			if(env->mlx_win)
				mlx_destroy_window(env->mlx, env->mlx_win);
			if(env->mlx)
				mlx_destroy_display(env->mlx);
			free(env->mlx);
			exit(0);
		}
		if (env->is_over == true){
			write(out, "YOU WIN!!!\n", 11);
			system("aplay ./music/gyarados.wav >/dev/null 2>&1 &");
			env->is_transformed = true;
		}

	}

	// riempio la mappa
	mlx_clear_window(env->mlx,env->mlx_win); //ma prima ripulisco
	t_data img;
	for (int y = 0; y < env->row; y++) {
		for (int x = 0; x < env->column; x++) {
			if(env->M[y][x] == 'C')	//Caramella
				img = env->images[0].img;
			if(env->M[y][x] == '1') //StoneBlock
				img = env->images[15].img;
			if(env->M[y][x] == '0') //Tile_W
				img = env->images[19].img;
			if(env->M[y][x] == 'E') //Pokeball
				img = env->images[14].img;
			if(env->M[y][x] == 'P') //Magikarp
				img = chose_player(env);
			if(env->M[y][x] == 'R') //Tile_R
				img = env->images[18].img;
			if(env->M[y][x] == 'G') //Tile_G
				img = env->images[17].img;
			if(env->M[y][x] == 'B') //Tile_B
				img = env->images[16].img;
			if(env->M[y][x] == 'r') //Paint_R
				img = env->images[13].img;
			if(env->M[y][x] == 'g') //Paint_G
				img = env->images[12].img;
			if(env->M[y][x] == 'b') //Paint_B
				img = env->images[11].img;

			mlx_put_image_to_window(env->mlx, env->mlx_win, img.image, x*64, y*64);
		}
	}

	//Metto il tempo a schermo (deve essere l'ultima cosa)
	int x = 20;
	int y = 20;
	for (int i = 0; time_string[i]; i++) {
		char c = time_string[i];
		int digit = -1;
		if (c >= '0' && c <= '9') digit = c - '0';
		else if (c == ':') digit = 10;
		if (digit != -1) {
			if (digit < 0 || digit > 10){
				printf("Error: functions 'get_countdown_str' or 'get_elapsed_time_str' got a problem in returning time\n");
				fflush(stdout);
				continue;
			}
			for (int row = 0; row < 7; row++) {
				for (int col = 0; col < 5; col++) {
					if ((digits[digit][row] >> (5 - 1 - col)) & 1) {
						for (int i = 0; i < 3; i++) {
							for (int j = 0; j < 3; j++) {
								mlx_pixel_put(env->mlx, env->mlx_win, x + col*4 + i, y + row*4 + j, 0xFACF0A);
							}
						}
					}
				}
			}
		}
		x += 17;	// Space among digits
	}
	return 0;
}

/*
*/
t_data chose_player(t_env *env) {
	if(env->pos_p.x == env->pos_e.x && env->pos_p.y == env->pos_e.y && env->score == env->coin){
		return env->images[1].img;
	}
	else if (env->body_paint == 'r') {
		if (env->frame < 100)
			return env->images[10].img;
		else if (env->frame > 200)
			return env->images[7].img;
		else
			return env->images[4].img;
	}
	else if (env->body_paint == 'g') {
		if (env->frame < 100)
			return env->images[9].img;
		else if (env->frame > 200)
			return env->images[6].img;
		else
			return env->images[3].img;
	}
	else { //sottointeso if(env->body_paint == 'B')
		if (env->frame < 100)
			return env->images[8].img;
		else if (env->frame > 200)
			return env->images[5].img;
		else
			return env->images[2].img;
	}
}

