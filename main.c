#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <time.h>

typedef enum
{
    INTEGER_DATATYPE,
    FLOAT_DATATYPE,
    CHAR_DATATYPE,
    STR_DATATYPE,
    NONE,
    IF
} DATATYPE;

typedef struct
{
    size_t capacity;
    size_t index;
    char **data;
    char **state;
    DATATYPE *type;
} LEXER;

// typedef struct
// {
//     size_t capacity;
//     size_t index;
//     char **data;
//     DATATYPE *type;
// } CODEBLOCK;

// typedef struct
// {
//     size_t capacity;
//     size_t index;
//     CODEBLOCK **data;
//     DATATYPE *type;
// } CODEBLOCK_LIST;

typedef struct
{
    size_t capacity;
    size_t index;
    char **data;   // array of strings
    DATATYPE type; // single enum value
} CODEBLOCK;

typedef struct
{
    size_t capacity;
    size_t index;
    CODEBLOCK *data; // array of CODEBLOCKs
} CODEBLOCK_LIST;

typedef struct
{
    size_t capacity;
    size_t index;
    char **data;
} CONDITION;

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
CODEBLOCK_LIST *list_codeblock;

char **scope_stack = NULL;

void find_variable(char *data, size_t indx_count, bool init_parse);
char *combine_char(size_t start, size_t end, char *data);
void addChar(char *str, char c);
DATATYPE find_variable_data_type(char *variable);
char *space_adjust(char *data);
char *combine_two(const char *data1, const char *data2);
char *operation_concat(const OPERATION *op);
int operation_excute(OPERATION *operation);
float operation_excute_float(OPERATION *operation);
float sub_operation_excute_float(SUB_OPERATION *operation);
int sub_operation_excute(SUB_OPERATION *operation);
void remove_underscores(OPERATION *op);

int is_number(char *s);
int is_operators(char *s);
void bracket_operation(bool isFloat);

void test_all_stacks();
void test_char_stack();
void test_str_stack();
void test_float_stack();
void test_int_stack();
void test_sub_operation_stack();
void test_operation_stack();
void test_lexer_stack();

bool condition_apply(char *data);

bool isNumber(const char *str)
{
    int i = 0;
    if (str[0] == '\0')
        return false; // empty string
    while (str[i])
    {
        if (!isdigit(str[i]))
            return false;
        i++;
    }
    return true;
}
// char **operation_order = {"-","+","*","/"};

void init_lexer_stack()
{
    lexer_stack = malloc(sizeof(LEXER));
    lexer_stack->capacity = 1;
    lexer_stack->index = 0;
    lexer_stack->type = malloc(lexer_stack->capacity * sizeof(char *));
    lexer_stack->state = malloc(lexer_stack->capacity * sizeof(char *));
    lexer_stack->data = malloc(lexer_stack->capacity * sizeof(DATATYPE));
}

void add_lexer_stack(char *data, char *state, DATATYPE type)
{
    if (lexer_stack->capacity <= lexer_stack->index)
    {
        lexer_stack->capacity = lexer_stack->capacity * 2;
        lexer_stack->data = realloc(lexer_stack->data, lexer_stack->capacity * sizeof(char *));
        lexer_stack->state = realloc(lexer_stack->state, lexer_stack->capacity * sizeof(char *));
        lexer_stack->type = realloc(lexer_stack->type, lexer_stack->capacity * sizeof(DATATYPE));
    }
    lexer_stack->data[lexer_stack->index] = strdup(data);
    lexer_stack->state[lexer_stack->index] = state;
    lexer_stack->type[lexer_stack->index] = type;
    lexer_stack->index++;
}

void change_lexer_type(size_t i, DATATYPE type)
{
    lexer_stack->type[i] = type;
}

void init_codeblocklist_stack()
{
    list_codeblock = malloc(sizeof(CODEBLOCK_LIST));
    list_codeblock->capacity = 1;
    list_codeblock->index = 0;
    list_codeblock->data = malloc(list_codeblock->capacity * sizeof(CODEBLOCK));
}

void add_codeblocklist_stack(const char *text, DATATYPE type)
{
    if (list_codeblock->capacity <= list_codeblock->index)
    {
        list_codeblock->capacity *= 2;
        list_codeblock->data = realloc(list_codeblock->data,
                                       list_codeblock->capacity * sizeof(CODEBLOCK));
    }

    CODEBLOCK *block = &list_codeblock->data[list_codeblock->index];
    block->capacity = 1;
    block->index = 0;
    block->data = malloc(block->capacity * sizeof(char *));
    block->type = type;

    block->data[block->index++] = strdup(text);

    list_codeblock->index++;
}

CODEBLOCK *add_codeblock_stack(const char *text, const char *state, DATATYPE type)
{
    CODEBLOCK *code_block = malloc(sizeof(CODEBLOCK));
    code_block->capacity = 1;
    code_block->index = 0;
    code_block->data = malloc(sizeof(char *));
    code_block->data[0] = strdup(text);
    code_block->type = type;
    return code_block;
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
void change_float(float data, size_t i)
{
    float_stack->data[i] = data;
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
// bool is_value = false;

bool isChar = false;
bool char_ditected = false;
bool is_in_param = false;
bool is_method = true;

char *variable_Name = "none";
char *var_d_type = "none";
char *data_type = "none";

int index_var_g = 0;
DATATYPE type_var_g = NONE;

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

char *addString(char *data, char *pre_string)
{
    size_t len1 = strlen(pre_string);
    size_t len2 = strlen(data);

    char *new_string = malloc(len1 + len2 + 1);
    if (!new_string)
        return NULL;

    strcpy(new_string, pre_string);
    strcat(new_string, data);

    return new_string;
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
            add_lexer_stack(t, "init", NONE);
            free(t);
            t = malloc(1);
            t[0] = '\0';
        }
        else if (ch == '}')
        {
            char *newchar = strcat(t, "}");
            add_lexer_stack(t, "init", NONE);
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
        printf("Var init \n");
        char *new_lexer = space_adjust(lexer_stack->data[j]);
        find_variable(new_lexer, j, false);
    }
}

bool isCondition = false;
char *condition = NULL;
bool cb_range = false;
bool if_statment = false;
bool code_block = false;
size_t statemtn_start = 0;

char *statement_excute_part = NULL;
char *statement = "NONE";

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void find_variable(char *data, size_t indx_count, bool init_parse)
{
    bool is_operator = false;
    size_t count = 0;
    size_t str_len = strlen(data);
    DATATYPE type = NONE;

    bool isFloat = false;
    bool isCharDetected = false;
    bool isStringDetected = false;
    bool is_value = false;
    bool is_var_in_op = false;
    bool is_float = false;
    bool is_string = false;

    bool is_if_condition = false;

    bool isBlockCode = false;

    for (size_t i = 0; i < str_len; i++)
    {
        bool is_separator = data[i] == ' ';

        if (isalpha(data[i]) && !isString && is_value)
        {
            is_var_in_op = true;
        }

        if (data[i] == '.' && !isString && is_value)
        {
            is_float = true;
        }

        if (data[i] == '"')
        {
            is_string = !is_string;
        }

        // Process separators (tokens)
        if (is_separator && !isString)
        {
            char *txt = combine_char(count, i, data);
            count = i + 1;

            bool asign_symbol = *txt == '=';
            bool is_char = *txt == '\'';
            if (asign_symbol)
            {
                is_value = true;
                is_variable_name = false;
                free(txt);
                continue;
            }

            if (strcmp(txt, "if") == 0)
            {
                statement = "IF";
                if_statment = true;
                free(txt);
                continue;
            }
            if (strcmp(txt, "else") == 0)
            {
                statement = "ELSE";
                free(txt);
                continue;
            }
            if (strcmp(txt, "SW") == 0)
            {
                statement = "SW";
                free(txt);
                continue;
            }
            if (strcmp(txt, "FOR") == 0)
            {
                statement = "FOR";
                free(txt);
                continue;
            }

            if (if_statment && strcmp(txt, "(") == 0 && !isCondition)
            {
                isCondition = true;
            }

            if (strcmp(txt, "{") != 0)
            {
            }

            if (isCondition && strcmp(txt, "{") != 0 && strcmp(txt, "}") != 0)
            {
                char *ns = addString(txt, condition ? condition : "");
                free(condition);
                condition = ns;
                // char *ns = addString(txt, condition);
                if (condition == NULL)
                {
                    // handle allocation failure
                }
                else
                {
                    condition = realloc(condition, strlen(ns));
                    condition = ns;
                }
                free(txt);
                continue;
            }

            if (isCondition && strcmp(txt, "{") == 0)
            {
                char *space_condition = space_adjust(condition);
                add_operetors(space_condition);
                operation_excute(operation_stack);
                bool isConTrue = condition_apply(space_condition);
                printf("Condition is : %d\n", isConTrue);
                isCondition = false;
                
                if (isConTrue)
                {
                    statement_excute_part = "IF";
                }
                else
                {
                    statement_excute_part = "ELSE";
                }
                code_block = true;

                free(txt);
                continue;
            }

            if (statement_excute_part && (strcmp(statement, statement_excute_part) == 0))
            {
                CODEBLOCK *block = add_codeblock_stack(data, "true", IF);
            }

            // Detect string, char, float
            isStringDetected = is_value && is_string;
            isCharDetected = is_value && is_char;
            isFloat = !isString && is_float;

            if (is_float)
            {
                printf("float\n");
            }

            // Update isString for quotes
            if (is_value && is_string)
            {
                isString = !isString;
            }

            // Detect datatype
            if (strcmp(txt, "int") == 0)
            {
                isVariable = true;
                is_variable_name = true;
                init_var = true;
                type = INTEGER_DATATYPE;
                free(txt);
                continue;
            }
            else if (strcmp(txt, "float") == 0)
            {
                isVariable = true;
                is_variable_name = true;
                init_var = true;
                type = FLOAT_DATATYPE;
                free(txt);
                continue;
            }
            else if (strcmp(txt, "char") == 0)
            {
                isVariable = true;
                is_variable_name = true;
                init_var = true;
                type = CHAR_DATATYPE;
                free(txt);
                continue;
            }
            else if (strcmp(txt, "str") == 0)
            {
                isVariable = true;
                is_variable_name = true;
                init_var = true;
                type = STR_DATATYPE;
                free(txt);
                continue;
            }

            // Detect operators (+, -, *, /, (, ))
            bool operate_symbol = txt[0] != '\0' && txt[1] == '\0' &&
                                  (txt[0] == '+' || txt[0] == '-' || txt[0] == '*' ||
                                   txt[0] == '/' || txt[0] == '(' || txt[0] == ')');

            if (operate_symbol || (is_value && type == INTEGER_DATATYPE))
            {
                is_operator = true;
            }

            if (operate_symbol || (is_value && type == FLOAT_DATATYPE))
            {
                is_operator = true;
            }

            // Variable assignment
            if (isVariable && is_variable_name)
            {
                variable_Name = strdup(txt);
                pre_text = strdup(txt);
                is_variable_name = false;
                // variable_Name = txt;
                // pre_text = txt;
                free(txt);
                continue;
            }

            // when find varible inside oepration
            if (is_value && !is_variable_name && is_operator)
            {
                if (is_var_in_op)
                {
                    int pre_index_g = index_var_g;
                    DATATYPE type_var_g = find_variable_data_type(txt);
                    char str[32];
                    if (type_var_g == FLOAT_DATATYPE)
                    {
                        snprintf(str, sizeof(str), "%f", float_stack->data[index_var_g]);
                        is_var_in_op = false;
                        index_var_g = pre_index_g;
                    }
                    else if (type_var_g == INTEGER_DATATYPE)
                    {
                        snprintf(str, sizeof(str), "%d", int_stack->data[index_var_g]);
                        is_var_in_op = false;
                        index_var_g = pre_index_g;
                    }
                    add_operetors(str);
                }
                else
                {
                    add_operetors(txt);
                }
                free(txt);
                continue;
            }

            type = find_variable_data_type(txt);
            declear_var = true;
            variable_Name = txt;
            pre_text = txt;
            isCondition = false;
            condition = NULL;
            cb_range = false;
            if_statment = false;
            free(txt);
            continue;
        }
    }

    char *txts = combine_char(count, str_len, data);
    if ((is_value && type == INTEGER_DATATYPE) || type == FLOAT_DATATYPE)
    {
        is_operator = true;
    }

    if (is_float && is_operator)
    {
        isFloat = true;
    }

    if (if_statment && strcmp(txts, "}") == 0)
    {
        if_statment = false;
    }

    if (is_operator)
    {
        if (is_var_in_op)
        {
            int pre_index_g = index_var_g;
            DATATYPE type_var_g = find_variable_data_type(txts);
            char str[32];
            if (type_var_g == FLOAT_DATATYPE)
            {
                snprintf(str, sizeof(str), "%f", float_stack->data[index_var_g]);
                is_var_in_op = false;
                index_var_g = pre_index_g;
            }
            else if (type_var_g == INTEGER_DATATYPE)
            {
                snprintf(str, sizeof(str), "%d", int_stack->data[index_var_g]);
                is_var_in_op = false;
                index_var_g = pre_index_g;
            }
            add_operetors(str);
        }
        else
        {
            add_operetors(txts);
        }
        // concat whole operation to one string
        // operation_concat(operation_stack);
        // solve brackets and rplace solve bracker with answer adn other solve parts  with _
        bracket_operation(is_float || type == FLOAT_DATATYPE);
        // remove _
        remove_underscores(operation_stack);

        if (type == FLOAT_DATATYPE)
        {
            // excute opration wihtout bracket
            float v1 = operation_excute_float(operation_stack); //(float)
            char str[32];
            snprintf(str, sizeof(str), "%f", v1);
            // add final result to txts variable
            txts = strdup(str);
        }
        else if (type == INTEGER_DATATYPE)
        {
            // excute opration wihtout bracket
            int v1 = operation_excute(operation_stack); //(int)
            char str[32];
            snprintf(str, sizeof(str), "%d", v1);
            // add final result to txts variable
            txts = strdup(str);
        }
    }

    // Store variables in lexer stack
    if (!declear_var && init_parse)
    {
        switch (type)
        {
        case STR_DATATYPE:
            if (isStringDetected && !isString)
            {
                change_lexer_type(indx_count, STR_DATATYPE);
                add_str(txts, variable_Name);
            }
            break;
        case CHAR_DATATYPE:
            if (isCharDetected && !isChar)
            {
                change_lexer_type(indx_count, CHAR_DATATYPE);
                add_char(txts[0], variable_Name);
            }
            break;
        case INTEGER_DATATYPE:
            change_lexer_type(indx_count, INTEGER_DATATYPE);
            add_int(atoi(txts), variable_Name);
            break;
        case FLOAT_DATATYPE:
            change_lexer_type(indx_count, FLOAT_DATATYPE);
            add_float(atof(txts), variable_Name);
            break;
        default:
            break;
        }
    }
    else if (declear_var && !init_parse)
    {
        switch (type)
        {
        case STR_DATATYPE:
            if (isStringDetected && !isString)
            {
                change_lexer_type(indx_count, STR_DATATYPE);
                add_str(txts, variable_Name);
            }
            break;
        case CHAR_DATATYPE:
            if (isCharDetected && !isChar)
            {
                change_lexer_type(indx_count, CHAR_DATATYPE);
                add_char(txts[0], variable_Name);
            }
            break;
        case INTEGER_DATATYPE:
            change_lexer_type(indx_count, INTEGER_DATATYPE);
            change_int(atoi(txts), index_var_g);
            break;
        case FLOAT_DATATYPE:
            if (isFloat)
            {
                change_lexer_type(indx_count, FLOAT_DATATYPE);
                change_float(atof(txts), index_var_g);
            }
            break;
        default:
            break;
        }
    }

    free_operation_stack(operation_stack);
    free(txts);

    // Reset flags
    declear_var = false;
    init_var = false;
    isCharDetected = false;
    isChar = false;
    isString = false;
    isStringDetected = false;
    isFloat = false;
    isVariable = false;
    is_variable_name = false;
    is_value = false;
    is_operator = false;
    data_type = "none";
    variable_Name = "none";
    is_var_in_op = false;
    type = NONE;
    isCondition = false;
    condition = NULL;
    cb_range = false;
    if_statment = false;
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

        // skip carriage returns/newlines
        if (current == '\r' || current == '\n')
            continue;

        // symbols list
        int is_symbol = (current == '=' || current == '(' || current == ')' ||
                         current == '+' || current == '-' || current == '/' ||
                         current == '*' || current == '<' || current == '>' ||
                         current == '{' || current == '}');

        if (is_symbol && j > 0 && new_data[j - 1] != ' ')
            new_data[j++] = ' ';

        new_data[j++] = current;

        if (is_symbol && (i + 1 < len) && data[i + 1] != ' ')
            new_data[j++] = ' ';
    }

    new_data[j] = '\0';
    return new_data;
}

bool condition_apply(char *data)
{
    bool *list = malloc(sizeof(bool) * 5);
    char *token = strtok(data, " ");

    // just declare pointers, no malloc needed
    char *left_data = NULL;
    char *main_op = NULL;
    char *sub_op = NULL;
    char *right_data = NULL;

    while (token != NULL)
    {
        if (strcmp(token, ">") == 0)
        {
            int left_int = atoi(left_data);
            token = strtok(NULL, " ");
            right_data = token; // capture right side
            int right_int = atoi(right_data);
            bool isTrueResult = left_int > right_int;
            list[0] = isTrueResult;
            continue;
        }
        else if (strcmp(token, "<") == 0)
        {
            int left_int = atoi(left_data);
            token = strtok(NULL, " ");
            right_data = token;
            int right_int = atoi(right_data);
            bool isTrueResult = left_int < right_int;
            list[0] = isTrueResult;
            continue;
        }
        else if (strcmp(token, "=") == 0)
        {
            int left_int = atoi(left_data);
            token = strtok(NULL, " ");
            right_data = token;
            int right_int = atoi(right_data);
            bool isTrueResult = left_int == right_int;
            list[0] = isTrueResult;
            continue;
        }

        // store left operand safely
        left_data = token;

        token = strtok(NULL, " ");
    }

    bool final = *(list);
    free(list);
    return final;
}

DATATYPE find_variable_data_type(char *variable)
{
    for (size_t i = 0; i < int_stack->index; i++)
    {
        if (strcmp(int_stack->name[i], variable) == 0)
        {
            index_var_g = i;
            return INTEGER_DATATYPE;
        }
    }

    for (size_t i = 0; i < float_stack->index; i++)
    {
        if (strcmp(float_stack->name[i], variable) == 0)
        {
            index_var_g = i;
            return FLOAT_DATATYPE;
        }
    }

    for (size_t i = 0; i < str_stack->index; i++)
    {
        if (strcmp(str_stack->name[i], variable) == 0)
        {
            index_var_g = i;
            return STR_DATATYPE;
        }
    }

    for (size_t i = 0; i < char_stack->index; i++)
    {
        if (strcmp(char_stack->name[i], variable) == 0)
        {
            index_var_g = i;
            return CHAR_DATATYPE;
        }
    }
    return NONE;
}

char *combine_char(size_t start, size_t end, char *data)
{
    if (!data || start >= end)
        return NULL;

    size_t length = end - start;
    char *txt = calloc(length + 1, 1); // zero‑initialized
    if (!txt)
        return NULL;

    size_t j = 0;
    for (size_t i = start; i < end; i++)
    {
        // skip spaces and carriage returns/newlines
        if (data[i] != ' ' && data[i] != '\r' && data[i] != '\n')
        {
            txt[j++] = data[i];
        }
    }
    txt[j] = '\0';

    if (j == 0)
    {
        free(txt);
        return NULL;
    }

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
static float apply_operation_float(float left, float right, const char *op)
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
            sprintf(buffer, "%d", result); // (int)

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
            sprintf(buffer, "%d", result); //(int)

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

int operation_excute_bool(OPERATION *operation)
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
            sprintf(buffer, "%d", result); // (int)

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
            sprintf(buffer, "%d", result); //(int)

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
        if (strcmp(op, "<") == 0)
        {
            if (i == 0 || i + 1 >= operation->index)
            {
                fprintf(stderr, "Invalid expression near operator %s\n", op);
                return 0;
            }
            int left = 0;
            int right = 0;
            int result = 0;

            if (isNumber(operation->data[i - 1]))
            {
                left = atoi(operation->data[i - 1]);
                right = atoi(operation->data[i + 1]);
                result = apply_operation(left, right, op);
            } else {
                ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            }

            char buffer[32];
            sprintf(buffer, "%d", result); //(int)

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
        else if (strcmp(op, ">") == 0)
        {
            if (i == 0 || i + 1 >= operation->index)
            {
                fprintf(stderr, "Invalid expression near operator %s\n", op);
                return 0;
            }

            int left = 0;
            int right = 0;
            int result = 0;

            if (isNumber(operation->data[i - 1]))
            {
                left = atoi(operation->data[i - 1]);
                right = atoi(operation->data[i + 1]);
                result = apply_operation(left, right, op);
            } else {
                ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            }


            char buffer[32];
            sprintf(buffer, "%d", result); //(int)

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

float operation_excute_float(OPERATION *operation)
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

            float left = atof(operation->data[i - 1]);
            float right = atof(operation->data[i + 1]);
            float result = apply_operation_float(left, right, op);

            char buffer[32];
            sprintf(buffer, "%f", result);

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

            float left = atof(operation->data[i - 1]);
            float right = atof(operation->data[i + 1]);
            float result = apply_operation_float(left, right, op);

            char buffer[32];
            sprintf(buffer, "%f", result); //(int)

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

    return atof(operation_stack->data[0]);
}

float sub_operation_excute_float(SUB_OPERATION *operation)
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
            float left = atof(operation->data[i - 1]);
            float right = atof(operation->data[i + 1]);
            float result = apply_operation_float(left, right, op);
            char buffer[32];
            sprintf(buffer, "%.6f", result);

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

            float left = atof(operation->data[i - 1]);
            float right = atof(operation->data[i + 1]);
            float result = apply_operation_float(left, right, op);
            char buffer[32];
            sprintf(buffer, "%.6f", result);

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

    return atof(operation->data[0]);
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

            int left = atof(operation->data[i - 1]);
            int right = atof(operation->data[i + 1]);
            int result = apply_operation(left, right, op);

            char buffer[32];
            sprintf(buffer, "%d", result); // (int)

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

            int left = atof(operation->data[i - 1]);
            int right = atof(operation->data[i + 1]);
            int result = apply_operation(left, right, op);
            char buffer[32];
            sprintf(buffer, "%d", (int)result); // (int)

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

void bracket_operation(bool isFloat)
{
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
            if (isFloat)
            {
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

                float res = sub_operation_excute_float(sub_operation_stack);
                if (i >= operation_stack->index)
                {
                    fprintf(stderr, "Index %zu out of bounds\n", i);
                    return;
                }

                char buffer[32];
                snprintf(buffer, sizeof(buffer), "%.6f", res);

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

                free(sub_operation_stack->data);
                i = 0;
                remove_underscores(operation_stack);
            }
            else
            {
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

                free(sub_operation_stack->data);
                i = 0;
                remove_underscores(operation_stack);
            }
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

    size_t write_index = 0;

    for (size_t i = 0; i < op->index; i++)
    {
        if (strcmp(op->data[i], "_") != 0)
        {
            if (write_index != i)
                op->data[write_index] = op->data[i];
            write_index++;
        }
        else
        {
            free(op->data[i]);
        }
    }

    op->index = write_index;
}

int main(void)
{
    clock_t start, end;
    double cpu_time_used;
    start = clock();
    init_lexer_stack();
    init_operation_stack();
    init_int_stack();
    init_str_stack();
    init_float_stack();
    init_char_stack();
    init_operation_sub_stack();
    init_codeblocklist_stack();

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
    for (size_t i = 0; i < float_stack->index; i++)
    {
        printf("name : %s || data : %f\n", float_stack->name[i], float_stack->data[i]);
    }

    end = clock();
    cpu_time_used = (((double)(end - start)) / CLOCKS_PER_SEC);
    printf("Time taken: %f seconds\n", cpu_time_used);

    return 0;
}

void test_lexer_stack()
{
    printf("----- LEXER STACK -----\n");
    for (size_t i = 0; i < lexer_stack->index; i++)
    {
        printf("%zu: data=%s type=%s\n", i, lexer_stack->data[i], lexer_stack->state[i]);
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
