#include <stdio.h>
#include <ctype.h>

#include "lexer.h"

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
 * @brief Returns the next next character of the lexer.
 * 
 * @param lexer The lexer to get the next next character of.
 * @return char The next next character of the lexer.
 */
static char peek_next_next(Ruja_Lexer *lexer) {
    return *(lexer->current + 2);
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

static char* token_kind_to_string(Ruja_Token_Kind kind) {
    switch (kind) {
        case RUJA_TOK_EOF           : return "EOF";
        case RUJA_TOK_LBRACE        : return "LBRACE";
        case RUJA_TOK_RBRACE        : return "RBRACE";
        case RUJA_TOK_LPAREN        : return "LPAREN";
        case RUJA_TOK_RPAREN        : return "RPAREN";
        case RUJA_TOK_LBRACKET      : return "LBRACKET";
        case RUJA_TOK_RBRACKET      : return "RBRACKET";
        case RUJA_TOK_COLON         : return "COLON";
        case RUJA_TOK_SEMICOLON     : return "SEMICOLON";
        case RUJA_TOK_COMMA         : return "COMMA";
        case RUJA_TOK_DOT           : return "DOT";
        case RUJA_TOK_EQUAL         : return "EQUAL";
        case RUJA_TOK_NE            : return "NE";
        case RUJA_TOK_LT            : return "LT";
        case RUJA_TOK_GT            : return "GT";
        case RUJA_TOK_ARROW         : return "ARROW";
        case RUJA_TOK_ADD           : return "ADD";
        case RUJA_TOK_SUB           : return "SUB";
        case RUJA_TOK_MUL           : return "MUL";
        case RUJA_TOK_DIV           : return "DIV";
        case RUJA_TOK_PERCENT       : return "PERCENT";
        case RUJA_TOK_BANG          : return "BANG";
        case RUJA_TOK_EQ            : return "EQ";
        case RUJA_TOK_LE            : return "LE";
        case RUJA_TOK_GE            : return "GE";
        case RUJA_TOK_ADD_EQ        : return "ADD_EQ";
        case RUJA_TOK_SUB_EQ        : return "SUB_EQ";
        case RUJA_TOK_MUL_EQ        : return "MUL_EQ";
        case RUJA_TOK_DIV_EQ        : return "DIV_EQ";
        case RUJA_TOK_PERCENT_EQ    : return "PERCENT_EQ";
        case RUJA_TOK_AND           : return "AND";
        case RUJA_TOK_OR            : return "OR";
        case RUJA_TOK_NOT           : return "NOT";
        case RUJA_TOK_IF            : return "IF";
        case RUJA_TOK_ELSE          : return "ELSE";
        case RUJA_TOK_ELIF          : return "ELIF";
        case RUJA_TOK_FOR           : return "FOR";
        case RUJA_TOK_IN            : return "IN";
        case RUJA_TOK_PROC          : return "PROC";
        case RUJA_TOK_RETURN        : return "RETURN";
        case RUJA_TOK_STRUCT        : return "STRUCT";
        case RUJA_TOK_ENUM          : return "ENUM";
        case RUJA_TOK_TRUE          : return "TRUE";
        case RUJA_TOK_FALSE         : return "FALSE";
        case RUJA_TOK_LET           : return "LET";
        case RUJA_TOK_ID            : return "ID";
        case RUJA_TOK_INT           : return "INT";
        case RUJA_TOK_FLOAT         : return "FLOAT";
        case RUJA_TOK_STRING        : return "STRING";
        case RUJA_TOK_CHAR          : return "CHAR";
        case RUJA_TOK_ERR_UNRECOGNIZED:
        case RUJA_TOK_ERR_UNTERMINATED_STRING:
        default                     : return "UNKNOWN";
    }
}

void token_to_string(Ruja_Token* token) {
    #define RED "\x1b[31m"
    #define RESET "\x1b[0m"
    if (token->kind == RUJA_TOK_ERR_UNRECOGNIZED) {
        printf(RED "[Lexer error]" RESET " at line %"PRIu64":\n", token->line);
        printf("\tUnrecognized token: '%.*s'\n", (int) token->length, token->start);
    } else if (token->kind == RUJA_TOK_ERR_UNTERMINATED_STRING) {
        printf(RED "[Lexer error]" RESET " at line %"PRIu64":\n", token->line);
        printf("\tUnterminated string literal: '%.*s'\n", (int) token->length, token->start);
    } else printf("Ruja_Token(%s,%.*s,%"PRIu64")\n", token_kind_to_string(token->kind), (int) token->length, token->start, token->line);
    #undef RED
    #undef RESET
}

// TODO: This is a redundant solution. Change it to a better one.
Ruja_Lexer lexer_new(char *content) {
    return (Ruja_Lexer) {
        .start = content,
        .current = content,
        .line = 1
    };
}

Ruja_Token next_token(Ruja_Lexer *lexer) {
    skip_whitespace(lexer);

    if(isalpha(peek(lexer)) || peek(lexer) == '_')
        return tok_identifier(lexer);

    if(isdigit(peek(lexer)))
        return tok_number(lexer);

    Ruja_Token result = {
        .kind = RUJA_TOK_ERR_UNRECOGNIZED,
        .start = lexer->start,
        .length = 0,
        .line = lexer->line
    };

    switch (peek(lexer)) {
        case '(' : { advance(lexer); result.kind = RUJA_TOK_LPAREN; result.length = 1; } break;
        case ')' : { advance(lexer); result.kind = RUJA_TOK_RPAREN; result.length = 1; } break;
        case '{' : { advance(lexer); result.kind = RUJA_TOK_LBRACE; result.length = 1; } break;
        case '}' : { advance(lexer); result.kind = RUJA_TOK_RBRACE; result.length = 1; } break;
        case '[' : { advance(lexer); result.kind = RUJA_TOK_LBRACKET; result.length = 1; } break;
        case ']' : { advance(lexer); result.kind = RUJA_TOK_RBRACKET; result.length = 1; } break;
        case ':' : { advance(lexer); result.kind = RUJA_TOK_COLON; result.length = 1; } break;
        case ';' : { advance(lexer); result.kind = RUJA_TOK_SEMICOLON; result.length = 1; } break;
        case ',' : { advance(lexer); result.kind = RUJA_TOK_COMMA; result.length = 1; } break;
        case '.' : { advance(lexer); result.kind = RUJA_TOK_DOT; result.length = 1; } break;
        case '=' : {
            advance(lexer);
            switch (peek(lexer)) {
                case '=' : { advance(lexer); result.kind = RUJA_TOK_EQ; result.length = 2;} break;
                default  : { result.kind = RUJA_TOK_EQUAL; result.length = 1; } break;
            }
        } break;
        case '<' : {
            advance(lexer);
            switch (peek(lexer)) {
                case '=' : { advance(lexer); result.kind = RUJA_TOK_LE; result.length = 2; } break;
                default  : { result.kind = RUJA_TOK_LT; result.length = 1; } break;
            }
        } break;
        case '>' : {
            advance(lexer);
            switch (peek(lexer)) {
                case '=' : { advance(lexer); result.kind = RUJA_TOK_GE; result.length = 2; } break;
                default  : { result.kind = RUJA_TOK_GT; result.length = 1; } break;
            }
        } break;
        case '+' : {
            advance(lexer);
            switch (peek(lexer)) {
                case '=' : { advance(lexer); result.kind = RUJA_TOK_ADD_EQ; result.length = 2; } break;
                default  : { result.kind = RUJA_TOK_ADD; result.length = 1; } break;
            }
        } break;
        case '-' : {
            advance(lexer);
            switch (peek(lexer)) {
                case '=' : { advance(lexer); result.kind = RUJA_TOK_SUB_EQ; result.length = 2; } break;
                case '>' : { advance(lexer); result.kind = RUJA_TOK_ARROW; result.length = 2; } break;
                default  : { result.kind = RUJA_TOK_SUB; result.length = 1;} break;
            }
        } break;
        case '*' : {
            advance(lexer);
            switch (peek(lexer)) {
                case '=' : { advance(lexer); result.kind = RUJA_TOK_MUL_EQ; result.length = 2; } break;
                default  : { result.kind = RUJA_TOK_MUL; result.length = 1; } break;
            }
        } break;
        case '/' : {
            advance(lexer);
            switch (peek(lexer)) {
                case '=' : { advance(lexer); result.kind = RUJA_TOK_DIV_EQ; result.length = 2; } break;
                default  : { result.kind = RUJA_TOK_DIV; result.length = 1; } break;
            }
        } break;
        case '%' : {
            advance(lexer);
            switch (peek(lexer)) {
                case '=' : { advance(lexer); result.kind = RUJA_TOK_PERCENT_EQ; result.length = 2; } break;
                default  : { result.kind = RUJA_TOK_PERCENT; result.length = 1; } break;
            }
        } break;
        case '!' : {
            advance(lexer);
            switch (peek(lexer)) {
                case '=' : { advance(lexer); result.kind = RUJA_TOK_NE; result.length = 2; } break;
                default  : { result.kind = RUJA_TOK_ERR_UNRECOGNIZED; result.length = 1; } break;
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
            if (peek(lexer) == '\0') result.kind = RUJA_TOK_ERR_UNTERMINATED_STRING;
            else advance(lexer);
        } break;
        case '\0': { result.kind = RUJA_TOK_EOF; result.length = 0; } break;
        default: { result.kind = RUJA_TOK_ERR_UNRECOGNIZED; result.length = 1; } break;
    }

    rebase(lexer);
    return result;
}