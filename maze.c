#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_SIZE 50
#define L 0 // leva hranice
#define R 1 // prava hranice
#define UD 2 // horni/dolni hranice

typedef struct {
    int rows;
    int cols;
    unsigned char *cells;
} Map;

// konstruktor
void map_ctor(Map *map) {
    map->rows = 0;
    map->cols = 0;
    map->cells = NULL;
}

// funkce "load_map" nacte mapu ze souboru "file_name" do struktury "map"
bool load_map(Map *map, char file_name[]) {
    // otevru soubor "file_name" pomoci funkce "fopen" s parametrem "r" pro cteni
    FILE *file;
    file = fopen(file_name, "r");
    // v pripade, ze se soubor nepovede otevrit, vypisu chybove hlaseni
    if (file == NULL) {
        fprintf(stderr, "Couldn't find the %s file.\n", file_name);
        return false;
    }

    char buff[MAX_SIZE];
    map->rows = 0;
    map->cols = 0;
    int idx = 0;
    // cyklem while projedu kazdy radek souboru a ulozim jej do promenne "buff"
    while (fgets(buff, sizeof(buff), file) != NULL) {
        // na prvnim radku zjistim pocet sloupcu a ulozim jej do promenne "map->cols"
        if (map->rows == 0) {
            map->cols = buff[2] - 48;
            map->rows++;
            continue;
        }
        // za kazdy dalsi radek souboru zvetsim velikost promenne "map->cells" o pocet sloupcu pomoci funkce "realloc"
        map->cells = (unsigned char*) realloc(map->cells, map->rows * map->cols);
        for (int i = 0; buff[i] != '\0'; i++) {
            if (buff[i] >= '0' && buff[i] <= '7') {
                // kazdou platnou cislici v promenne "buff" pridam do promenne "map->cells"
                map->cells[idx] = (unsigned char) buff[i];
                idx++;
            }
        }
        map->rows++;
    }
    // jeden radek musim odecist, protoze obsahuje pouze informace o poctu radku a sloupcu
    map->rows--;
    fclose(file);
    return true;
}

// destruktor
void map_dtor(Map *map) {
    // promennou "map->cells" uvolnim z pameti
    free(map->cells);
    map->cells = NULL;
    map->rows = 0;
    map->cols = 0;
}

// funkce vraci index radku a sloupce v promenne "map.cells"
int get_index(Map *map, int r, int c) {
    return (r - 1) * map->cols + c - 1;
}

// funkce prevede index promenne "map.cells" na radek
int get_row(Map *map, int idx) {
    return idx / map->cols + 1;
}

// funkce prevede index promenne "map.cells" na sloupec
int get_col(Map *map, int idx) {
    return idx % map->cols + 1;
}

// funkce vraci, zda dana stena je hranice, ci neni
bool isborder(Map *map, int r, int c, int border) {
    // zjistim si, jakou hodnotu ma dany radek a sloupec pomoci funkce "get_index"
    int pos_value = map->cells[get_index(map, r, c)] - 48;
    // pokud je hodnota vetsi nebo rovna 4, horni/spodni stena je hranice
    if (border == UD && pos_value >= 4) {
        return true;
    }
    // pokud je hodnota licha, leva stena je hranice
    if (border == L && pos_value % 2 == 1) {
        return true;
    }
    // pokud je hodnota 2, 3, 6 nebo 7, prava strana je hranice
    if (border == R && (pos_value == 2 || pos_value == 3 || pos_value == 6 || pos_value == 7)) {
        return true;
    }
    return false;
};

// funkce vrati, kterou stenou se ma pokracovat
int next_border(Map *map, int r, int c, int leftright, int previous) {
    // policka vypadaji stejne na lichem radku a lichem sloupci jako na sudem radku a sudem sloupci
    // stejne tak i policka na lichem radku a sudem sloupci jako na sudem radku a lichem sloupci
    // zjistim si tedy, zda je soucet radku a sloupce sudy nebo lichy
    bool even = (r + c) % 2 == 0;
    bool odd = (r + c) % 2 == 1; 

    // pri prichazeni do policka z leve strany (predchozi hranice byla prava)
    if (previous == R) {
        // plati stejny cyklus pro sudy soucet radku a sloupce pri pravidle prave ruky
        // jako pro lichy soucet radku a sloupce pri pravidle leve ruky
        if ((even && leftright == R) || (odd && leftright == L)) {
            // proto nejdriv zjistuji, zda je hranice na prave strane
            if (!isborder(map, r, c, R)) {
                return R;
            }
            // pokud ano, zkusim horni/dolni stranu
            if (!isborder(map, r, c, UD)) {
                return UD;
            }
            // pokud je i horni/dolni strana hranice, vratim levou stranu
            return L;
        }
        // to stejne plati i pro lichy soucet radku a sloupce pri pravidle prave ruky
        // a sudy soucet radku a sloupce pri pravidle leve ruky
        // jenom musim prehodit poradi z "prava, horni/dolni, leva" na "horni/dolni, prava, leva"
        if ((odd && leftright == R) || (even && leftright == L)) {
            if (!isborder(map, r, c, UD)) {
                return UD;
            }
            if (!isborder(map, r, c, R)) {
                return R;
            }
            return L;
        }
    }
    // pro prichod z prave strany (predchozi hranice byla leva) plati stejne pravidla
    if (previous == L) {
        // akorat je pro sudy soucet radku a sloupce pri pravidle prave ruky
        // a lichy soucet radku a sloupce pri pravidle leve ruky cyklus "horni/dolni, leva, prava" 
        if ((even && leftright == R) || (odd && leftright == L)) {
            if (!isborder(map, r, c, UD)) {
                return UD;
            }
            if (!isborder(map, r, c, L)) {
                return L;
            }
            return R;
        }
        // zde znovu cyklus jen prehodim na "leva, horni/dolni, prava" namisto "horni/dolni, leva, prava"
        if ((odd && leftright == R) || (even && leftright == L)) {
            if (!isborder(map, r, c, L)) {
                return L;
            }
            if (!isborder(map, r, c, UD)) {
                return UD;
            }
            return R;
        }
    }
    // stejne pravidla jsou i pro prichod z horni/spodni strany
    if (previous == UD) {
        // zde je cyklus "leva, prava, horni/dolni"
        if ((even && leftright == R) || (odd && leftright == L)) {
            if (!isborder(map, r, c, L)) {
                return L;
            }
            if (!isborder(map, r, c, R)) {
                return R;
            }
            return UD;
        }
        // zde naopak "prava, leva, horni/dolni"
        if ((odd && leftright == R) || (even && leftright == L)) {
            if (!isborder(map, r, c, R)) {
                return R;
            }
            if (!isborder(map, r, c, L)) {
                return L;
            }
            return UD;
        }
    }
    return -1;
}

// funkce vraci, kterou stenou se ma zacit
int start_border(Map *map, int r, int c, int leftright) {
    // pokud zacinam na leve stene, vratim vysledek funkce "next_border" s parametrem predchozi hranice R (prava)
    if (c == 1) {
        return next_border(map, r, c, leftright, R);
    }
    // pokud zacinam na leve stene, vratim vysledek funkce "next_border" s parametrem L (leva)
    if (c == map->cols) {
        return next_border(map, r, c, leftright, L);
    }
    // pokud zacinam na horni/dolni stene, vratim vysledek funkce "next_border" s parametrem UD (horni/dolni)
    if (r == 1 || r == map->rows) {
        return next_border(map, r, c, leftright, UD);
    }
    return -1;
};

// funkce posune aktualni pozici podle zadane hranice
void move(int pos[], int border) {
    // pokud se presouvam pres levou stenu, uberu sloupec o 1
    if (border == L) {
        pos[1] -= 1;
        return;
    }
    // pokud pres pravou, sloupec prictu o 1
    if (border == R) {
        pos[1] += 1;
        return;
    }
    if (border == UD) {
        // pokud pres horni/dolni, zjistim, zda jsem na sudem nebo lichem souctu radku a sloupce
        // na lichem souctu zvysim radek o 1 
        if ((pos[0] + pos[1]) % 2 == 1) {
            pos[0] += 1;
            return;
        }
        // na sudem souctu snizim radek o 1
        pos[0] -= 1;
    }
}

// funkce zkontroluje, zda je hra u konce
bool check_end(Map *map, int r, int c) {
    // pokud se nachazim na policku mimo mapu, funkce vrati "true"
    if (r > map->rows || r < 1 || c > map->cols || c < 1) {
        return true;
    }
    return false;
}

// funkce vypise pomocne informace
void help() {
    printf("-----HELP-----\n\n");
    printf("Before you run the program, you need to create a txt file with a map following the rules:\n");
    printf("1) Only acceptable characters are space, enter and nubmers 0-7\n");
    printf("2) All rows must be the same length, up to 50 characters\n");
    printf("3) There have to be spaces between each number\n");
    printf("\nHow to run the program:\n");
    printf("1) ./maze --test file.txt\n");
    printf("Tests if the file you selected has the correct format. Prints Valid if it's correct, otherwise prints Invalid.\n");
    printf("2) ./maze --rpath R C file.txt\n");
    printf("R is the starting row, C the starting column.\n");
    printf("Prints all the coordinates (row, col) leading out of the maze following the right hand rule.\n");
    printf("3) ./maze --lpath R C file.txt\n");
    printf("R is the starting row, C the starting column.\n");
    printf("Prints all the coordinates (row, col) leading out of the maze following the left hand rule.\n");
}

// funkce zkontroluje validitu souboru v argumentu
void test(char file_name[]) {
    // otevru soubor "file_name" pomoci funkce "fopen" s parametrem "r" pro cteni
    FILE *file;
    file = fopen(file_name, "r");
    // v pripade, ze se soubor nepovede otevrit, vypisu chybove hlaseni
    if (file == NULL) {
        fprintf(stderr, "Couldn't find the %s file.\n", file_name);
        return;
    }

    char buff[MAX_SIZE];
    int row = 0;
    int cols;
    int rows;
    while (fgets(buff, sizeof(buff), file) != NULL) {
        // v prvnim radku zjistim pozadovany pocet radku a sloupcu
        if (row == 0) {
            rows = buff[0] -48;
            cols = buff[2] -48;
            row++;
            continue;
        }
        int col = 0;
        // v ostatnich radcich kontroluju, zda jsou zadany spravne znaky
        for (int i = 0; buff[i] != '\0'; i++) {
            if (buff[i] < '0' && buff[i] > '7' && buff[i] != ' ' && buff[i] != '\n') {
                printf("Invalid\n");
                return;
            }
            // za kazdou validni cislici pripoctu pocet sloupcu v radku
            if (buff[i] >= '0' && buff[i] <= '7') {
                col++;
            }
        }
        // pokud pocet sloupcu v radku neodpovida predpisu poctu sloupcu na prvnim radku, vypisu "Invalid"
        if (col != cols) {
            printf("Invalid\n");
            return;
        }
        // kazdym cyklem pripisu pocet radku
        row++;
    }
    // jeden radek musim odecist, protoze obsahuje pouze informace o poctu radku a sloupcu
    row--;
    // pokud pocet radku neodpovida predpisu poctu radku na prvnim radku, vypisu Invalid
    if (row != rows) {
        printf("Invalid\n");
        return;
    }

    fclose(file);
    printf("Valid\n");
}

// funkce najde a vypise cestu ven z bludiste pomoci pravidla prave nebo leve ruky
void find_path(char style, int row, int col, char file_name[]) {
    // vytvorim si strukturu map
    Map map;
    map_ctor(&map);
    // pomoci funkce "load_map" strukturu vyplnim hodnotami ze souboru "file_name"
    if (!load_map(&map, file_name)) {
        // v pripade chybneho vyplneni mapy vypisu chybove hlaseni a funkci ukoncim
        fprintf(stderr, "Failed to load map.\n");
        return;
    }
    // pokud ma struktura spatne vyplnene hodnoty, vypisu chybove hlaseni a funkci ukoncim
    if ((row < 1 || row > map.rows) || (col < 1 || col > map.cols)) {
        fprintf(stderr, "Wrong starting coordinates.\n");
        return;
    }

    int leftright = style == 'r' ? R : L;
    int idx = get_index(&map, row, col);
    int current_pos[2] = {get_row(&map, idx), get_col(&map, idx)}; // aktualni pozice na mape (radek, sloupec)
    int next = start_border(&map, row, col, leftright);

    // pomoci cyklu while se budu pohybovat po mape a postupne zjistovat cestu
    // cyklus kazdym opakovanim zkontroluje, zda jsem nedosel na konec mapy
    while (!check_end(&map, current_pos[0], current_pos[1])) {
        printf("%d,%d\n", current_pos[0], current_pos[1]);
        // zjistim si dalsi stenu, pres kterou projit pomoci funkce "next_border"
        next = next_border(&map, current_pos[0], current_pos[1], leftright, next);
        // pokud funkce "next_border" vratila -1 (nenasla prazdnou stenu), vypisu chybove hlaseni a cyklus ukoncim
        if (next == -1) {
            fprintf(stderr, "Path doesn't exist.\n");
            break;
        }
        // presunu se na dalsi policko pomoci funkce "move"
        move(current_pos, next);   
    }

    map_dtor(&map);
}

int main(int argc, char *argv[]) {
    // v pripade, ze program nema argumenty, vypisu chybove hlaseni
    if (argc == 1) {
        fprintf(stderr, "Can't run without arguments.\n");
        return 0;
    }
    // pokud ma program argument "--help", spustim funkci "help" pro vypsani pomocnych informaci
    if (argc == 2 && strcmp(argv[1], "--help") == 0) {
        help();
        return 1;
    }
    // pokud ma program 2 argumenty, z nichz prvni je "--test", spustim funkci "test" s parametrem druheho argumentu (soubor mapy)
    if (argc == 3 && strcmp(argv[1], "--test") == 0) {
        test(argv[2]);
        return 1;
    }
    // v pripade, ze ma program 4 argumenty, rozlisuji, zda je prvni argument "--rpath" nebo "--lpath"
    // pote spustim funkci "find_path" s prislusnymi parametry (r/l, radek, sloupec, soubor mapy)
    if (argc == 5) {
        if (strcmp(argv[1], "--rpath") == 0) {
            find_path('r', (int)argv[2][0] - 48, (int)argv[3][0] - 48, argv[4]);
            return 1;
        }
        if (strcmp(argv[1], "--lpath") == 0) {
            find_path('l', (int)argv[2][0] - 48, (int)argv[3][0] - 48, argv[4]);
            return 1;
        }
    }
    // v ostatnich pripadech vypisu chybove hlaseni
    fprintf(stderr, "Wrong arguments, run ./maze --help for more info.\n");

    return 0;
}