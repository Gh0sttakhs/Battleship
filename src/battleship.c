#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(x) Sleep(x)
#define CLEAR_SCREEN system("cls")
#else
#include <unistd.h>
#define SLEEP_MS(x) usleep((x)*1000)
#define CLEAR_SCREEN system("clear")
#endif

#define ROWS 10
#define COLS 10

#define RESET       "\033[0m"
#define WATER_COLOR "\033[0;34m"
#define HIT_COLOR   "\033[1;31m"
#define MISS_COLOR  "\033[1;36m"
#define SHIP_COLOR  "\033[1;32m"
#define UI_COLOR    "\033[1;33m"
#define BORDER_COLOR "\033[0;37m"

#define WATER '~'
#define HIT   'X'
#define MISS  'O'

void clear_screen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void sleep_ms(int milliseconds) {
    #ifdef _WIN32
        Sleep(milliseconds);
    #else
        usleep(milliseconds * 1000);
    #endif
}

typedef struct {
    char name[20];
    int size;
    int hits;
    char symbol;
} Ship;

int target_stack[200][2];
int stack_top = 0;

void init_board(char board[ROWS][COLS]);
void init_fleet(Ship fleet[]);
void print_single_board(char board[ROWS][COLS], int show_ships);
void print_dual_boards(char bot_board[ROWS][COLS], char player_board[ROWS][COLS]);
int place_ship_random(char board[ROWS][COLS], Ship *ship);
void place_ship_manual(char board[ROWS][COLS], Ship *ship);
int check_fleet_sunk(Ship fleet[]);
void player_turn(char enemy_board[ROWS][COLS], Ship enemy_fleet[]);
void bot_turn(char player_board[ROWS][COLS], Ship player_fleet[]);
void add_target(int r, int c, char board[ROWS][COLS]);
void wait_for_enter();

int main() {
    srand(time(NULL));
    
    char playerBoard[ROWS][COLS];
    char botBoard[ROWS][COLS];
    
    Ship playerFleet[5];
    Ship botFleet[5];

    init_board(playerBoard);
    init_board(botBoard);
    init_fleet(playerFleet);
    init_fleet(botFleet);
    
    printf(UI_COLOR "\n=== BATTLESHIP PRO COMMANDER ===\n" RESET);

    printf("Enemy is positioning their fleet...\n");
    for (int i = 0; i < 5; i++) {
        place_ship_random(botBoard, &botFleet[i]);
    }
    SLEEP_MS(1000);
    printf("Enemy fleet is ready!\n");

    printf("\n" UI_COLOR "--- DEPLOY YOUR FLEET ---" RESET "\n");
    wait_for_enter();
    for (int i = 0; i < 5; i++) {
        place_ship_manual(playerBoard, &playerFleet[i]);
    }

    int game_running = 1;
    while(game_running) {
        CLEAR_SCREEN;
        print_dual_boards(botBoard, playerBoard);
        
        player_turn(botBoard, botFleet);
        if (check_fleet_sunk(botFleet)) {
            CLEAR_SCREEN;
            print_single_board(botBoard, 1);
            printf(SHIP_COLOR "\nVICTORY! You sank the entire enemy fleet!\n" RESET);
            break;
        }
        
        printf("Enemy is firing...\n");
        SLEEP_MS(1000);
        bot_turn(playerBoard, playerFleet);
        if (check_fleet_sunk(playerFleet)) {
            print_single_board(playerBoard, 1);
            printf(HIT_COLOR "\nDEFEAT! Your fleet has been destroyed.\n" RESET);
            break;
        }

        printf(UI_COLOR "\n[PRESS ENTER FOR NEXT ROUND]" RESET);
        int c;
        while ((c = getchar()) != '\n' && c != EOF); 
        getchar();
    }
    
    return 0;
}

void wait_for_enter() {
    printf("[Press ENTER to continue...]");
    getchar();
}

void init_board(char board[ROWS][COLS]) {
    for(int i=0; i<ROWS; i++) {
        for(int j=0; j<COLS; j++) {
            board[i][j] = WATER;
        }
    }
}

void init_fleet(Ship fleet[]) {
    strcpy(fleet[0].name, "Carrier");    fleet[0].size = 5; fleet[0].hits = 0; fleet[0].symbol = 'C';
    strcpy(fleet[1].name, "Battleship"); fleet[1].size = 4; fleet[1].hits = 0; fleet[1].symbol = 'B';
    strcpy(fleet[2].name, "Cruiser");    fleet[2].size = 3; fleet[2].hits = 0; fleet[2].symbol = 'R';
    strcpy(fleet[3].name, "Submarine");  fleet[3].size = 3; fleet[3].hits = 0; fleet[3].symbol = 'S';
    strcpy(fleet[4].name, "Destroyer");  fleet[4].size = 2; fleet[4].hits = 0; fleet[4].symbol = 'D';
}

void print_single_board(char board[ROWS][COLS], int show_ships) {
    printf("    A B C D E F G H I J\n");
    printf("   " BORDER_COLOR "+--------------------+" RESET "\n");

    for(int i=0; i<ROWS; i++) {
        printf(BORDER_COLOR "%2d |" RESET, i+1);
        for(int j=0; j<COLS; j++) {
            char cell = board[i][j];
            if (cell == WATER) printf(WATER_COLOR "%c " RESET, cell);
            else if (cell == HIT) printf(HIT_COLOR "%c " RESET, cell);
            else if (cell == MISS) printf(MISS_COLOR "%c " RESET, cell);
            else { 
                if (show_ships) printf(SHIP_COLOR "%c " RESET, cell);
                else printf(WATER_COLOR "%c " RESET, WATER);
            }
        }
        printf(BORDER_COLOR "|" RESET "\n");
    }
    printf("   " BORDER_COLOR "+--------------------+" RESET "\n");
}

void print_dual_boards(char bot_board[ROWS][COLS], char player_board[ROWS][COLS]) {
    printf("\n");
    printf(HIT_COLOR    "        ENEMY WATERS                  " RESET);
    printf(SHIP_COLOR   "         YOUR FLEET         \n" RESET);
    
    printf("    A B C D E F G H I J                   A B C D E F G H I J\n");
    
    printf("   " BORDER_COLOR "+--------------------+" RESET "                " BORDER_COLOR "+--------------------+" RESET "\n");

    for(int i=0; i<ROWS; i++) {
        printf(BORDER_COLOR "%2d |" RESET, i+1); 
        for(int j=0; j<COLS; j++) {
            char cell = bot_board[i][j];
            if (cell == WATER) printf(WATER_COLOR "%c " RESET, cell);
            else if (cell == HIT) printf(HIT_COLOR "%c " RESET, cell);
            else if (cell == MISS) printf(MISS_COLOR "%c " RESET, cell);
            else printf(WATER_COLOR "%c " RESET, WATER);
        }
        printf(BORDER_COLOR "|" RESET);

        printf("             ");

        printf(BORDER_COLOR "%2d |" RESET, i+1);
        for(int j=0; j<COLS; j++) {
            char cell = player_board[i][j];
            if (cell == WATER) printf(WATER_COLOR "%c " RESET, cell);
            else if (cell == HIT) printf(HIT_COLOR "%c " RESET, cell);
            else if (cell == MISS) printf(MISS_COLOR "%c " RESET, cell);
            else printf(SHIP_COLOR "%c " RESET, cell);
        }
        printf(BORDER_COLOR "|" RESET);
        printf("\n");
    }
    printf("   " BORDER_COLOR "+--------------------+" RESET "                " BORDER_COLOR "+--------------------+" RESET "\n");
}

int place_ship_random(char board[ROWS][COLS], Ship *ship) {
    int placed = 0;
    while (!placed) {
        int row = rand() % 10;
        int col = rand() % 10;
        int dir = rand() % 2; 

        if (dir == 0 && col + ship->size > 10) continue;
        if (dir == 1 && row + ship->size > 10) continue;

        int collision = 0;
        for (int k = 0; k < ship->size; k++) {
            if (dir == 0) { if (board[row][col + k] != WATER) collision = 1; }
            else          { if (board[row + k][col] != WATER) collision = 1; }
        }
        if (collision) continue;

        for (int k = 0; k < ship->size; k++) {
            if (dir == 0) board[row][col + k] = ship->symbol;
            else          board[row + k][col] = ship->symbol;
        }
        placed = 1;
    }
    return 1;
}

void place_ship_manual(char board[ROWS][COLS], Ship *ship) {
    int placed = 0;
    while (!placed) {
        print_single_board(board, 1);
        printf(UI_COLOR "\nDeploying: %s (Size: %d)\n" RESET, ship->name, ship->size);
        printf("Enter starting coordinate (e.g., A 1): ");
        
        char colChar;
        int rowNum;
        if (scanf(" %c %d", &colChar, &rowNum) != 2) {
             printf(HIT_COLOR "Invalid format! Try A 1.\n" RESET);
             while(getchar() != '\n');
             continue;
        }
        
        int col = toupper(colChar) - 'A';
        int row = rowNum - 1;

        printf("Enter direction (0 = Horizontal, 1 = Vertical): ");
        int dir;
        scanf("%d", &dir);
        while(getchar() != '\n');

        if (row < 0 || row > 9 || col < 0 || col > 9) {
            printf(HIT_COLOR "Coordinates out of bounds!\n" RESET);
            continue;
        }
        if (dir == 0 && col + ship->size > 10) {
            printf(HIT_COLOR "Ship goes out of bounds horizontally!\n" RESET);
            continue;
        }
        if (dir == 1 && row + ship->size > 10) {
            printf(HIT_COLOR "Ship goes out of bounds vertically!\n" RESET);
            continue;
        }

        int collision = 0;
        for (int k = 0; k < ship->size; k++) {
            if (dir == 0) { if (board[row][col + k] != WATER) collision = 1; }
            else          { if (board[row + k][col] != WATER) collision = 1; }
        }
        if (collision) {
            printf(HIT_COLOR "Cannot place here! Overlaps with another ship.\n" RESET);
            continue;
        }

        for (int k = 0; k < ship->size; k++) {
            if (dir == 0) board[row][col + k] = ship->symbol;
            else          board[row + k][col] = ship->symbol;
        }
        placed = 1;
        printf(SHIP_COLOR "Ship deployed successfully!\n" RESET);
    }
}

void player_turn(char enemy_board[ROWS][COLS], Ship enemy_fleet[]) {
    int valid_shot = 0;
    while (!valid_shot) {
        printf("\nFire at coordinates (e.g., A 5): ");
        char c_char; int r_num;
        scanf(" %c %d", &c_char, &r_num);
        
        int c = toupper(c_char) - 'A';
        int r = r_num - 1;

        if (r < 0 || r > 9 || c < 0 || c > 9) {
            printf(HIT_COLOR "Invalid coordinates!\n" RESET);
            continue;
        }

        if (enemy_board[r][c] == HIT || enemy_board[r][c] == MISS) {
            printf(UI_COLOR "Already fired there!\n" RESET);
            continue;
        }

        char target = enemy_board[r][c];

        if (target != WATER) {
            enemy_board[r][c] = HIT; 
            printf(HIT_COLOR "\n>>> BOOM! DIRECT HIT! <<<\n" RESET);
            
            for (int i = 0; i < 5; i++) {
                if (enemy_fleet[i].symbol == target) {
                    enemy_fleet[i].hits++;
                    if (enemy_fleet[i].hits == enemy_fleet[i].size) {
                        printf(HIT_COLOR ">>> YOU SANK THE %s! <<<\n" RESET, enemy_fleet[i].name);
                    }
                    break;
                }
            }
        } else {
            printf(MISS_COLOR "\n>>> SPLASH! Missed! <<<\n" RESET);
            enemy_board[r][c] = MISS;
        }
        valid_shot = 1;
    }
}

void add_target(int r, int c, char board[ROWS][COLS]) {
    if (r < 0 || r >= 10 || c < 0 || c >= 10) return;
    if (board[r][c] == HIT || board[r][c] == MISS) return;
    
    for (int i = 0; i < stack_top; i++) {
        if (target_stack[i][0] == r && target_stack[i][1] == c) return;
    }

    target_stack[stack_top][0] = r;
    target_stack[stack_top][1] = c;
    stack_top++;
}

void bot_turn(char player_board[ROWS][COLS], Ship player_fleet[]) {
    int r, c;
    
    printf("Enemy is thinking...\n");
    sleep_ms(1500);

    int found_target = 0;
    while (stack_top > 0) {
        stack_top--; 
        r = target_stack[stack_top][0];
        c = target_stack[stack_top][1];

        if (player_board[r][c] != HIT && player_board[r][c] != MISS) {
            found_target = 1;
            break;
        }
    }

    if (!found_target) {
        do {
            r = rand() % 10;
            c = rand() % 10;
        } while (player_board[r][c] == HIT || player_board[r][c] == MISS);
    }

    printf("Enemy fires at %c %d...", 'A' + c, r + 1);
    
    char target = player_board[r][c];

    if (target != WATER) {
        printf(HIT_COLOR " IT'S A HIT!\n" RESET);
        player_board[r][c] = HIT;
        
        add_target(r - 1, c, player_board);
        add_target(r + 1, c, player_board);
        add_target(r, c - 1, player_board);
        add_target(r, c + 1, player_board);

        for (int i=0; i<5; i++) {
            if (player_fleet[i].symbol == target) {
                player_fleet[i].hits++;
                if (player_fleet[i].hits == player_fleet[i].size) {
                    printf(HIT_COLOR "Enemy SANK your %s!\n" RESET, player_fleet[i].name);
                    stack_top = 0;
                }
            }
        }
    } else {
        printf(MISS_COLOR " They missed.\n" RESET);
        player_board[r][c] = MISS;
    }
}

int check_fleet_sunk(Ship fleet[]) {
    for (int i = 0; i < 5; i++) {
        if (fleet[i].hits < fleet[i].size) return 0; 
    }
    return 1; 
}