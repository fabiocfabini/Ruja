#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../includes/symbol_table.h"


Symbol *symbol_new_var(Type type, char *name, size_t name_length) {
    Symbol *symbol = malloc(sizeof(Symbol));
    if (symbol == NULL) {
        fprintf(stderr, "Error: Could not allocate memory for symbol var.\n");
        return NULL;
    }

    symbol->type = SYMBOL_VAR;
    symbol->key = name;
    symbol->key_length = name_length;
    symbol->as.var.type = type;

    return symbol;
}

void symbol_free(Symbol *symbol) {
    if (symbol == NULL) {
        return;
    }

    switch (symbol->type) {
        case SYMBOL_VAR: break; // Nothing to free
    }

    free(symbol);
}

void symbol_print(Symbol *symbol) {
    if (symbol == NULL) {
        printf("NULL\n");
        return;
    }

    switch (symbol->type) {
        case SYMBOL_VAR: {
            printf("VAR(");
            switch (symbol->as.var.type) {
                case VAR_TYPE_NIL: printf("nil,"); break;
                case VAR_TYPE_BOOL: printf("bool,"); break;
                case VAR_TYPE_CHAR: printf("char,"); break;
                case VAR_TYPE_I32: printf("i32,"); break;
                case VAR_TYPE_F64: printf("f64,"); break;
                case VAR_TYPE_STRING: printf("string,"); break;
            }
            printf("%.*s)\n", (int) symbol->key_length, symbol->key);
            break;
        }
    }
}


Ruja_Symbol_Table *symbol_table_new(size_t capacity) {
    Ruja_Symbol_Table *symbol_table = malloc(sizeof(Ruja_Symbol_Table));
    if (symbol_table == NULL) {
        fprintf(stderr, "Error: Could not allocate memory for symbol table.\n");
        return NULL;
    }

    symbol_table->count = 0;
    symbol_table->capacity = capacity;
    symbol_table->symbols = calloc(capacity, sizeof(Symbol*));

    return symbol_table;
}

void symbol_table_free(Ruja_Symbol_Table *symbol_table) {
    if (symbol_table == NULL) {
        return;
    }

    for (size_t i = 0; i < symbol_table->capacity; i++) {
        if (symbol_table->symbols[i] != NULL) {
            symbol_free(symbol_table->symbols[i]);
            symbol_table->symbols[i] = NULL;
        }
    }

    free(symbol_table->symbols);
    free(symbol_table);
}

void symbol_table_print(Ruja_Symbol_Table *symbol_table) {
    if (symbol_table == NULL) {
        return;
    }

    printf("Symbol Table: {\n");
    for (size_t i = 0; i < symbol_table->capacity; i++) {
        printf("  [%zu]: ", i);
        symbol_print(symbol_table->symbols[i]);
    }
    printf("}\n");
}

static size_t hash(char *str, size_t len) {
    size_t hash = 5381;

    while (len--)
         hash = ((hash << 5) + hash) + *str++; /* hash * 33 + c */

    return hash;
}

void symbol_table_resize(Ruja_Symbol_Table *symbol_table, size_t new_capacity) {
    Symbol **new_symbols = calloc(new_capacity, sizeof(Symbol*));
    Symbol **old_symbols = symbol_table->symbols;
    size_t old_capacity = symbol_table->capacity;
    symbol_table->symbols = new_symbols;

    symbol_table->count = 0;
    symbol_table->capacity = new_capacity;
    for (size_t i = 0; i < old_capacity; i++) {
        if (old_symbols[i] != NULL) {
            symbol_table_insert(symbol_table, old_symbols[i]);
        }
    }

    free(old_symbols);
}

void symbol_table_insert(Ruja_Symbol_Table *symbol_table, Symbol *symbol) {
    if (symbol_table->count >= symbol_table->capacity) {
        symbol_table_resize(symbol_table, symbol_table->capacity * 2);
    }

    size_t hash_value = hash(symbol->key, symbol->key_length) % symbol_table->capacity;

    int i = 1;
    while (symbol_table->symbols[hash_value] != NULL) {
        hash_value = (hash_value + i * i) % symbol_table->capacity;
        i++;
    }

    symbol_table->symbols[hash_value] = symbol;
    symbol_table->count++;
}

Symbol *symbol_table_lookup(Ruja_Symbol_Table *symbol_table, char *key, size_t key_length) {
    size_t hash_value = hash(key, key_length) % symbol_table->capacity;

    int i = 1;
    while (symbol_table->symbols[hash_value] != NULL) {
        if (symbol_table->symbols[hash_value]->key_length == key_length &&
            strncmp(symbol_table->symbols[hash_value]->key, key, key_length) == 0) {
            return symbol_table->symbols[hash_value];
        }

        hash_value = (hash_value + i * i) % symbol_table->capacity;
        i++;
    }

    return NULL;
}