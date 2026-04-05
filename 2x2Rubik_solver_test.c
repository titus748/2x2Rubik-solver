#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <stdint.h>

#define SIZE 65536

char colors[6] = {'w', 'b', 'r', 'y', 'g', 'o'};
char colorsMap[26];
const char ROTATE[9][3] = {"R", "U", "F", "R'", "U'", "F'", "R2", "U2", "F2"};
const int8_t RUF_IDS[3][4] = {{0,2,6,4},{3,4,6,5},{2,1,5,6}};
const int8_t MAP[8][3] = {{15,8,19},{14,22,9},{12,18,3},{13,2,23},{4,16,11},{5,10,21},{7,0,17},{6,20,1}};
int8_t data[7];
int8_t solved[7] = {0,3,6,9,12,15,18};
char input[25];
int8_t ans[11];
int8_t ansSize;
clock_t st, en;
float total, all=0;

struct entry{
    int state;
    int8_t* path;
    int8_t pathSize;
    bool fromInput;
};

struct hash{
    struct entry*** entries;
    int* sizes;
};

//return-> -1:repeat, 1:find, 0:update
int update(struct hash* table, int8_t data[7], int8_t* path, int8_t pathSize, bool fromInput){//update data in hash table
    int state = 0;
    for(int i=0; i<7; i++)state = state*21+data[i];
    int key = state%SIZE;
    int size = table->sizes[key];
    if(size == 0)table->entries[key] = malloc(sizeof(struct entry*));
    else{
        for(int i=0; i<size; i++){
            if(table->entries[key][i]->state == state){
                if(fromInput ^ table->entries[key][i]->fromInput){
                    if(fromInput){
                        for(int j=0; j<pathSize; j++){
                            ans[ansSize++] = path[j];
                        }
                        for(int j=table->entries[key][i]->pathSize-1; j>=0; j--){
                            int8_t p = table->entries[key][i]->path[j];
                            ans[ansSize++] = p<6 ? (p+3)%6:p;
                        }
                    } else{
                        for(int j=0; j<table->entries[key][i]->pathSize; j++){
                            ans[ansSize++] = table->entries[key][i]->path[j];
                        }
                        for(int j=pathSize-1; j>=0; j--){
                            int8_t p = path[j];
                            ans[ansSize++] = p<6 ? (p+3)%6:p;
                        }
                    }
                    return 1;
                }
                return -1;
            }
        }
        table->entries[key] = realloc(table->entries[key], sizeof(struct entry*)*(size+1));
    }
    table->entries[key][size] = malloc(sizeof(struct entry));
    table->entries[key][size]->state = state;
    table->entries[key][size]->pathSize = pathSize;
    table->entries[key][size]->fromInput = fromInput;
    table->entries[key][size]->path = malloc(sizeof(int8_t)*pathSize);
    for(int i=0; i<pathSize; i++){
        table->entries[key][size]->path[i] = path[i];
    }
    table->sizes[key]++;
    return 0;
}

void copyS(int8_t* a, int8_t b, int8_t rotate){
    int8_t bp = b/3*3;
    int8_t bd = (b + rotate)%3;
    *a = bp + bd;
}

void rotate_RF(int8_t s[7], int type, const int8_t IDS[4]){
    int8_t t;
    if(type == 0){
        copyS(&t, s[IDS[3]], 2);
        copyS(&s[IDS[3]], s[IDS[2]], 1);
        copyS(&s[IDS[2]], s[IDS[1]], 2);
        copyS(&s[IDS[1]], s[IDS[0]], 1);
        s[IDS[0]] = t;
    } else if(type == 1){
        copyS(&t, s[IDS[0]], 1);
        copyS(&s[IDS[0]], s[IDS[1]], 2);
        copyS(&s[IDS[1]], s[IDS[2]], 1);
        copyS(&s[IDS[2]], s[IDS[3]], 2);
        s[IDS[3]] = t;
    } else{
        t = s[IDS[0]];
        s[IDS[0]] = s[IDS[2]];
        s[IDS[2]] = t;
        t = s[IDS[1]];
        s[IDS[1]] = s[IDS[3]];
        s[IDS[3]] = t;
    }
}

void rotate_U(int8_t s[7], int type, const int8_t IDS[4]){
    int8_t t;
    if(type == 0){
        t = s[IDS[3]];
        s[IDS[3]] = s[IDS[2]];
        s[IDS[2]] = s[IDS[1]];
        s[IDS[1]] = s[IDS[0]];
        s[IDS[0]] = t;
    } else if(type == 1){
        t = s[IDS[0]];
        s[IDS[0]] = s[IDS[1]];
        s[IDS[1]] = s[IDS[2]];
        s[IDS[2]] = s[IDS[3]];
        s[IDS[3]] = t;
    } else{
        t = s[IDS[0]];
        s[IDS[0]] = s[IDS[2]];
        s[IDS[2]] = t;
        t = s[IDS[1]];
        s[IDS[1]] = s[IDS[3]];
        s[IDS[3]] = t;
    }
}

bool check(int8_t data[7]){//check if solved
    for(int i=0; i<7; i++){
        if(data[i] != i*3)return false;
    }
    return true;
}

bool test_path(int8_t data[7], int8_t* path, int pathSize, struct hash* table, bool fromInput){//get data with path
    int8_t copy[7];
    for(int i=0; i<7; i++){
        copy[i] = data[i];
    }
    for(int i=0; i<pathSize; i++){
        if(path[i]%3 == 0)rotate_RF(copy, path[i]/3, RUF_IDS[0]);
        else if(path[i]%3 == 1)rotate_U(copy, path[i]/3, RUF_IDS[1]);
        else rotate_RF(copy, path[i]/3, RUF_IDS[2]);
    }
    int ret = update(table, copy, path, pathSize, fromInput);
    if(ret == 1)return true;
    if(ret == -1)path[0] = -1;
    return false;
}

void next_paths(int8_t*** paths, int* pathsSize, int* pathSize){//get next paths of the current
    if(*pathsSize == 0){
        for(int i=0; i<9; i++){
            (*paths)[i] = malloc(sizeof(int8_t));
            (*paths)[i][0] = i;
        }
        *pathsSize = 9;
    } else{
        int8_t** res = malloc(sizeof(int8_t*)* *pathsSize*6);
        int resSize = 0;
        for(int i=0; i<*pathsSize; i++){
            if((*paths)[i][0] == -1)continue;
            for(int j=0; j<9; j++){
                if(j%3 == (*paths)[i][*pathSize-1]%3)continue;
                res[resSize] = malloc(sizeof(int8_t)*(*pathSize+1));
                for(int k=0; k<*pathSize; k++){
                    res[resSize][k] = (*paths)[i][k];
                }
                res[resSize++][*pathSize] = j;
            }
            free((*paths)[i]);
        }
        free(*paths);
        *paths = res;
        *pathsSize = resSize;
    }
    (*pathSize)++;
}

void getInput(){// get data from input
    printf("fubdlr(lu,ru,rd,ld): ");
    scanf("%s", input);
    for(int i=0; i<8; i++){
        char c[3];
        for(int j=0; j<3; j++)c[j] = input[MAP[i][j]];
        if(i == 0){
            for(int j=0; j<3; j++){
                colors[j] = c[j];
                colors[j+3] = colorsMap[c[j]-'a']+'a';
            }
            continue;
        }
        int8_t n = 0;
        for(int j=0; j<3; j++){
            if(c[j] == colors[3])n += 12;
            else if(c[j] == colors[4])n += 6;
            else if(c[j] == colors[5])n += 3;
            if(c[j] == colors[3] || c[j] == colors[0])n += j;
        }
        n -= 3;
        data[i-1] = n;
    }
}

void query(){// get answer
    if(check(data)){
        printf("It is solved!\n");
        return;
    }
    int8_t** paths = malloc(sizeof(int8_t*)*9);
    int pathsSize = 0;
    int pathSize = 0;
    struct hash* table = malloc(sizeof(struct hash));
    table->entries = malloc(sizeof(struct entry**)*SIZE);
    table->sizes = calloc(SIZE, sizeof(int));
    ansSize = 0;
    update(table, solved, NULL, 0, false);
    update(table, data, NULL, 0, true);

    // god's number: 11 = 6+5
    for(int i=0; i<6; i++){
        next_paths(&paths, &pathsSize, &pathSize);
        for(int j=0; j<pathsSize; j++){
            if(test_path(data, paths[j], pathSize, table, true))break;
        }
        if(ansSize > 0)break;
        if(i == 5)break;
        for(int j=0; j<pathsSize; j++){
            if(paths[j][0] == -1)continue;
            if(test_path(solved, paths[j], pathSize, table, false))break;
        }
        if(ansSize > 0)break;
    }
    if(ansSize > 0){
        printf("%dsteps: ", ansSize);
        for(int i=0; i<ansSize; i++){
            printf("%s", ROTATE[ans[i]]);
        }
        putchar('\n');
    } else printf("not found\n");
    for(int i=0; i<SIZE; i++){
        if(table->sizes[i] > 0){
            for(int j=0; j<table->sizes[i]; j++){
                free(table->entries[i][j]->path);
                free(table->entries[i][j]);
            }
            free(table->entries[i]);
        }
    }
    free(table->sizes);
    free(table);
    for(int i=0; i<pathSize; i++){
        free(paths[i]);
    }
    free(paths);
}

int main(){
    for(int i=0; i<3; i++){
        int a = colors[i]-'a', b = colors[i+3]-'a';
        colorsMap[a] = b;
        colorsMap[b] = a;
    }
    //getInput();
    for(int t=1; t<=1000; t++){
    	printf("test%d:\n", t);
        for(int i=0; i<7; i++)data[i] = solved[i];
        for(int i=0; i<30; i++){
        	int p = rand()%9;
    	    printf("%s", ROTATE[p]);
        	if(p%3 == 0)rotate_RF(data, p/3, RUF_IDS[0]);
            else if(p%3 == 1)rotate_U(data, p/3, RUF_IDS[1]);
            else rotate_RF(data, p/3, RUF_IDS[2]);
        }
        printf(":\n");
        st = clock();
        query();
        en = clock();
        total = (float)(en-st)/CLOCKS_PER_SEC;
        all += total;
        printf("runtime: %fs\n----------------------------------------\n", total);
    }
    printf("total runtime: %fs\n", all);
    return 0;
}
