#include "stdio.h"
#include "rand.h"

int numbers2048[4][4] = { { } }; // The two dimentional array to store the numbers
int tempUnit2048[4] = {}; // For function move()
unsigned score2048 = 0;
int validity2048 = 0; // The validity of the user's move(0 represent invalidity and 1 represent validity)
char option2048[2] = ""; // The option of the user

void initData(void);
void morge2048(void);
void printNums2048(void);
int isAlive2048(void);
int canEliminate2048(void);
int zeroNum2048(void);
void addrandom2048(void);
void move2048(void);
void merge2048(void);

PUBLIC void start2048Game(int fd_stdin, int fd_stdout) {
    // Specify the rules of the game
    
    clear();
    printf("Welcome to 2048 Game!\n\n\n");
    printf("Control:\n");
    printf("             LEFT: a    RIGHT: d\n");
    printf("             UP:   w    DOWN:  s\n");
    printf("             EXIT: press enter  \n\n\n");
    // Initialize the data
    initData();

    // Initalization
    addrandom2048();
    addrandom2048();
    printNums2048();

    // Turns in loops
    // while (scanf(" %c", &option)) {
    while (read(fd_stdin, option2048, 2)) {
        
        clear();
        printf("Welcome to 2048 Game!!!\n\n\n");
        printf("Controls:\n");
        printf("             LEFT: a    RIGHT: d\n");
        printf("             UP:   w    DOWN:  s\n");
        printf("             EXIT: press enter  \n\n\n");
        // Check if the player is dead
        if (!isAlive2048()) {
            printf("You lose!!!\a\n");
            break;
        }

        morge2048();
        if (validity2048) {
            addrandom2048();
        }
        validity2048 = 0;
        
        printNums2048();
    }
    clear();
}

void morge2048(void) {
    /* Morges(moves and merges) the number blocks */
    
    switch (option2048[0]) {
    case 'w':
        for (int j = 0; j <= 3; j++) {
            for (int i = 0, k = 0; i <= 3; i++) {
                tempUnit2048[k++] = numbers2048[i][j];
            }

            move2048();
            merge2048();
            move2048();

            for (int i = 0, k = 0; i <= 3; i++) {
                numbers2048[i][j] = tempUnit2048[k++];
            }
        }
        break;
    case 'a':
        for (int i = 0; i <= 3; i++) {
            for (int j = 0, k = 0; j <= 3; j++) {
                tempUnit2048[k++] = numbers2048[i][j];
            }

            move2048();
            merge2048();
            move2048();

            for (int j = 0, k = 0; j <= 3; j++) {
                numbers2048[i][j] = tempUnit2048[k++];
            }
        }
        break;
    case 's':
        for (int j = 0; j <= 3; j++) {
            for (int i = 3, k = 0; i >= 0; i--) {
                tempUnit2048[k++] = numbers2048[i][j];
            }

            move2048();
            merge2048();
            move2048();

            for (int i = 3, k = 0; i >= 0; i--) {
                numbers2048[i][j] = tempUnit2048[k++];
            }
        }
        break;
    case 'd':
        for (int i = 0; i <= 3; i++) {
            for (int j = 3, k = 0; j >= 0; j--) {
                tempUnit2048[k++] = numbers2048[i][j];
            }

            move2048();
            merge2048();
            move2048();

            for (int j = 3, k = 0; j >= 0; j--) {
                numbers2048[i][j] = tempUnit2048[k++];
            }
        }
        break;
    default:
        printf("Illegal input!!!\a\n");
    }
    return;
}

void printNums2048(void) {
    /* Prints out the blocks of numbers */
    
    int i, j;
    printf("             ---------------------\n");
    for (i = 0; i <= 3; i++) {
        printf("             |");
        for (j = 0; j <= 3; j++) {
            if (numbers2048[i][j] != 0) {
                printf("%4d|", numbers2048[i][j]);
            }
            else {
                printf("    |");
            }
        }
        printf("\n             |----|----|----|----|\n");
    }
    printf("Score: %d\n", score2048);

    return;
}

int isAlive2048(void) {
    /* Checks if the player is still alive */
    
    if (zeroNum2048()) {
        return 1;
    }
    else if (canEliminate2048()) {
        return 1;
    }
    else
        return 0;
}

int canEliminate2048(void) {
    /* Checks if the number blocks(BLOCLED!!!) can be eliminated */
    
    // Rows
    for (int i = 0; i <= 3; i++) {
        for (int j = 0; j <= 2; j++) {
            if (numbers2048[i][j] == numbers2048[i][j + 1])
                return 1;
        }
    }

    // Columns
    for (int i = 0; i <= 2; i++) {
        for (int j = 0; j <= 3; j++) {
            if (numbers2048[i][j] == numbers2048[i + 1][j])
                return 1;
        }
    }

    return 0;
}

int zeroNum2048(void) {
    /* Counts the number of zeroes in the number block */
    
    int count = 0;
    for (int i = 0; i <= 3; i++) {
        for (int j = 0; j <= 3; j++) {
            if (numbers2048[i][j] == 0)
                count++;
        }
    }
    return count;
}

void addrandom2048(void) {
    /* This function is used to add one 2 into a random empty place */
    
    // The random seed
    // srand2048(int(time(0)));
    srand2048(1234);
    
    int index = rand2048() % zeroNum2048();
    int position = 0;
    for (int i = 0; i <= 3; i++) {
        for (int j = 0; j <= 3; j++) {
            if (numbers2048[i][j] == 0) {
                if (position == index) {
                    numbers2048[i][j] = 2;
                    return;
                }
                position++;
            }
        }
    }
    
    return;
}

void move2048(void) {
    int current = -1, count = 0;
    
    // Positioning the first zero in this unit
    for (int i = 0; i <= 2; i++) {
        if (tempUnit2048[i] == 0) {
            current = i;
            break;
        }
    }

    // No zeroes in this unit
    if (current == -1) {
        return;
    }

    // Move the zero(es) back
    for (; current <= 2; ) {
        if (tempUnit2048[current + 1] == 0) {
            count++;
            current++;
        }
        else {
            validity2048 = 1;
            tempUnit2048[current - count] = tempUnit2048[current + 1];
            current++;
            for (int k = current - count; k <= current; k++)
                tempUnit2048[k] = 0;
        }
    }

    return;
}

void merge2048(void) {
    for (int i = 0; i <= 2; i++) {
        if (tempUnit2048[i] != 0) {
            if (tempUnit2048[i + 1] == tempUnit2048[i]) {
                validity2048 = 1;
                tempUnit2048[i] = tempUnit2048[i + 1] + tempUnit2048[i];
                score2048 += tempUnit2048[i];
                tempUnit2048[i + 1] = 0;
                i++;
            }
        }
        else
            break;
    }

    return;
}

/**
 * Initializes the data
 */
void initData(void) {
    int row = 0, column = 0;
    
    for (row = 0; row < 4; row++) {
        for (column = 0; column < 4; column++) {
            numbers2048[row][column] = 0;
        }
    }
    
    for (row = 0; row < 4; row++) {
        tempUnit2048[row] = 0;
    }

    score2048 = 0;
    validity2048 = 0;
    option2048[2] = "";
}
