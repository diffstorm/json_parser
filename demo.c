/**
    @brief JSON Parser Library

    A lightweight, single-header C library for parsing JSON data. Designed for simplicity and portability, this parser provides a low-footprint solution to decode JSON-formatted strings into structured tokens while adhering to core JSON specifications.

    @date 2025-05-03
    @version 1.0
    @author Eray Ozturk | erayozturk1@gmail.com
    @url github.com/diffstorm
    @license MIT License
*/

#include <stdio.h>
#include <string.h>
#define JSON_PARSER_IMPLEMENTATION
#include "json_parser.h"

int main() {
    const char *json = "{\"name\":\"John\\u00D0e\",\"age\":30,\"scores\":[90.5,80.0]}";
    json_parser_t parser;
    json_parser_init(&parser, json, strlen(json));
    
    json_error_t error = json_parser_parse(&parser);
    if (error != JSON_ERROR_NONE) {
        printf("Error: %s\n", json_error_string(error));
        return 1;
    }
    
    size_t count;
    const json_token_t *tokens = json_get_tokens(&parser, &count);
    for (size_t i = 0; i < count; i++) {
        const json_token_t *t = &tokens[i];
        printf("Token %zu: ", i);
        switch (t->type) {
            case JSON_TOKEN_OBJECT:
                printf("Object\n"); break;
            case JSON_TOKEN_ARRAY:
                printf("Array\n"); break;
            case JSON_TOKEN_STRING:
                printf("String: %s\n", t->value.string); break;
            case JSON_TOKEN_NUMBER:
                printf("Number: %f\n", t->value.number); break;
            case JSON_TOKEN_TRUE:
                printf("Boolean: true\n"); break;
            case JSON_TOKEN_FALSE:
                printf("Boolean: false\n"); break;
            case JSON_TOKEN_NULL:
                printf("Null\n"); break;
            default:
                printf("Unknown\n");
        }
    }
    
    json_parser_free(&parser);
    return 0;
}
