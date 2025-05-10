/**
    @brief JSON Parser Library

    A lightweight, single-header C library for parsing JSON data. Designed for simplicity and portability, this parser provides a low-footprint solution to decode JSON-formatted strings into structured tokens while adhering to core JSON specifications.

    @date 2025-05-03
    @version 1.0
    @author Eray Ozturk | erayozturk1@gmail.com
    @url github.com/diffstorm
    @license MIT License
*/
#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#ifndef JSON_DEFAULT_MAX_TOKENS
#define JSON_DEFAULT_MAX_TOKENS 128
#endif

#ifndef JSON_DEFAULT_MAX_DEPTH
#define JSON_DEFAULT_MAX_DEPTH 32
#endif

#ifndef JSON_DEFAULT_MAX_STRING
#define JSON_DEFAULT_MAX_STRING 256
#endif

typedef enum
{
    JSON_ERROR_NONE = 0,
    JSON_ERROR_INVALID_TOKEN,
    JSON_ERROR_UNEXPECTED_CHAR,
    JSON_ERROR_MAX_TOKENS,
    JSON_ERROR_STRING_TOO_LONG,
    JSON_ERROR_INVALID_ESCAPE,
    JSON_ERROR_INVALID_UNICODE,
    JSON_ERROR_NESTING_DEPTH,
    JSON_ERROR_INVALID_NUMBER,
    JSON_ERROR_TRAILING_CHARS,
    JSON_ERROR_ALLOCATION_FAILED
} json_error_t;

typedef enum
{
    JSON_TOKEN_INVALID,
    JSON_TOKEN_OBJECT,
    JSON_TOKEN_ARRAY,
    JSON_TOKEN_STRING,
    JSON_TOKEN_NUMBER,
    JSON_TOKEN_TRUE,
    JSON_TOKEN_FALSE,
    JSON_TOKEN_NULL
} json_token_type_t;

typedef struct
{
    json_token_type_t type;
    union
    {
        char *string;
        double number;
    } value;
    size_t start;
    size_t end;
} json_token_t;

typedef struct
{
    const char *json;
    size_t length;
    size_t pos;

    json_token_t *tokens;
    size_t token_count;
    size_t token_cap;

    size_t max_depth;
    size_t max_string;

    json_error_t error;
    int depth;
} json_parser_t;

// Initialization and cleanup
void json_parser_init(json_parser_t *parser, const char *json, size_t length);
void json_parser_free(json_parser_t *parser);
json_error_t json_parser_parse(json_parser_t *parser);

// Utility functions
const char *json_error_string(json_error_t error);
const json_token_t *json_get_tokens(const json_parser_t *parser, size_t *count);

#ifdef __cplusplus
}
#endif

#endif /* JSON_PARSER_H */

/*******************************
    Implementation
 *******************************/

#ifdef JSON_PARSER_IMPLEMENTATION

static void json_set_error(json_parser_t *parser, json_error_t error)
{
    if(parser->error == JSON_ERROR_NONE)
    {
        parser->error = error;
    }
}

static int json_parse_value(json_parser_t *parser);

void json_parser_init(json_parser_t *parser, const char *json, size_t length)
{
    memset(parser, 0, sizeof(*parser));
    parser->json = json;
    parser->length = length;
    parser->token_cap = JSON_DEFAULT_MAX_TOKENS;
    parser->max_depth = JSON_DEFAULT_MAX_DEPTH;
    parser->max_string = JSON_DEFAULT_MAX_STRING;
    parser->tokens = malloc(parser->token_cap * sizeof(json_token_t));

    if(!parser->tokens)
    {
        parser->error = JSON_ERROR_ALLOCATION_FAILED;
    }
}

void json_parser_free(json_parser_t *parser)
{
    for(size_t i = 0; i < parser->token_count; i++)
    {
        if(parser->tokens[i].type == JSON_TOKEN_STRING)
        {
            free(parser->tokens[i].value.string);
        }
    }

    free(parser->tokens);
    memset(parser, 0, sizeof(*parser));
}

const char *json_error_string(json_error_t error)
{
    switch(error)
    {
        case JSON_ERROR_NONE:
            return "No error";

        case JSON_ERROR_INVALID_TOKEN:
            return "Invalid token";

        case JSON_ERROR_UNEXPECTED_CHAR:
            return "Unexpected character";

        case JSON_ERROR_MAX_TOKENS:
            return "Max tokens exceeded";

        case JSON_ERROR_STRING_TOO_LONG:
            return "String too long";

        case JSON_ERROR_INVALID_ESCAPE:
            return "Invalid escape sequence";

        case JSON_ERROR_INVALID_UNICODE:
            return "Invalid Unicode escape";

        case JSON_ERROR_NESTING_DEPTH:
            return "Nesting depth exceeded";

        case JSON_ERROR_INVALID_NUMBER:
            return "Invalid number format";

        case JSON_ERROR_TRAILING_CHARS:
            return "Trailing characters";

        case JSON_ERROR_ALLOCATION_FAILED:
            return "Memory allocation failed";

        default:
            return "Unknown error";
    }
}

const json_token_t *json_get_tokens(const json_parser_t *parser, size_t *count)
{
    if(count)
    {
        *count = parser->token_count;
    }

    return parser->tokens;
}

static void json_skip_whitespace(json_parser_t *parser)
{
    while(parser->pos < parser->length && isspace(parser->json[parser->pos]))
    {
        parser->pos++;
    }
}

static int json_add_token(json_parser_t *parser, json_token_type_t type)
{
    if(parser->token_count >= parser->token_cap)
    {
        size_t new_cap = parser->token_cap * 2;
        json_token_t *new_tokens = realloc(parser->tokens, new_cap * sizeof(json_token_t));

        if(!new_tokens)
        {
            json_set_error(parser, JSON_ERROR_ALLOCATION_FAILED);
            return -1;
        }

        parser->tokens = new_tokens;
        parser->token_cap = new_cap;
    }

    json_token_t *token = &parser->tokens[parser->token_count++];
    memset(token, 0, sizeof(*token));
    token->type = type;
    token->start = parser->pos;
    token->end = parser->pos;
    return 0;
}

static int json_parse_unicode_escape(json_parser_t *parser, char *buf, size_t *idx)
{
    if(parser->pos + 4 > parser->length)
    {
        return -1;
    }

    unsigned int codepoint = 0;

    for(int i = 0; i < 4; i++)
    {
        char c = parser->json[parser->pos++];
        codepoint <<= 4;

        if(c >= '0' && c <= '9')
        {
            codepoint += c - '0';
        }
        else if(c >= 'a' && c <= 'f')
        {
            codepoint += c - 'a' + 10;
        }
        else if(c >= 'A' && c <= 'F')
        {
            codepoint += c - 'A' + 10;
        }
        else
        {
            return -1;
        }
    }

    // Check if it's a high surrogate (D800-DBFF)
    if(codepoint >= 0xD800 && codepoint <= 0xDBFF)
    {
        // Expect a low surrogate (DC00-DFFF) next
        if(parser->pos + 6 > parser->length ||
                parser->json[parser->pos] != '\\' ||
                parser->json[parser->pos + 1] != 'u')
        {
            return -1; // Missing low surrogate
        }

        parser->pos += 2; // Skip "\u"
        unsigned int low_surrogate = 0;

        for(int i = 0; i < 4; i++)
        {
            char c = parser->json[parser->pos++];
            low_surrogate <<= 4;

            if(c >= '0' && c <= '9')
            {
                low_surrogate += c - '0';
            }
            else if(c >= 'a' && c <= 'f')
            {
                low_surrogate += c - 'a' + 10;
            }
            else if(c >= 'A' && c <= 'F')
            {
                low_surrogate += c - 'A' + 10;
            }
            else
            {
                return -1;
            }
        }

        if(low_surrogate < 0xDC00 || low_surrogate > 0xDFFF)
        {
            return -1; // Invalid low surrogate
        }

        // Combine into 32-bit codepoint
        codepoint = 0x10000 + ((codepoint - 0xD800) << 10) + (low_surrogate - 0xDC00);
    }
    else if(codepoint >= 0xDC00 && codepoint <= 0xDFFF)
    {
        return -1; // Unpaired low surrogate
    }

    // UTF-8 encoding for codepoints > 0xFFFF
    if(codepoint <= 0x7F)
    {
        if(*idx >= parser->max_string - 1)
        {
            return -1;
        }

        buf[(*idx)++] = codepoint;
    }
    else if(codepoint <= 0x7FF)
    {
        if(*idx >= parser->max_string - 2)
        {
            return -1;
        }

        buf[(*idx)++] = 0xC0 | (codepoint >> 6);
        buf[(*idx)++] = 0x80 | (codepoint & 0x3F);
    }
    else if(codepoint <= 0xFFFF)
    {
        if(*idx >= parser->max_string - 3)
        {
            return -1;
        }

        buf[(*idx)++] = 0xE0 | (codepoint >> 12);
        buf[(*idx)++] = 0x80 | ((codepoint >> 6) & 0x3F);
        buf[(*idx)++] = 0x80 | (codepoint & 0x3F);
    }
    else
    {
        if(*idx >= parser->max_string - 4)
        {
            return -1;
        }

        buf[(*idx)++] = 0xF0 | (codepoint >> 18);
        buf[(*idx)++] = 0x80 | ((codepoint >> 12) & 0x3F);
        buf[(*idx)++] = 0x80 | ((codepoint >> 6) & 0x3F);
        buf[(*idx)++] = 0x80 | (codepoint & 0x3F);
    }

    return 0;
}

static int json_parse_string(json_parser_t *parser)
{
    if(json_add_token(parser, JSON_TOKEN_STRING))
    {
        return -1;
    }

    json_token_t *token = &parser->tokens[parser->token_count - 1];
    token->start = parser->pos + 1;
    char *buffer = malloc(parser->max_string);

    if(!buffer)
    {
        json_set_error(parser, JSON_ERROR_ALLOCATION_FAILED);
        return -1;
    }

    size_t idx = 0;
    parser->pos++; // Skip opening quote

    while(parser->pos < parser->length)
    {
        char c = parser->json[parser->pos++];

        if(c == '"')
        {
            token->end = parser->pos - 1;
            buffer[idx] = '\0';
            token->value.string = buffer;
            return 0;
        }

        if(c == '\\')
        {
            if(parser->pos >= parser->length)
            {
                break;
            }

            c = parser->json[parser->pos++];

            switch(c)
            {
                case '"':
                    c = '"';
                    break;

                case '\\':
                    c = '\\';
                    break;

                case '/':
                    c = '/';
                    break;

                case 'b':
                    c = '\b';
                    break;

                case 'f':
                    c = '\f';
                    break;

                case 'n':
                    c = '\n';
                    break;

                case 'r':
                    c = '\r';
                    break;

                case 't':
                    c = '\t';
                    break;

                case 'u':
                    if(json_parse_unicode_escape(parser, buffer, &idx))
                    {
                        free(buffer);
                        json_set_error(parser, JSON_ERROR_INVALID_UNICODE);
                        return -1;
                    }

                    continue;

                default:
                    free(buffer);
                    json_set_error(parser, JSON_ERROR_INVALID_ESCAPE);
                    return -1;
            }
        }

        if(idx >= parser->max_string - 1)
        {
            free(buffer);
            json_set_error(parser, JSON_ERROR_STRING_TOO_LONG);
            return -1;
        }

        buffer[idx++] = c;
    }

    free(buffer);
    json_set_error(parser, JSON_ERROR_UNEXPECTED_CHAR);
    return -1;
}

static int json_parse_number(json_parser_t *parser)
{
    const char *start = parser->json + parser->pos;
    const char *p = start;
    int has_sign = 0;
    int has_integer = 0;
    int has_fraction = 0;
    int has_exponent = 0;

    // Check optional sign
    if(*p == '-')
    {
        has_sign = 1;
        p++;
    }

    // Validate integer part
    if(*p == '0')
    {
        p++;
        has_integer = 1;
    }
    else if(isdigit(*p))
    {
        has_integer = 1;
        p++;

        while(isdigit(*p))
        {
            p++;
        }
    }
    else
    {
        json_set_error(parser, JSON_ERROR_INVALID_NUMBER);
        return -1;
    }

    // Validate fractional part
    if(*p == '.')
    {
        p++;

        if(!isdigit(*p))
        {
            json_set_error(parser, JSON_ERROR_INVALID_NUMBER);
            return -1;
        }

        has_fraction = 1;
        p++;

        while(isdigit(*p))
        {
            p++;
        }
    }

    // Validate exponent part
    if(*p == 'e' || *p == 'E')
    {
        p++;

        if(*p == '+' || *p == '-')
        {
            p++;
        }

        if(!isdigit(*p))
        {
            json_set_error(parser, JSON_ERROR_INVALID_NUMBER);
            return -1;
        }

        has_exponent = 1;
        p++;

        while(isdigit(*p))
        {
            p++;
        }
    }

    // Verify the entire number was parsed correctly
    char *end;
    double num = strtod(start, &end);

    if(end != p)
    {
        json_set_error(parser, JSON_ERROR_INVALID_NUMBER);
        return -1;
    }

    if(json_add_token(parser, JSON_TOKEN_NUMBER))
    {
        return -1;
    }

    json_token_t *token = &parser->tokens[parser->token_count - 1];
    token->value.number = num;
    token->start = parser->pos;
    parser->pos += (p - start);
    token->end = parser->pos;
    return 0;
}

static int json_parse_literal(json_parser_t *parser, const char *literal, json_token_type_t type)
{
    size_t len = strlen(literal);

    if(parser->pos + len > parser->length)
    {
        return -1;
    }

    if(strncmp(parser->json + parser->pos, literal, len))
    {
        return -1;
    }

    parser->pos += len;

    if(json_add_token(parser, type))
    {
        return -1;
    }

    return 0;
}

static int json_parse_array(json_parser_t *parser);
static int json_parse_object(json_parser_t *parser);

static int json_parse_value(json_parser_t *parser)
{
    json_skip_whitespace(parser);

    if(parser->pos >= parser->length)
    {
        return -1;
    }

    char c = parser->json[parser->pos];

    switch(c)
    {
        case '{':
            return json_parse_object(parser);

        case '[':
            return json_parse_array(parser);

        case '"':
            return json_parse_string(parser);

        case 't':
            return json_parse_literal(parser, "true", JSON_TOKEN_TRUE);

        case 'f':
            return json_parse_literal(parser, "false", JSON_TOKEN_FALSE);

        case 'n':
            return json_parse_literal(parser, "null", JSON_TOKEN_NULL);

        default:
            if(c == '-' || isdigit(c) || c == '.')
            {
                return json_parse_number(parser);
            }
            else
            {
                json_set_error(parser, JSON_ERROR_INVALID_TOKEN);
                return -1;
            }
    }
}

static int json_parse_object(json_parser_t *parser)
{
    if(parser->depth >= parser->max_depth)
    {
        json_set_error(parser, JSON_ERROR_NESTING_DEPTH);
        return -1;
    }

    size_t start_pos = parser->pos;
    parser->depth++;

    if(json_add_token(parser, JSON_TOKEN_OBJECT) < 0)
    {
        return -1;
    }

    json_token_t *obj_token = &parser->tokens[parser->token_count - 1];
    obj_token->start = start_pos;
    obj_token->end = 0;
    parser->pos++; // Skip '{'
    json_skip_whitespace(parser);

    if(parser->json[parser->pos] == '}')
    {
        parser->pos++;
        obj_token->end = parser->pos; // Set end after closing '}'
        parser->depth--;
        return 0;
    }

    while(parser->pos < parser->length)
    {
        json_skip_whitespace(parser);

        if(parser->pos >= parser->length)
        {
            break;
        }

        if(parser->json[parser->pos] == '}')
        {
            parser->pos++;
            obj_token->end = parser->pos;
            parser->depth--;
            return 0;
        }

        if(parser->json[parser->pos] != '"')
        {
            json_set_error(parser, JSON_ERROR_UNEXPECTED_CHAR);
            return -1;
        }

        if(json_parse_string(parser))
        {
            return -1;
        }

        json_skip_whitespace(parser);

        if(parser->json[parser->pos++] != ':')
        {
            json_set_error(parser, JSON_ERROR_UNEXPECTED_CHAR);
            return -1;
        }

        if(json_parse_value(parser))
        {
            return -1;
        }

        json_skip_whitespace(parser);

        if(parser->json[parser->pos] == '}')
        {
            continue;
        }

        if(parser->json[parser->pos++] != ',')
        {
            json_set_error(parser, JSON_ERROR_UNEXPECTED_CHAR);
            return -1;
        }
    }

    json_set_error(parser, JSON_ERROR_UNEXPECTED_CHAR);
    return -1;
}

static int json_parse_array(json_parser_t *parser)
{
    if(parser->depth >= parser->max_depth)
    {
        json_set_error(parser, JSON_ERROR_NESTING_DEPTH);
        return -1;
    }

    size_t start_pos = parser->pos; // Position of '['
    parser->depth++;

    if(json_add_token(parser, JSON_TOKEN_ARRAY) < 0)
    {
        return -1;
    }

    json_token_t *arr_token = &parser->tokens[parser->token_count - 1];
    arr_token->start = start_pos; // Start at '['
    arr_token->end = 0;
    parser->pos++; // Skip '['
    json_skip_whitespace(parser);

    if(parser->json[parser->pos] == ']')
    {
        parser->pos++;
        arr_token->end = parser->pos; // End after ']'
        parser->depth--;
        return 0;
    }

    while(parser->pos < parser->length)
    {
        json_skip_whitespace(parser);

        if(parser->pos >= parser->length)
        {
            break;
        }

        // Check for closing bracket before parsing next element
        if(parser->json[parser->pos] == ']')
        {
            parser->pos++;
            arr_token->end = parser->pos; // Set end after ']'
            parser->depth--;
            return 0;
        }

        // Parse the array element
        if(json_parse_value(parser))
        {
            return -1;
        }

        json_skip_whitespace(parser);

        // Check for comma or closing bracket after the element
        if(parser->json[parser->pos] == ']')
        {
            // Handle closing bracket in the next iteration
            continue;
        }
        else if(parser->json[parser->pos] == ',')
        {
            parser->pos++; // Consume the comma
            json_skip_whitespace(parser);

            // Trailing comma check: if next character is ']', it's invalid
            if(parser->json[parser->pos] == ']')
            {
                json_set_error(parser, JSON_ERROR_UNEXPECTED_CHAR);
                return -1;
            }
        }
        else
        {
            // Neither comma nor closing bracket
            json_set_error(parser, JSON_ERROR_UNEXPECTED_CHAR);
            return -1;
        }
    }

    json_set_error(parser, JSON_ERROR_UNEXPECTED_CHAR);
    return -1;
}

json_error_t json_parser_parse(json_parser_t *parser)
{
    parser->error = JSON_ERROR_NONE;
    json_skip_whitespace(parser);

    if(parser->pos >= parser->length)
    {
        json_set_error(parser, JSON_ERROR_INVALID_TOKEN);
        return parser->error;
    }

    if(json_parse_value(parser) < 0)
    {
        return parser->error;
    }

    json_skip_whitespace(parser);

    if(parser->pos != parser->length)
    {
        json_set_error(parser, JSON_ERROR_TRAILING_CHARS);
    }

    return parser->error;
}

#endif /* JSON_PARSER_IMPLEMENTATION */
