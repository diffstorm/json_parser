# JSON Parser Library

A lightweight, single-header C library for parsing JSON data. Designed for simplicity and portability, this parser provides a low-footprint solution to decode JSON-formatted strings into structured tokens while adhering to core JSON specifications.

## Features

- **Standard Compliance**: Supports parsing of JSON objects, arrays, strings, numbers, and literals (`true`, `false`, `null`).
- **Unicode Support**: Handles UTF-16 surrogate pairs and encodes Unicode escape sequences into valid UTF-8.
- **Configurable Limits**: Tunable thresholds for maximum nesting depth, token count, and string length.
- **Error Reporting**: Detailed error codes and human-readable error messages for troubleshooting parsing issues.
- **Memory Safety**: Cleanup functions ensure allocated resources are properly released.

## Components

### Core Structures
- **`json_parser_t`**: Manages the parser state, including input data, token storage, and error tracking.
- **`json_token_t`**: Represents a parsed JSON token, storing its type (object, array, string, number, etc.) and associated value.
- **`json_error_t`**: Enumerates all possible parsing errors, such as invalid tokens, nesting depth exceeded, or allocation failures.

### Key Functions
- **Initialization & Cleanup**: `json_parser_init` prepares the parser, while `json_parser_free` releases allocated memory.
- **Parsing**: `json_parser_parse` processes the input JSON string and populates tokens.
- **Utilities**: `json_get_tokens` retrieves parsed tokens, and `json_error_string` converts error codes to descriptive messages.

## Error Handling
The parser tracks errors during execution and halts on the first encountered issue. Errors range from syntax violations (e.g., unexpected characters) to resource constraints (e.g., exceeding token limits). Users can programmatically check the error type and respond accordingly.

## Building
The library is implemented as a single-header file (`json_parser.h`) with optional embedded implementation. It can be integrated into projects in two ways:
1. **As a Header-Only Library**: Define `JSON_PARSER_IMPLEMENTATION` in one source file to include the implementation.
2. **As a Static Library**: Use the provided CMake configuration to compile a static library, simplifying linking in larger projects.

## Usage
1. **Initialize the Parser**: Provide the JSON input string and its length.
2. **Parse the Input**: Execute the parsing routine and check for errors.
3. **Retrieve Tokens**: Access the parsed tokens to read JSON structure and values.
4. **Cleanup**: Release parser resources after processing.

## :snowman: Author

Eray Öztürk ([@diffstorm](https://github.com/diffstorm))

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
