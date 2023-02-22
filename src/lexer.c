#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "../includes/lexer.h"

/**
 * @brief Sets the start position of the lexer to the current position.
 * 
 * @param lexer 
 */
static void rebase(Ruja_Lexer *lexer) {
    lexer->start = lexer->current;
}

/**
 * @brief Advances the current position of the lexer by one character.
 * 
 * @param lexer The lexer to advance.
 */
static void advance(Ruja_Lexer *lexer) {
    lexer->current++;
}

/**
 * @brief Returns the current character of the lexer.
 * 
 * @param lexer The lexer to get the current character of.
 * @return char The current character of the lexer.
 */
static char peek(Ruja_Lexer *lexer) {
    return *(lexer->current);
}

/**
 * @brief Returns the character of the lexer at the given offset (in relation to the start position).
 * 
 * @param lexer The lexer to get the character of.
 * @param offset The offset of the character.
 * @return char The character of the lexer at the given offset.
 */
static char peek_offset(Ruja_Lexer *lexer, size_t offset) {
    return *(lexer->start + offset);
}

/**
 * @brief Returns the next character of the lexer.
 * 
 * @param lexer The lexer to get the next character of.
 * @return char The next character of the lexer.
 */
static char peek_next(Ruja_Lexer *lexer) {
    return *(lexer->current + 1);
}

/**
 * @brief Skips all whitespace and comments.
 * 
 * @param lexer The lexer to skip whitespace and comments of.
 */
static void skip_whitespace(Ruja_Lexer *lexer) {
    bool keep_going = true;
    while (keep_going) {
        switch (peek(lexer)) {
            case ' ' :
            case '\r':
            case '\t':
            case '\v':
            case '\f': {
                advance(lexer);
            } break;
            case '\n': {
                lexer->line++;
                advance(lexer);
            } break;
            case '/': { // It is not a space. Check if it is a comment
                if (peek_next(lexer) == '/') {
                    while (peek(lexer) != '\n' && peek(lexer) != '\0') {
                        advance(lexer);
                    }
                } else keep_going = false;
            } break;
            // TODO: Add support for multiline comments
            default: { keep_going = false; break; } // It is not a comment. Return
        }
    }

    rebase(lexer);
}

/**
 * @brief Matches a string with the current token.
 *  If the match is successful, it returns the token kind that was passed in key.
 *  If the match is unsuccessful, it returns the token kind RUJA_TOK_ID.
 *  
 * @param lexer The lexer instance to match the token of.
 * @param start The start of match.
 * @param length The length of the match.
 * @param expected The expected string to match.
 * @param key The token kind to return if the match is successful.
 * @return Ruja_Token_Kind The type of the token that was matched.
 */
static Ruja_Token_Kind match(Ruja_Lexer* lexer, size_t start, size_t length, const char* expected, Ruja_Token_Kind key) {
    for(size_t i = start; i < length; i++) {
        if(peek_offset(lexer, i) != expected[i - start]) {
            return RUJA_TOK_ID;
        }
    }
    return key;
}

/**
 * @brief Checks if the current token is a keyword.
 * 
 * @param lexer The lexer to check the current token of.
 * @return Ruja_Token_Kind The kind of the current token.
 */
static Ruja_Token_Kind id_v_keyword(Ruja_Lexer* lexer) {
    switch(peek_offset(lexer, 0)) {
        case 'a': { return match(lexer, 1, 2, "nd", RUJA_TOK_AND); break; }
        case 'e': {
            switch(peek_offset(lexer, 1)) {
                case 'l': {
                    switch(peek_offset(lexer, 2)) {
                        case 's': return match(lexer, 3, 1, "e", RUJA_TOK_ELSE); break;
                        case 'i': return match(lexer, 3, 1, "f", RUJA_TOK_ELIF); break;
                    }
                } break;
                case 'n': return match(lexer, 2, 2, "um", RUJA_TOK_ENUM); break;
            }
        } break;
        case 'f': {
            switch(peek_offset(lexer, 1)) {
                case 'a': return match(lexer, 2, 3, "lse", RUJA_TOK_FALSE); break;
                case 'o': return match(lexer, 2, 1, "r", RUJA_TOK_FOR); break;
            }
        } break;
        case 'i': {
            switch(peek_offset(lexer, 1)) {
                case 'f': return match(lexer, 2, 0, "", RUJA_TOK_IF); break;
                case 'n': return match(lexer, 2, 0, "", RUJA_TOK_IN); break;
                case '8': return match(lexer, 2, 0, "", RUJA_TOK_TYPE_I8); break;
            }
        } break;
        case 'l': { return match(lexer, 1, 2, "et", RUJA_TOK_LET); break; }
        case 'n': { return match(lexer, 1, 2, "ot", RUJA_TOK_NOT); break; }
        case 'o': { return match(lexer, 1, 2, "r", RUJA_TOK_OR); break; }
        case 'p': { return match(lexer, 1, 3, "roc", RUJA_TOK_PROC); break; }
        case 'r': { return match(lexer, 1, 5, "eturn", RUJA_TOK_RETURN); break; }
        case 's': { return match(lexer, 1, 5, "truct", RUJA_TOK_STRUCT); break; }
        case 't': { return match(lexer, 1, 3, "rue", RUJA_TOK_TRUE); break; }
    }

    return RUJA_TOK_ID;
}

/**
 * @brief Returns the next token of the lexer. Either an identifier or a keyword.
 * 
 * @param lexer The lexer to get the next token of.
 * @return Ruja_Token The next token of the lexer.
 */
static Ruja_Token tok_identifier(Ruja_Lexer *lexer) {
    // At this point we know that the first character is a letter or an underscore
    while (isalnum(peek(lexer)) || peek(lexer) == '_') {
        advance(lexer);
    }

    Ruja_Token result = {
        .kind = id_v_keyword(lexer),
        .start = lexer->start,
        .length = (size_t) (lexer->current - lexer->start),
        .line = lexer->line
    };

    rebase(lexer);
    return result;
}

/**
 * @brief Returns the next token of the lexer. Either an integer or a float.
 * 
 * @param lexer The lexer to get the next token of.
 * @return Ruja_Token The next token of the lexer.
 */
static Ruja_Token tok_number(Ruja_Lexer* lexer) {
    while (isdigit(peek(lexer))) advance(lexer);

    // Look for a fractional part
    if (peek(lexer) == '.' && isdigit(peek_next(lexer))) {
        // Consume the "."
        advance(lexer);

        while (isdigit(peek(lexer))) advance(lexer);

        Ruja_Token result = {
            .kind = RUJA_TOK_FLOAT,
            .start = lexer->start,
            .length = (size_t) (lexer->current - lexer->start),
            .line = lexer->line
        };

        rebase(lexer);
        return result;
    }

    Ruja_Token result = {
        .kind = RUJA_TOK_INT,
        .start = lexer->start,
        .length = (size_t) (lexer->current - lexer->start),
        .line = lexer->line
    };

    rebase(lexer);
    return result;
}

/**
 * @brief Returns a human readable string of the token kind.
 * 
 * @param kind 
 * @return char* 
 */
static char* token_kind_to_string(Ruja_Token_Kind kind) {
    switch (kind) {
        case RUJA_TOK_EOF        : return "EOF";
        case RUJA_TOK_LBRACE     : return "LBRACE";
        case RUJA_TOK_RBRACE     : return "RBRACE";
        case RUJA_TOK_LPAREN     : return "LPAREN";
        case RUJA_TOK_RPAREN     : return "RPAREN";
        case RUJA_TOK_LBRACKET   : return "LBRACKET";
        case RUJA_TOK_RBRACKET   : return "RBRACKET";
        case RUJA_TOK_COLON      : return "COLON";
        case RUJA_TOK_SEMICOLON  : return "SEMICOLON";
        case RUJA_TOK_COMMA      : return "COMMA";
        case RUJA_TOK_DOT        : return "DOT";
        case RUJA_TOK_ASSIGN     : return "ASSIGN";
        case RUJA_TOK_QUESTION   : return "QUESTION";
        case RUJA_TOK_NE         : return "NE";
        case RUJA_TOK_LT         : return "LT";
        case RUJA_TOK_GT         : return "GT";
        case RUJA_TOK_ARROW      : return "ARROW";
        case RUJA_TOK_ADD        : return "ADD";
        case RUJA_TOK_SUB        : return "SUB";
        case RUJA_TOK_MUL        : return "MUL";
        case RUJA_TOK_DIV        : return "DIV";
        case RUJA_TOK_PERCENT    : return "PERCENT";
        case RUJA_TOK_BANG       : return "BANG";
        case RUJA_TOK_EQ         : return "EQ";
        case RUJA_TOK_LE         : return "LE";
        case RUJA_TOK_GE         : return "GE";
        case RUJA_TOK_ADD_EQ     : return "ADD_EQ";
        case RUJA_TOK_SUB_EQ     : return "SUB_EQ";
        case RUJA_TOK_MUL_EQ     : return "MUL_EQ";
        case RUJA_TOK_DIV_EQ     : return "DIV_EQ";
        case RUJA_TOK_PERCENT_EQ : return "PERCENT_EQ";
        case RUJA_TOK_AND        : return "AND";
        case RUJA_TOK_OR         : return "OR";
        case RUJA_TOK_NOT        : return "NOT";
        case RUJA_TOK_IF         : return "IF";
        case RUJA_TOK_ELSE       : return "ELSE";
        case RUJA_TOK_ELIF       : return "ELIF";
        case RUJA_TOK_FOR        : return "FOR";
        case RUJA_TOK_IN         : return "IN";
        case RUJA_TOK_PROC       : return "PROC";
        case RUJA_TOK_RETURN     : return "RETURN";
        case RUJA_TOK_STRUCT     : return "STRUCT";
        case RUJA_TOK_ENUM       : return "ENUM";
        case RUJA_TOK_TRUE       : return "TRUE";
        case RUJA_TOK_FALSE      : return "FALSE";
        case RUJA_TOK_LET        : return "LET";
        case RUJA_TOK_TYPE_I8    : return "I8";
        case RUJA_TOK_ID         : return "ID";
        case RUJA_TOK_INT        : return "INT";
        case RUJA_TOK_FLOAT      : return "FLOAT";
        case RUJA_TOK_STRING     : return "STRING";
        case RUJA_TOK_CHAR       : return "CHAR";
        case RUJA_TOK_ERR        : return "ERROR";
        default                  : return "UNKNOWN";
    }
}

static char* read_file(const char* filepath){
    #define READ_ERROR(condition, msg) \
        if (condition) { \
            fprintf(stderr, msg" '%s': %s.\n", filepath, strerror(errno)); \
            if (buffer != NULL) { \
                free(buffer); \
            } \
            return NULL; \
        }

    char* buffer = NULL;
    FILE* file = fopen(filepath, "r");
    READ_ERROR(file == NULL, "Could not open file");

    READ_ERROR(fseek(file, 0L, SEEK_END) == -1, "Could not seek to end of file");

    long file_size = ftell(file);
    READ_ERROR(file_size == -1, "Could not get size of file");

    READ_ERROR(fseek(file, 0L, SEEK_SET) == -1, "Could not seek to start of file");

    buffer = malloc(file_size + 1);
    READ_ERROR(buffer == NULL, "Could not allocate memory for file");

    size_t new_len = fread(buffer, sizeof(char), (size_t) file_size, file);
    READ_ERROR(ferror(file) != 0, "Could not read file");
    buffer[new_len++] = '\0';

    READ_ERROR(fclose(file) == EOF, "Could not close file");

    #undef READ_ERROR
    return buffer;
}

static void err_lexer(Ruja_Lexer *lexer, Ruja_Token* token, const char* msg) {
    fprintf(stderr, "%s:%"PRIu64": "RED"lex error"RESET" %s '%.*s'.\n", lexer->source, token->line, msg, (int) token->length, token->start);
    token->kind = RUJA_TOK_ERR;
}

void token_to_string(Ruja_Token* token) {
    printf("Ruja_Token(%s,%.*s,%"PRIu64")\n", token_kind_to_string(token->kind), (int) token->length, token->start, token->line);
}

Ruja_Lexer* lexer_new(const char* filepath) {
    char* content = read_file(filepath);
    if (content == NULL) return NULL;

    Ruja_Lexer* lexer = malloc(sizeof(Ruja_Lexer));
    if (lexer == NULL) {
        fprintf(stderr, "Could not allocate memory for lexer: %s.\n", strerror(errno));
        free(content);
        return NULL;
    }

    lexer->source = filepath;
    lexer->content_start = content;
    lexer->start = content;
    lexer->current = content;
    lexer->line = 1;

    return lexer;
}

void lexer_free(Ruja_Lexer *lexer) {
    free(lexer->content_start);
    free(lexer);
}

Ruja_Token next_token(Ruja_Lexer *lexer) {
    skip_whitespace(lexer);

    if(isalpha(peek(lexer)) || peek(lexer) == '_')
        return tok_identifier(lexer);

    if(isdigit(peek(lexer)))
        return tok_number(lexer);

    Ruja_Token result = {
        .kind = RUJA_TOK_ERR,
        .start = lexer->start,
        .length = 1,
        .line = lexer->line
    };

    switch (peek(lexer)) {
        case '(' : { advance(lexer); result.kind = RUJA_TOK_LPAREN; } break;
        case ')' : { advance(lexer); result.kind = RUJA_TOK_RPAREN; } break;
        case '{' : { advance(lexer); result.kind = RUJA_TOK_LBRACE; } break;
        case '}' : { advance(lexer); result.kind = RUJA_TOK_RBRACE; } break;
        case '[' : { advance(lexer); result.kind = RUJA_TOK_LBRACKET; } break;
        case ']' : { advance(lexer); result.kind = RUJA_TOK_RBRACKET; } break;
        case ':' : { advance(lexer); result.kind = RUJA_TOK_COLON; } break;
        case ';' : { advance(lexer); result.kind = RUJA_TOK_SEMICOLON; } break;
        case ',' : { advance(lexer); result.kind = RUJA_TOK_COMMA; } break;
        case '.' : { advance(lexer); result.kind = RUJA_TOK_DOT; } break;
        case '?' : { advance(lexer); result.kind = RUJA_TOK_QUESTION; } break;
        case '=' : {
            advance(lexer);
            switch (peek(lexer)) {
                case '=' : { advance(lexer); result.kind = RUJA_TOK_EQ; result.length = 2;} break;
                default  : { result.kind = RUJA_TOK_ASSIGN; } break;
            }
        } break;
        case '<' : {
            advance(lexer);
            switch (peek(lexer)) {
                case '=' : { advance(lexer); result.kind = RUJA_TOK_LE; result.length = 2; } break;
                default  : { result.kind = RUJA_TOK_LT; } break;
            }
        } break;
        case '>' : {
            advance(lexer);
            switch (peek(lexer)) {
                case '=' : { advance(lexer); result.kind = RUJA_TOK_GE; result.length = 2; } break;
                default  : { result.kind = RUJA_TOK_GT; } break;
            }
        } break;
        case '+' : {
            advance(lexer);
            switch (peek(lexer)) {
                case '=' : { advance(lexer); result.kind = RUJA_TOK_ADD_EQ; result.length = 2; } break;
                default  : { result.kind = RUJA_TOK_ADD; } break;
            }
        } break;
        case '-' : {
            advance(lexer);
            switch (peek(lexer)) {
                case '=' : { advance(lexer); result.kind = RUJA_TOK_SUB_EQ; result.length = 2; } break;
                case '>' : { advance(lexer); result.kind = RUJA_TOK_ARROW; result.length = 2; } break;
                default  : { result.kind = RUJA_TOK_SUB;} break;
            }
        } break;
        case '*' : {
            advance(lexer);
            switch (peek(lexer)) {
                case '=' : { advance(lexer); result.kind = RUJA_TOK_MUL_EQ; result.length = 2; } break;
                default  : { result.kind = RUJA_TOK_MUL; } break;
            }
        } break;
        case '/' : {
            advance(lexer);
            switch (peek(lexer)) {
                case '=' : { advance(lexer); result.kind = RUJA_TOK_DIV_EQ; result.length = 2; } break;
                default  : { result.kind = RUJA_TOK_DIV; } break;
            }
        } break;
        case '%' : {
            advance(lexer);
            switch (peek(lexer)) {
                case '=' : { advance(lexer); result.kind = RUJA_TOK_PERCENT_EQ; result.length = 2; } break;
                default  : { result.kind = RUJA_TOK_PERCENT; } break;
            }
        } break;
        case '!' : {
            advance(lexer);
            switch (peek(lexer)) {
                case '=' : { advance(lexer); result.kind = RUJA_TOK_NE; result.length = 2; } break;
                default  : { err_lexer(lexer, &result, "Unrecognized token"); } break;
            }
        } break;
        case '"': {
            advance(lexer);
            result.kind = RUJA_TOK_STRING;
            result.start = lexer->current;
            while (peek(lexer) != '"' && peek(lexer) != '\0') {
                if (peek(lexer) == '\n') lexer->line++;
                advance(lexer);
            }
            result.length = lexer->current - result.start;
            if (peek(lexer) == '\0') err_lexer(lexer, &result, "Unterminated string");
            else advance(lexer);
        } break;
        case '\0': { result.kind = RUJA_TOK_EOF; result.length = 0; } break;
        default: { err_lexer(lexer, &result, "Unrecognized token"); } break;
    }

    rebase(lexer);
    return result;
}