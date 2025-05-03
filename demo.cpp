/**
    @brief JSON Parser Library

    A lightweight, single-header C library for parsing JSON data. Designed for simplicity and portability, this parser provides a low-footprint solution to decode JSON-formatted strings into structured tokens while adhering to core JSON specifications.

    @date 2025-05-03
    @version 1.0
    @author Eray Ozturk | erayozturk1@gmail.com
    @url github.com/diffstorm
    @license MIT License
*/

#include <iostream>
#include <string>

#include "json_parser.h"

int main() {
    // Use raw string literal to avoid escaping characters
    const std::string json = R"({"name":"John\u00D0e","age":30,"scores":[90.5,80.0]})";
    
    json_parser_t parser;
    json_parser_init(&parser, json.data(), json.size());
    
    const json_error_t error = json_parser_parse(&parser);
    if (error != JSON_ERROR_NONE) {
        std::cerr << "Error: " << json_error_string(error) << "\n";
        json_parser_free(&parser);
        return 1;
    }
    
    size_t token_count = 0;
    const json_token_t* tokens = json_get_tokens(&parser, &token_count);
    
    for (size_t i = 0; i < token_count; ++i) {
        const auto& token = tokens[i];
        std::cout << "Token " << i << ": ";
        
        switch (token.type) {
            case JSON_TOKEN_OBJECT:
                std::cout << "Object\n";
                break;
            case JSON_TOKEN_ARRAY:
                std::cout << "Array\n";
                break;
            case JSON_TOKEN_STRING:
                std::cout << "String: " << token.value.string << "\n";
                break;
            case JSON_TOKEN_NUMBER:
                std::cout << "Number: " << token.value.number << "\n";
                break;
            case JSON_TOKEN_TRUE:
                std::cout << "Boolean: true\n";
                break;
            case JSON_TOKEN_FALSE:
                std::cout << "Boolean: false\n";
                break;
            case JSON_TOKEN_NULL:
                std::cout << "Null\n";
                break;
            default:
                std::cout << "Unknown\n";
        }
    }
    
    json_parser_free(&parser);
    return 0;
}