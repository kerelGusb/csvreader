#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "hashmap.h"

#define HM_SIZE 4096
#define MAX_ROW_SIZE 8192

typedef enum {
    NOT_VIS,
    VIS,
    DONE
} DFSState;

typedef struct Cell Cell_t;

struct Cell {
    bool is_calculated;

    int row, col;

    int value;

    bool left_is_const;
    int left_value;
    Cell_t* left_arg;

    bool right_is_const;
    int right_value;
    Cell_t* right_arg;

    char oper;

    DFSState state;
};


typedef struct Table {
    Cell_t** cells;
    int rows, cols;
} Table_t;


typedef struct RawTable {
    char*** table;
    int rows, cols;
} RawTable_t;


char** split(const char* s, const char* delims, size_t* size) {
    size_t capacity = 4;
    size_t cnt = 0;

    char** tokens = malloc(capacity * sizeof(char*));
    if (!tokens) return NULL;

    const char* start = s;
    const char* p = s;

    while (true) {
        bool is_delim = false;

        if (*p == '\0') is_delim = true;
        else {
            for (const char* d = delims; *d; ++d) {
                if (*p == *d) {
                    is_delim = true;
                    break;
                }
            }
        }

        if (is_delim) {
            size_t len = p - start;

            char* token = malloc(len + 1);
            if (!token) {
                for (size_t i = 0; i < cnt; ++i) {
                    free(tokens[i]);
                }
                free(tokens);
                return NULL;
            }

            memcpy(token, start, len);
            token[len] = '\0';

            if (cnt == capacity) {
                capacity *= 2;
                char** tmp = realloc(tokens, capacity * sizeof(char*));
                if (!tmp) {
                    for (size_t i = 0; i < cnt; ++i) {
                        free(tokens[i]);
                    }
                    free(tokens);
                    return NULL;
                }
                tokens = tmp;
            }

            tokens[cnt++] = token;
            if (*p == '\0') break;
            start = p + 1;
        }
        p++;
    }

    *size = cnt;

    return tokens;
}


void free_split(char** strs, int cnt) {
    for (int i = 0; i < cnt; ++i)
        free(strs[i]);
    free(strs);
}


RawTable_t* copy_table(FILE* file) {
    RawTable_t* rt = malloc(sizeof(RawTable_t));
    if (!rt) return NULL;

    rt->rows = 0;
    rt->cols = 0;
    rt->table = NULL;

    // counting columns
    char buffer[MAX_ROW_SIZE];

    if (!fgets(buffer, sizeof(buffer), file)) {
        free(rt);
        return NULL;
    }

    buffer[strcspn(buffer, "\n")] = '\0';

    size_t col_cnt;
    char** cols = split(buffer, ",\n", &col_cnt);

    if (!cols) {
        free(rt);
        return NULL;
    }

    rt->cols = col_cnt;
    free_split(cols, col_cnt);

    if (rt->cols < 2) {
        free(rt);
        return NULL;
    }

    fseek(file, 0, SEEK_SET);

    int capacity = 0;

    while (fgets(buffer, sizeof(buffer), file)) {
        if (rt->rows == capacity) {
            capacity = (capacity == 0) ? 4 : capacity * 2;
            char*** tmp = realloc(rt->table, capacity * sizeof(char**));
            if (!tmp) {
                for (int i = 0; i < rt->rows; ++i) {
                    for (int j = 0; j < rt->cols; ++j)
                        free(rt->table[i][j]);

                    free(rt->table[i]);
                }
                free(rt->table);
                free(rt);

                return NULL;
            }

            rt->table = tmp;
        }

        buffer[strcspn(buffer, "\n")] = '\0';

        size_t col_cnt = 0;
        char** row = split(buffer, ",\n", &col_cnt);

        if (!row || col_cnt != (size_t)rt->cols) {
            for (int i = 0; i < rt->rows; ++i) {
                for (int j = 0; j < rt->cols; ++j) {
                    free(rt->table[i][j]);
                }
                free(rt->table[i]);
            }
            free(rt->table);
            free(rt);

            return NULL;
        }

        rt->table[rt->rows++] = row;
    }

    return rt;
}


void rt_free(RawTable_t* rt) {
    for (int i = 0; i < rt->rows; ++i) {
        for (int j = 0; j < rt->cols; ++j) {
            free(rt->table[i][j]);
        }
        free(rt->table[i]);
    }
    free(rt->table);
    free(rt);
}


int fill_hm(Hashmap_t* hm, RawTable_t* rt) {
    for (int i = 1; i < rt->rows; ++i) {
        for (int j = 1; j < rt->cols; ++j) {
            char* col_name = rt->table[0][j];
            if (strcmp(col_name, "") == 0) return 1;

            char* row_name = rt->table[i][0];
            if (strcmp(row_name, "") == 0) return 1;

            int cell_name_len = strlen(col_name) + strlen(row_name);
            char* cell_name = malloc(cell_name_len + 1);

            strcpy(cell_name, col_name);
            strcat(cell_name, row_name);

            // creates unique index for every cell
            // so we can find cell by its name
            hm_insert(hm, cell_name, (i - 1) * (rt->cols - 1) + j - 1);

            free(cell_name);
        }
    }

    return 0;
}

Table_t* init_table(size_t rows, size_t cols) {
    Table_t* t = malloc(sizeof(Table_t));
    if (!t) return NULL;

    t->rows = rows - 1;
    t->cols = cols - 1;

    t->cells = malloc(t->rows * sizeof(Cell_t*));
    if (!t->cells) {
        free(t);
        return NULL;
    }

    for (int i = 0; i < t->rows; ++i) {
        Cell_t* row = calloc(t->cols, sizeof(Cell_t));
        if (!row) {
            for (int j = 0; j < i; ++j) {
                free(t->cells[j]);
            }
            free(t->cells);
            free(t);

            return NULL;
        }

        for (int j = 0; j < t->cols; ++j) {
            row[j].row = i;
            row[j].col = j;
        }

        t->cells[i] = row;
    }

    return t;
}

void table_free(Table_t* t) {
    for (int i = 0; i < t->rows; ++i)
        free(t->cells[i]);
    free(t->cells);
    free(t);
}


bool is_number(const char* s) {
    char* end;
    strtol(s, &end, 10);
    return *end == '\0';
}


int parse_cells(Table_t* t, Hashmap_t* hm, RawTable_t* rt) {
    for (int i = 0; i < t->rows; ++i) {
        for (int j = 0; j < t->cols; ++j) {
            char* raw_cell = rt->table[i + 1][j + 1];

            if (strcmp(raw_cell, "") == 0) return 1;
            
            Cell_t* cell = &t->cells[i][j];

            cell->row = i;
            cell->col = j;

            if (raw_cell[0] == '=') {
                // pattern: =<Cell_name|Const>['+', '-', '*', '/']<Cell_name|Const>
                char* expr = raw_cell + 1;
                char* op = strpbrk(expr, "+-*/");
                if (!op) return 1;

                cell->oper = *op;

                *op = '\0';
                char* left_expr = expr;
                char* right_expr = op + 1;

                if (strcmp(left_expr, "") == 0 || strcmp(right_expr, "") == 0) return 1;

                if (is_number(left_expr)) {
                    cell->left_is_const = true;
                    cell->left_value = atoi(left_expr);

                } else {
                    cell->left_is_const = false;

                    Entry_t* left_entry = hm_get(hm, left_expr);
                    if (!left_entry) return 1;

                    int left_cell_idx = left_entry->value;
                    cell->left_arg = &t->cells[left_cell_idx / t->cols][left_cell_idx % t->cols];
                
                }

                if (is_number(right_expr)) {
                    cell->right_is_const = true;
                    cell->right_value = atoi(right_expr);

                } else {
                    cell->right_is_const = false;

                    Entry_t* right_entry = hm_get(hm, right_expr);
                    if (!right_entry) return 1;

                    int right_cell_idx = right_entry->value;

                    cell->right_arg = &t->cells[right_cell_idx / t->cols][right_cell_idx % t->cols];

                }

                cell->is_calculated = false;

            } else {
                char* end;
                int val = strtol(raw_cell, &end, 10);
                if (*end != '\0') return 1;

                cell->value = val;
                cell->is_calculated = true;
            }
        }
    }

    return 0;
}

typedef struct StackItem {
    Cell_t* cell;
    int stage;
    // 0 - no children processed
    // 1 - both children processed
} StackItem_t;


int dfs(Cell_t* start) {
    int stack_cap = 4;
    StackItem_t* stack = malloc(sizeof(StackItem_t) * stack_cap); 
    if (!stack) return 1;

    int top = 0;
    stack[top++] = (StackItem_t){start, 0};

    while (top > 0) {
        if (top + 3 >= stack_cap) {
            stack_cap *= 2;
            StackItem_t* tmp = realloc(stack, sizeof(StackItem_t) * stack_cap);
            if (!tmp) {
                free(stack);
                return 1;
            }
            stack = tmp;
        }
        StackItem_t item = stack[--top];
        Cell_t* cell = item.cell;

        if (cell->state == DONE)
            continue;

        if (cell->state == VIS && item.stage == 0) {
            free(stack);
            return 2; // cycle detected
        }

        if (item.stage == 0) {
            if (cell->is_calculated) {
                cell->state = DONE;
                continue;
            }

            cell->state = VIS;

            stack[top++] = (StackItem_t){cell, 1};

            if (!cell->left_is_const)
                stack[top++] = (StackItem_t){cell->left_arg, 0};
            if (!cell->right_is_const)
                stack[top++] = (StackItem_t){cell->right_arg, 0};

            continue;
        }

        if (item.stage == 1) {
            int left = cell->left_is_const
                ? cell->left_value
                : cell->left_arg->value;

            int right = cell->right_is_const
                ? cell->right_value
                : cell->right_arg->value;

            switch(cell->oper) {
                case '+': cell->value = left + right; break;
                case '-': cell->value = left - right; break;
                case '*': cell->value = left * right; break;
                case '/':
                    if (right == 0)  {
                        free(stack);
                        return 3; // div by zero
                    }
                    cell->value = left / right;
                    break;
                default:
                    free(stack);
                    return 4; // invalid oper
            }

            cell->is_calculated = true;
            cell->state = DONE;
        }
    }

    free(stack);

    return 0;
}


int calculate_cells(Table_t* t) {
    for (int i = 0; i < t->rows; ++i) {
        for (int j = 0; j < t->cols; ++j) {
            Cell_t* cur_cell = &t->cells[i][j];
            if (!cur_cell->is_calculated) {
                int res = dfs(cur_cell);

                if (res) return res;
            }
        }
    }

    return 0;
}   


void print_table(Table_t *t, RawTable_t *rt) {
    if (!t || !t->cells) {
        printf("Table is NULL\n");
        return;
    }

    for (int j = 0; j < rt->cols; ++j) {
        printf(rt->table[0][j]);
        if (j < rt->cols - 1)
            printf(",");
    }
    printf("\n");

    for (int i = 0; i < t->rows; ++i) {
        printf(rt->table[i + 1][0]);
        printf(",");
        for (int j = 0; j < t->cols; ++j) {
            printf("%d", t->cells[i][j].value);

            if (j < t->cols - 1)
                printf(",");
        }
        printf("\n");
    }
}


int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "ERROR: Wrong input\nUsing: csvreader <filename>");
        return 1;
    }

    int res;

    char* filename = argv[1];
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "ERROR: File does not exist");
        return 1;
    }

    RawTable_t* rt = copy_table(file);
    fclose(file);

    if (!rt) {
        fprintf(stderr, "ERROR: Failed to parse CSV\n");
        return 1;
    }

    // print_raw_table(rt);

    Hashmap_t* hm = hm_init(HM_SIZE);
    res = fill_hm(hm, rt);
    if (res) {
        fprintf(stderr, "ERROR: Empty column or row name\n");
        hm_free(hm);
        rt_free(rt);

        return 1;
    }

    Table_t* t = init_table(rt->rows, rt->cols);
    if (!t) {
        fprintf(stderr, "ERROR: Failed to create table\n");
        hm_free(hm);
        rt_free(rt);

        return 1;
    }

    res = parse_cells(t, hm, rt);
    if (res) {
        fprintf(stderr, "ERROR: Failed to parse values\n");
        table_free(t);
        hm_free(hm);
        rt_free(rt);

        return 1;
    }

    res = calculate_cells(t);   
    if (res) {
        const char* err_msg;
        switch (res) {
            case 1: 
                err_msg = "ERROR: Failed to allocate memory\n";
                break;
            case 2:
                err_msg = "ERROR: Cycle dependency\n";
                break;
            case 3:
                err_msg = "ERROR: Division by zero\n";
                break;
            case 4:
                err_msg = "ERROR: Invalid operation\n";
                break;
            default:
                err_msg = "ERROR: Unknown error\n";
                break;
        }

        fprintf(stderr, "%s", err_msg);
        table_free(t);
        hm_free(hm);
        rt_free(rt);
        
        return 1;
    }

    print_table(t, rt);

    table_free(t);
    hm_free(hm);
    rt_free(rt);

    return 0;
}