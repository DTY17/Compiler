#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

typedef struct
{
    size_t capacity;
    size_t index;
    char **data;
    char **type;
} LEXER;

typedef struct
{
    size_t *lexer_index;
    size_t capacity;
    size_t index;
    char **data;
} TOKEN;

typedef struct
{
    size_t capacity;
    size_t index;
    char **data;
} OPERATION;

typedef struct
{
    size_t capacity;
    size_t index;
    char **data;
} SUB_OPERATION;

typedef struct
{
    size_t capacity;
    size_t index;
    char **name;
    int *data;
} INT;

typedef struct
{
    size_t capacity;
    size_t index;
    char **name;
    char **data;
} STR;

typedef struct
{
    size_t capacity;
    size_t index;
    char **name;
    float *data;
} FLOAT;

typedef struct
{
    size_t capacity;
    size_t index;
    char **name;
    char *data;
} CHAR;

LEXER *lexer_stack;
OPERATION *operation_stack;
INT *int_stack;
STR *str_stack;
FLOAT *float_stack;
CHAR *char_stack;
SUB_OPERATION *sub_operation_stack;

void find_variable(char *data, size_t indx_count, bool init_parse);
char *combine_char(size_t start, size_t end, char *data);
void addChar(char *str, char c);
char *find_variable_data_type(char *variable);
char *space_adjust(char *data);
char *combine_two(const char *data1, const char *data2);
char *operation_concat(const OPERATION *op);
int operation_excute(OPERATION *operation);
int sub_operation_excute(SUB_OPERATION *operation);
void remove_underscores(OPERATION *op);

int is_number(char *s);
int is_operators(char *s);
void bracket_operation();

void test_all_stacks();
void test_char_stack();
void test_str_stack();
void test_float_stack();
void test_int_stack();
void test_sub_operation_stack();
void test_operation_stack();
void test_lexer_stack();

// char **operation_order = {"-","+","*","/"};

void init_lexer_stack()
{
    lexer_stack = malloc(sizeof(LEXER));
    lexer_stack->capacity = 1;
    lexer_stack->index = 0;
    lexer_stack->type = malloc(lexer_stack->capacity * sizeof(char *));
    lexer_stack->data = malloc(lexer_stack->capacity * sizeof(char *));
}

void add_lexer_stack(char *data, char *type)
{
    if (lexer_stack->capacity <= lexer_stack->index)
    {
        lexer_stack->capacity = lexer_stack->capacity * 2;
        lexer_stack->data = realloc(lexer_stack->data, lexer_stack->capacity * sizeof(char *));
        lexer_stack->type = realloc(lexer_stack->type, lexer_stack->capacity * sizeof(char *));
    }
    lexer_stack->data[lexer_stack->index] = strdup(data);
    lexer_stack->type[lexer_stack->index] = strdup(type);
    lexer_stack->index++;
}

void change_lexer_type(size_t i, char *type)
{
    lexer_stack->type[i] = strdup(type);
}

void init_operation_stack()
{
    operation_stack = malloc(sizeof(OPERATION));
    operation_stack->capacity = 1;
    operation_stack->index = 0;
    operation_stack->data = malloc(operation_stack->capacity * sizeof(char *));
}

void add_operetors(char *data)
{
    if (operation_stack->capacity <= operation_stack->index)
    {
        operation_stack->capacity *= 2;
        operation_stack->data = realloc(operation_stack->data, operation_stack->capacity * sizeof(char *));
    }

    operation_stack->data[operation_stack->index] = strdup(data);
    operation_stack->index++;
}

void free_operation_stack(OPERATION *op)
{
    if (!op)
        return;

    for (size_t i = 0; i < op->index; i++)
    {
        free(op->data[i]);
    }
    free(op->data);
    free(op);
    init_operation_stack();
}

void init_operation_sub_stack()
{
    sub_operation_stack = malloc(sizeof(SUB_OPERATION));
    sub_operation_stack->capacity = 1;
    sub_operation_stack->index = 0;
    sub_operation_stack->data = malloc(sub_operation_stack->capacity * sizeof(char *));
}

void add_operetion_sub(char *data)
{
    if (sub_operation_stack->capacity <= sub_operation_stack->index)
    {
        sub_operation_stack->capacity *= 2;
        sub_operation_stack->data = realloc(sub_operation_stack->data, sub_operation_stack->capacity * sizeof(char *));
    }

    sub_operation_stack->data[sub_operation_stack->index] = strdup(data);
    sub_operation_stack->index++;
}

void free_operation_sub_stack(SUB_OPERATION *op)
{
    if (!op)
        return;

    for (size_t i = 0; i < op->index; i++)
    {
        free(op->data[i]);
    }
    free(op->data);
    free(op);
    init_operation_sub_stack();
}

void init_int_stack()
{
    int_stack = malloc(sizeof(INT));
    int_stack->capacity = 1;
    int_stack->index = 0;
    int_stack->data = malloc(int_stack->capacity * sizeof(int));
    int_stack->name = malloc(int_stack->capacity * sizeof(char *));
}

void add_int(int data, char *name)
{
    if (int_stack->capacity <= int_stack->index)
    {
        int_stack->capacity *= 2;
        int_stack->data = realloc(int_stack->data, int_stack->capacity * sizeof(int));
        int_stack->name = realloc(int_stack->name, int_stack->capacity * sizeof(char *));
    }

    int_stack->data[int_stack->index] = data;
    int_stack->name[int_stack->index] = strdup(name);
    int_stack->index++;
}

void change_int(int data, size_t i)
{
    int_stack->data[i] = data;
}

int find_int_by_name(char *name)
{
    for (size_t i = 0; i < int_stack->index; i++)
    {
        if (strcmp(int_stack->name[i], name) == 0)
        {
            return int_stack->data[i];
        }
    }
    return 0;
}

void init_str_stack()
{
    str_stack = malloc(sizeof(STR));
    str_stack->capacity = 1;
    str_stack->index = 0;
    str_stack->data = malloc(str_stack->capacity * sizeof(char *));
    str_stack->name = malloc(str_stack->capacity * sizeof(char *));
}

void add_str(const char *data, const char *name)
{
    if (str_stack->capacity <= str_stack->index)
    {
        str_stack->capacity *= 2;
        str_stack->data = realloc(str_stack->data, str_stack->capacity * sizeof(char *));
        str_stack->name = realloc(str_stack->name, str_stack->capacity * sizeof(char *));
    }

    str_stack->data[str_stack->index] = strdup(data);
    str_stack->name[str_stack->index] = strdup(name);
    str_stack->index++;
}

void init_float_stack()
{
    float_stack = malloc(sizeof(STR));
    float_stack->capacity = 1;
    float_stack->index = 0;
    float_stack->data = malloc(float_stack->capacity * sizeof(float *));
    float_stack->name = malloc(float_stack->capacity * sizeof(char *));
}

void add_float(const float data, const char *name)
{
    if (float_stack->capacity <= float_stack->index)
    {
        float_stack->capacity *= 2;
        float_stack->data = realloc(float_stack->data, float_stack->capacity * sizeof(float));
        float_stack->name = realloc(float_stack->name, float_stack->capacity * sizeof(char *));
    }

    float_stack->data[float_stack->index] = data;
    float_stack->name[float_stack->index] = strdup(name);
    float_stack->index++;
}

void init_char_stack()
{
    char_stack = malloc(sizeof(CHAR));
    char_stack->capacity = 1;
    char_stack->index = 0;
    char_stack->data = malloc(char_stack->capacity * sizeof(char));
    char_stack->name = malloc(char_stack->capacity * sizeof(char *));
}

void add_char(const char data, const char *name)
{
    if (char_stack->capacity <= char_stack->index)
    {
        char_stack->capacity *= 2;
        char_stack->data = realloc(char_stack->data, char_stack->capacity * sizeof(char));
        char_stack->name = realloc(char_stack->name, char_stack->capacity * sizeof(char *));
    }

    char_stack->data[char_stack->index] = data;
    char_stack->name[char_stack->index] = strdup(name);
    char_stack->index++;
}

bool isString = false;
bool string_ditected = false;
bool isFloat = false;
bool isVariable = false;
bool is_variable_name = false;
bool is_value = false;

bool isChar = false;
bool char_ditected = false;
bool is_in_param = false;
bool is_method = true;

char *variable_Name = "none";
char *var_d_type = "none";
char *data_type = "none";

int index_var_g = 0;
char *type_var_g = "none";

bool declear_var = false;
bool init_var = false;

char *method_name = "none";
char *return_type = "none";

char *pre_text = "none";
char *next_text = "none";

void addChar(char *str, char c)
{
    size_t len = strlen(str);
    str[len] = c;
    str[len + 1] = '\0';
}

void tokenization(FILE *file)
{
    char *t = malloc(1);
    t[0] = '\0';
    int ch = fgetc(file);
    while (ch != EOF)
    {
        if (ch == '\n')
        {
            // skip \n
        }
        else if (ch == ';')
        {
            add_lexer_stack(t, "init");
            // space_adjust(t);
            free(t);
            t = malloc(1);
            t[0] = '\0';
        }
        else
        {
            t = realloc(t, strlen(t) + 2);
            size_t len = strlen(t);
            t[len] = ch;
            t[len + 1] = '\0';
        }
        ch = fgetc(file);
    }
}

void init_variable()
{
    for (size_t j = 0; j < lexer_stack->index; j++)
    {
        char *new_lexer = space_adjust(lexer_stack->data[j]);
        find_variable(new_lexer, j, true);
    }
}

void add_values_var()
{
    for (size_t j = 0; j < lexer_stack->index; j++)
    {
        char *new_lexer = space_adjust(lexer_stack->data[j]);
        find_variable(new_lexer, j, false);
    }
}

void find_variable(char *data, size_t indx_count, bool init_parse)
{

    bool is_operator = false;
    size_t count = 0;
    for (size_t i = 0; i < strlen(data); i++)
    {
        if (is_value && data[i] == '"')
        {
            isString = !isString;
            string_ditected = true;
            continue;
        }

        if (is_value && data[i] == '\'')
        {
            isChar = !isChar;
            char_ditected = true;
            continue;
        }

        if (!isString && data[i] == '.')
        {
            isFloat = true;
        }

        // method ditect section
        if (!isString && data[i] == '(' && is_variable_name)
        {
            // is_operator = true;
            is_variable_name = false;
            is_in_param = true;
            is_method = true;
        }
        if (!isString && data[i] == ')' && is_variable_name)
        {
            is_in_param = true;
            is_operator = true;
            is_method = true;
        }

        if (!isString && data[i] == '(')
        {
            is_operator = true;
            is_variable_name = false;
        }
        if (!isString && data[i] == ')')
        {
            is_operator = true;
        }
        if (!isString && data[i] == '{' && is_method)
        {
            is_method = true;
        }
        if (!isString && data[i] == '}' && is_method)
        {
            is_method = false;
        }

        // Seperator
        if (data[i] == ' ' && !isString)
        {
            char *txt = combine_char(count, i, data);
            count = i + 1;

            if (
                !isString &&
                (strcmp(txt, "+") == 0 ||
                 strcmp(txt, "-") == 0 ||
                 strcmp(txt, "/") == 0 ||
                 strcmp(txt, "*") == 0 ||
                 strcmp(txt, "(") == 0 ||
                 strcmp(txt, ")") == 0))
            {
                is_operator = true;
            }

            if (isVariable)
            {
                if (is_variable_name)
                {
                    variable_Name = txt;
                    is_variable_name = false;
                    pre_text = txt;
                    continue;
                }
            }

            if (is_value && !is_variable_name)
            {
                add_operetors(txt);
                continue;
            }

            if (strcmp(txt, "=") == 0 && isVariable)
            {
                is_variable_name = false;
                is_value = true;
                pre_text = txt;
                continue;
            }

            if (strcmp(txt, "int") == 0)
            {
                is_variable_name = true;
                isVariable = true;
                data_type = "int";
                init_var = true;
                pre_text = txt;
                continue;
            }
            else if (strcmp(txt, "float") == 0)
            {
                is_variable_name = true;
                isVariable = true;
                data_type = "float";
                init_var = true;
                pre_text = txt;
                continue;
            }
            else if (strcmp(txt, "str") == 0)
            {
                is_variable_name = true;
                isVariable = true;
                data_type = "str";
                init_var = true;
                pre_text = txt;
                continue;
            }
            else if (strcmp(txt, "char") == 0)
            {
                is_variable_name = true;
                isVariable = true;
                data_type = "char";
                init_var = true;
                pre_text = txt;
                continue;
            }
            else
            {
                declear_var = true;
                type_var_g = find_variable_data_type(txt);
                if (strcmp(type_var_g, "int") == 0)
                {
                    variable_Name = txt;
                    isVariable = true;
                    data_type = "int";
                    init_var = true;
                    pre_text = txt;
                    continue;
                }
                else if (strcmp(type_var_g, "float") == 0)
                {
                    isVariable = true;
                    data_type = "float";
                    init_var = true;
                    pre_text = txt;
                    continue;
                }
                else if (strcmp(type_var_g, "str") == 0)
                {
                    isVariable = true;
                    data_type = "str";
                    init_var = true;
                    pre_text = txt;
                    continue;
                }
                else if (strcmp(type_var_g, "char") == 0)
                {
                    isVariable = true;
                    data_type = "char";
                    init_var = true;
                    pre_text = txt;
                    continue;
                }
            }
            pre_text = txt;
        }
    }
    printf("round\n");

    char *txts = combine_char(count, strlen(data), data);
    add_operetors(txts);
    // printf("txts %s\n", txts);
    // printf("is operation %d\n", is_operator);
    if (is_operator)
    {
        operation_concat(operation_stack);
        bracket_operation();
        remove_underscores(operation_stack);
        int v1 = operation_excute(operation_stack);
        char str[32];
        snprintf(str, sizeof(str), "%d", v1);
        txts = str;
    }

    if (string_ditected)
        txts = combine_char(count + 1, strlen(data) - 1, data);
    if (char_ditected)
        txts = combine_char(count + 1, strlen(data) - 1, data);

    if (!declear_var && init_parse)
    {
        if (strcmp(data_type, "int") == 0)
        {
            change_lexer_type(indx_count, "VAR");
            add_int(atoi(txts), variable_Name);
        }

        if (strcmp(data_type, "str") == 0)
        {
            if (string_ditected && !isString)
            {
                change_lexer_type(indx_count, "VAR");
                add_str(txts, variable_Name);
            }
        }

        if (strcmp(data_type, "float") == 0)
        {
            if (isFloat)
            {
                change_lexer_type(indx_count, "VAR");
                add_float(atof(txts), variable_Name);
            }
        }

        if (strcmp(data_type, "char") == 0)
        {
            if (char_ditected && !isChar)
            {
                change_lexer_type(indx_count, "VAR");
                add_char(txts[0], variable_Name);
            }
        }
        pre_text = txts;
    }
    else if (declear_var && !init_parse)
    {
        if (strcmp(type_var_g, "int") == 0)
        {
            change_lexer_type(indx_count, "VAR");
            change_int(atoi(txts), index_var_g);
        }

        if (strcmp(type_var_g, "str") == 0)
        {
            if (string_ditected && !isString)
            {
                change_lexer_type(indx_count, "VAR");
                add_str(txts, variable_Name);
            }
        }

        if (strcmp(type_var_g, "float") == 0)
        {
            if (isFloat)
            {
                change_lexer_type(indx_count, "VAR");
                add_float(atof(txts), variable_Name);
            }
        }

        if (strcmp(type_var_g, "char") == 0)
        {
            if (char_ditected && !isChar)
            {
                change_lexer_type(indx_count, "VAR");
                add_char(txts[0], variable_Name);
            }
        }
        pre_text = txts;
    }

    free_operation_stack(operation_stack);

    declear_var = false;
    init_var = false;
    char_ditected = false;
    isChar = false;
    isString = false;
    string_ditected = false;
    isFloat = false;
    isVariable = false;
    is_variable_name = false;
    is_value = false;
    is_operator = false;
    data_type = "none";
    variable_Name = "none";
}

char *space_adjust(char *data)
{
    size_t len = strlen(data);
    char *new_data = malloc(len * 2 + 1);
    if (!new_data)
        return NULL;

    size_t j = 0;

    for (size_t i = 0; i < len; i++)
    {
        char current = data[i];
        char next = data[i + 1];

        if (current != ' ' && next == '=')
        {
            new_data[j++] = current;
            new_data[j++] = ' ';
            continue;
        }

        if (current == '=' && next != ' ')
        {
            new_data[j++] = current;
            new_data[j++] = ' ';
            continue;
        }

        if (current != ' ' && next == '(')
        {
            new_data[j++] = current;
            new_data[j++] = ' ';
            continue;
        }

        if (current == '(' && next != ' ')
        {
            new_data[j++] = current;
            new_data[j++] = ' ';
            continue;
        }

        if (current != ' ' && next == ')')
        {
            new_data[j++] = current;
            new_data[j++] = ' ';
            continue;
        }

        if (current == ')' && next != ' ')
        {
            new_data[j++] = current;
            new_data[j++] = ' ';
            continue;
        }

        if (current != ' ' && next == '+')
        {
            new_data[j++] = current;
            new_data[j++] = ' ';
            continue;
        }

        if (current == '+' && next != ' ')
        {
            new_data[j++] = current;
            new_data[j++] = ' ';
            continue;
        }

        if (current != ' ' && next == '-')
        {
            new_data[j++] = current;
            new_data[j++] = ' ';
            continue;
        }

        if (current == '-' && next != ' ')
        {
            new_data[j++] = current;
            new_data[j++] = ' ';
            continue;
        }

        if (current != ' ' && next == '*')
        {
            new_data[j++] = current;
            new_data[j++] = ' ';
            continue;
        }

        if (current == '*' && next != ' ')
        {
            new_data[j++] = current;
            new_data[j++] = ' ';
            continue;
        }

        if (current != ' ' && next == '/')
        {
            new_data[j++] = current;
            new_data[j++] = ' ';
            continue;
        }

        if (current == '/' && next != ' ')
        {
            new_data[j++] = current;
            new_data[j++] = ' ';
            continue;
        }

        new_data[j++] = current;
    }

    new_data[j] = '\0';
    return new_data;
}

char *find_variable_data_type(char *variable)
{
    for (size_t i = 0; i < int_stack->index; i++)
    {
        if (strcmp(int_stack->name[i], variable) == 0)
        {
            index_var_g = i;
            return "int";
        }
    }

    for (size_t i = 0; i < float_stack->index; i++)
    {
        if (strcmp(float_stack->name[i], variable) == 0)
        {
            index_var_g = i;
            return "float";
        }
    }

    for (size_t i = 0; i < str_stack->index; i++)
    {
        if (strcmp(str_stack->name[i], variable) == 0)
        {
            index_var_g = i;
            return "str";
        }
    }

    for (size_t i = 0; i < char_stack->index; i++)
    {
        if (strcmp(char_stack->name[i], variable) == 0)
        {
            index_var_g = i;
            return "char";
        }
    }
    return "none";
}

char *combine_char(size_t start, size_t end, char *data)
{
    size_t length = (end - start) + 2;
    char *txt = malloc(length);
    txt[0] = '\0';

    for (size_t i = start; i < end; i++)
    {
        addChar(txt, data[i]);
    }
    txt[length] = '\0';
    return txt;
}

char *operation_concat(const OPERATION *op)
{
    if (!op || op->index == 0 || !op->data)
        return NULL;

    size_t total = 1;
    for (size_t i = 0; i < op->index; i++)
    {
        if (op->data[i])
            total += strlen(op->data[i]);
    }

    char *result = malloc(total);
    if (!result)
        return NULL;

    char *p = result;
    for (size_t i = 0; i < op->index; i++)
    {
        if (op->data[i])
        {
            size_t len = strlen(op->data[i]);
            memcpy(p, op->data[i], len);
            p += len;
        }
    }
    *p = '\0';
    return result;
}

char *combine_two(const char *data1, const char *data2)
{
    size_t len1 = strlen(data1);
    size_t len2 = strlen(data2);

    char *result = malloc(len1 + len2 + 1);
    if (!result)
        return NULL;

    strcpy(result, data1);
    strcat(result, data2);

    return result;
}

static int apply_operation(int left, int right, const char *op)
{
    if (strcmp(op, "*") == 0)
        return left * right;
    if (strcmp(op, "/") == 0)
    {
        if (right == 0)
        {
            fprintf(stderr, "Division by zero error!\n");
            return 0;
        }
        return left / right;
    }
    if (strcmp(op, "+") == 0)
        return left + right;
    if (strcmp(op, "-") == 0)
        return left - right;
    return 0;
}

int operation_excute(OPERATION *operation)
{
    size_t i = 0;
    while (i < operation->index)
    {
        char *op = operation->data[i];
        if (strcmp(op, "*") == 0 || strcmp(op, "/") == 0)
        {
            if (i == 0 || i + 1 >= operation->index)
            {
                fprintf(stderr, "Invalid expression near operator %s\n", op);
                return 0;
            }

            int left = atoi(operation->data[i - 1]);
            int right = atoi(operation->data[i + 1]);
            int result = apply_operation(left, right, op);

            char buffer[32];
            sprintf(buffer, "%d", result);

            free(operation->data[i - 1]);
            operation->data[i - 1] = strdup(buffer);

            free(operation->data[i]);
            free(operation->data[i + 1]);

            for (size_t k = i; k + 2 < operation->index; k++)
            {
                operation->data[k] = operation->data[k + 2];
            }

            operation->index -= 2;
            i = 0;
        }
        else
        {
            i++;
        }
    }

    i = 0;
    while (i < operation->index)
    {
        char *op = operation->data[i];
        if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0)
        {
            if (i == 0 || i + 1 >= operation->index)
            {
                fprintf(stderr, "Invalid expression near operator %s\n", op);
                return 0;
            }

            int left = atoi(operation->data[i - 1]);
            int right = atoi(operation->data[i + 1]);
            int result = apply_operation(left, right, op);

            char buffer[32];
            sprintf(buffer, "%d", result);

            free(operation->data[i - 1]);
            operation->data[i - 1] = strdup(buffer);

            free(operation->data[i]);
            free(operation->data[i + 1]);

            for (size_t k = i; k + 2 < operation->index; k++)
            {
                operation->data[k] = operation->data[k + 2];
            }

            operation->index -= 2;
            i = 0;
        }
        else
        {
            i++;
        }
    }

    return atoi(operation_stack->data[0]);
}

int sub_operation_excute(SUB_OPERATION *operation)
{
    size_t i = 0;
    while (i < operation->index)
    {
        char *op = operation->data[i];
        if (strcmp(op, "*") == 0 || strcmp(op, "/") == 0)
        {
            if (i == 0 || i + 1 >= operation->index)
            {
                fprintf(stderr, "Invalid expression near operator %s\n", op);
                return 0;
            }

            int left = atoi(operation->data[i - 1]);
            int right = atoi(operation->data[i + 1]);
            int result = apply_operation(left, right, op);

            char buffer[32];
            sprintf(buffer, "%d", result);

            free(operation->data[i - 1]);
            operation->data[i - 1] = strdup(buffer);

            free(operation->data[i]);
            free(operation->data[i + 1]);

            for (size_t k = i; k + 2 < operation->index; k++)
            {
                operation->data[k] = operation->data[k + 2];
            }

            operation->index -= 2;
            i = 0;
        }
        else
        {
            i++;
        }
    }

    i = 0;
    while (i < operation->index)
    {
        char *op = operation->data[i];
        if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0)
        {
            if (i == 0 || i + 1 >= operation->index)
            {
                fprintf(stderr, "Invalid expression near operator %s\n", op);
                return 0;
            }

            int left = atoi(operation->data[i - 1]);
            int right = atoi(operation->data[i + 1]);
            int result = apply_operation(left, right, op);
            char buffer[32];
            sprintf(buffer, "%d", result);

            free(operation->data[i - 1]);
            operation->data[i - 1] = strdup(buffer);

            free(operation->data[i]);
            free(operation->data[i + 1]);

            for (size_t k = i; k + 2 < operation->index; k++)
            {
                operation->data[k] = operation->data[k + 2];
            }

            operation->index -= 2;
            i = 0;
        }
        else
        {
            i++;
        }
    }

    return atoi(operation->data[0]);
}

void bracket_operation()
{
    printf("on bracket filter");
    size_t start = 0;
    size_t i = 0;
    while (i < operation_stack->index)
    {
        char *op = operation_stack->data[i];
        if (strcmp(op, "(") == 0)
        {
            start = i + 1;
            i++;
            continue;
        }
        else if (strcmp(op, ")") == 0)
        {
            printf(") detected at %zu\n", i);

            init_operation_sub_stack();
            for (size_t j = start; j < i; j++)
            {
                char *char_data = operation_stack->data[j];
                add_operetion_sub(char_data);
                free(operation_stack->data[j]);
                operation_stack->data[j] = strdup("_");
            }
            free(operation_stack->data[start - 1]);
            operation_stack->data[start - 1] = strdup("_");

            int res = sub_operation_excute(sub_operation_stack);
            printf("res : %d\n\n", res);
            if (i >= operation_stack->index)
            {
                fprintf(stderr, "Index %zu out of bounds\n", i);
                return;
            }

            char buffer[32];
            snprintf(buffer, sizeof(buffer), "%d", res);
            if (operation_stack->data[i] != NULL)
            {
                free(operation_stack->data[i]);
                operation_stack->data[i] = NULL;
            }

            operation_stack->data[i] = strdup(buffer);
            if (!operation_stack->data[i])
            {
                fprintf(stderr, "Failed to allocate memory for new string\n");
                exit(1);
            }
            test_sub_operation_stack();
            free(sub_operation_stack->data);
            i = 0;
            continue;
        }
        else
        {
            i++;
            continue;
        }
    }
}

int is_operators(char *s)
{
    return strlen(s) == 1 && strchr("+-/*", s[0]);
}

int is_number(char *s)
{
    int dot_count = 0;

    if (*s == '\0')
        return 0;

    for (int i = 0; s[i]; i++)
    {
        if (isdigit(s[i]))
        {
            continue;
        }
        else if (s[i] == '.')
        {
            dot_count++;
            if (dot_count > 1)
                return 0;
            if (i == 0 || s[i + 1] == '\0')
                return 0;
        }
        else
        {
            return 0;
        }
    }
    return 1;
}

void remove_underscores(OPERATION *op)
{
    if (!op || !op->data)
        return;

    size_t write_index = 0; // where to copy valid entries

    for (size_t i = 0; i < op->index; i++)
    {
        if (strcmp(op->data[i], "_") != 0)
        {
            // keep this entry
            if (write_index != i)
                op->data[write_index] = op->data[i]; // move pointer
            write_index++;
        }
        else
        {
            // free the underscore string
            free(op->data[i]);
        }
    }

    op->index = write_index; // update stack size
}

int main(void)
{
    init_lexer_stack();
    init_operation_stack();
    init_int_stack();
    init_str_stack();
    init_float_stack();
    init_char_stack();
    init_operation_sub_stack();

    FILE *file = fopen("./data/data.txt", "r");
    if (file == NULL)
    {
        perror("No file ditected");
    }

    tokenization(file);
    init_variable();
    add_values_var();

    for (size_t i = 0; i < int_stack->index; i++)
    {
        printf("name : %s || data : %d\n", int_stack->name[i], int_stack->data[i]);
    }

    return 0;
}

void test_lexer_stack()
{
    printf("----- LEXER STACK -----\n");
    for (size_t i = 0; i < lexer_stack->index; i++)
    {
        printf("%zu: data='%s', type='%s'\n", i, lexer_stack->data[i], lexer_stack->type[i]);
    }
}

void test_operation_stack()
{
    printf("----- OPERATION STACK -----\n");
    for (size_t i = 0; i < operation_stack->index; i++)
    {
        printf("%zu: '%s'\n", i, operation_stack->data[i]);
    }
}

void test_sub_operation_stack()
{
    printf("----- SUB OPERATION STACK -----\n");
    for (size_t i = 0; i < sub_operation_stack->index; i++)
    {
        printf("%zu: '%s'\n", i, sub_operation_stack->data[i]);
    }
}

void test_int_stack()
{
    printf("----- INT STACK -----\n");
    for (size_t i = 0; i < int_stack->index; i++)
    {
        printf("%zu: name='%s', value=%d\n", i, int_stack->name[i], int_stack->data[i]);
    }
}

void test_float_stack()
{
    printf("----- FLOAT STACK -----\n");
    for (size_t i = 0; i < float_stack->index; i++)
    {
        printf("%zu: name='%s', value=%.2f\n", i, float_stack->name[i], float_stack->data[i]);
    }
}

void test_str_stack()
{
    printf("----- STR STACK -----\n");
    for (size_t i = 0; i < str_stack->index; i++)
    {
        printf("%zu: name='%s', value='%s'\n", i, str_stack->name[i], str_stack->data[i]);
    }
}

void test_char_stack()
{
    printf("----- CHAR STACK -----\n");
    for (size_t i = 0; i < char_stack->index; i++)
    {
        printf("%zu: name='%s', value='%c'\n", i, char_stack->name[i], char_stack->data[i]);
    }
}

void test_all_stacks()
{
    test_lexer_stack();
    test_operation_stack();
    test_sub_operation_stack();
    test_int_stack();
    test_float_stack();
    test_str_stack();
    test_char_stack();
}
