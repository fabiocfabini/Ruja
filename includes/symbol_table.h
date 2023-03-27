#ifndef RUJA_SYMBOL_TABLE_H
#define RUJA_SYMBOL_TABLE_H

#include "common.h"
#include "types.h"

typedef enum {
    SYMBOL_VAR,
} Symbol_Type;

typedef struct {
    Symbol_Type type;
    char* key;
    size_t key_length;
    union {
        struct {
            Var_Type type;
        } var;
    } as;
} Symbol;

Symbol *symbol_new_var(Var_Type type, char *name, size_t name_length);
void symbol_free(Symbol *symbol);
void symbol_print(Symbol *symbol);

#define DEFAUlT_SYMBOL_TABLE_CAPACITY 8
typedef struct {
    size_t count;
    size_t capacity;
    Symbol **symbols;
} Ruja_Symbol_Table;

Ruja_Symbol_Table *symbol_table_new(size_t capacity);
void symbol_table_free(Ruja_Symbol_Table *symbol_table);
void symbol_table_print(Ruja_Symbol_Table *symbol_table);

void symbol_table_resize(Ruja_Symbol_Table *symbol_table, size_t new_capacity);

void symbol_table_insert(Ruja_Symbol_Table *symbol_table, Symbol *symbol);
Symbol *symbol_table_lookup(Ruja_Symbol_Table *symbol_table, char *key, size_t key_length);
//NOTE: Why would I want to remove a symbol from the symbol table?
//bool symbol_table_remove(Ruja_Symbol_Table *symbol_table, char *key);



#endif // RUJA_SYMBOL_TABLE_H