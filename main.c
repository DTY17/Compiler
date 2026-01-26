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
INT *int_stack;
STR *str_stack;
FLOAT *float_stack;
CHAR *char_stack;

void find_variable(char *data, size_t count);
char *combine_char(size_t start, size_t end, char *data);
void addChar(char *str, char c);
char *find_variable_data_type(char *variable);
char *space_adjust(char *data);

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

void init_int_stack()
{
    int_stack = malloc(sizeof(INT));
    int_stack->capacity = 1;
    int_stack->index = 0;
    int_stack->data = malloc(int_stack->capacity * sizeof(int));
    int_stack->name = malloc(int_stack->capacity * sizeof(char *));
}

void add_int(int data, const char *name)
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
        find_variable(new_lexer, j);
    }
}

void find_variable(char *data, size_t indx_count)
{
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
            is_variable_name = false;
            is_in_param = true;
            is_method = true;
        }
        if (!isString && data[i] == ')')
        {
            is_in_param = false;
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

            if (isVariable)
            {
                if (is_variable_name)
                {
                    variable_Name = txt;
                    is_variable_name = false;
                    continue;
                }
            }

            if (is_value && !is_variable_name)
            {
                continue;
            }

            if (strcmp(txt, "=") == 0 && isVariable)
            {
                is_variable_name = false;
                is_value = true;
                continue;
            }

            if (strcmp(txt, "int") == 0)
            {
                is_variable_name = true;
                isVariable = true;
                data_type = "int";
                init_var = true;
                continue;
            }
            else if (strcmp(txt, "float") == 0)
            {
                is_variable_name = true;
                isVariable = true;
                data_type = "float";
                init_var = true;
                continue;
            }
            else if (strcmp(txt, "str") == 0)
            {
                is_variable_name = true;
                isVariable = true;
                data_type = "str";
                init_var = true;
                continue;
            }
            else if (strcmp(txt, "char") == 0)
            {
                is_variable_name = true;
                isVariable = true;
                data_type = "char";
                init_var = true;
                continue;
            }
            else
            {
                declear_var = true;
                type_var_g = find_variable_data_type(txt);
                if (strcmp(type_var_g, "int") == 0)
                {
                    is_variable_name = true;
                    isVariable = true;
                    data_type = "int";
                    init_var = true;
                    continue;
                }
                else if (strcmp(type_var_g, "float") == 0)
                {
                    is_variable_name = true;
                    isVariable = true;
                    data_type = "float";
                    init_var = true;
                    continue;
                }
                else if (strcmp(type_var_g, "str") == 0)
                {
                    is_variable_name = true;
                    isVariable = true;
                    data_type = "str";
                    init_var = true;
                    continue;
                }
                else if (strcmp(type_var_g, "char") == 0)
                {
                    is_variable_name = true;
                    isVariable = true;
                    data_type = "char";
                    init_var = true;
                    continue;
                }
            }
        }
    }

    char *txts = combine_char(count, strlen(data), data);

    if (string_ditected)
        txts = combine_char(count + 1, strlen(data) - 1, data);
    if (char_ditected)
        txts = combine_char(count + 1, strlen(data) - 1, data);

    if (!declear_var)
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
    }
    else
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
    }

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
    data_type = "none";
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
            return "int";
            index_var_g = i;
        }
    }

    for (size_t i = 0; i < float_stack->index; i++)
    {
        if (strcmp(float_stack->name[i], variable) == 0)
        {
            return "float";
            index_var_g = i;
        }
    }

    for (size_t i = 0; i < str_stack->index; i++)
    {
        if (strcmp(str_stack->name[i], variable) == 0)
        {
            return "str";
            index_var_g = i;
        }
    }

    for (size_t i = 0; i < char_stack->index; i++)
    {
        if (strcmp(char_stack->name[i], variable) == 0)
        {
            return "char";
            index_var_g = i;
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

int main(void)
{
    init_lexer_stack();
    init_int_stack();
    init_str_stack();
    init_float_stack();
    init_char_stack();

    FILE *file = fopen("./data/data.txt", "r");
    if (file == NULL)
    {
        perror("No file ditected");
    }
    tokenization(file);
    init_variable();

    
    for (size_t i = 0; i < int_stack->index; i++)
    {
        printf("data : %d\n", int_stack->data[i]);
    }

    return 0;
}
