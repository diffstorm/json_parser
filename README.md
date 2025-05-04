# JSON Parser Library [![Awesome](https://awesome.re/badge.svg)](https://github.com/diffstorm/json_parser)

[![Build Status](https://github.com/diffstorm/json_parser/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/diffstorm/json_parser/actions)
[![License](https://img.shields.io/github/license/diffstorm/json_parser)](https://github.com/diffstorm/json_parser/blob/main/LICENSE)
[![Language](https://img.shields.io/github/languages/top/diffstorm/json_parser)](https://github.com/diffstorm/json_parser)
[![Code Coverage](https://codecov.io/gh/diffstorm/json_parser/branch/main/graph/badge.svg)](https://codecov.io/gh/diffstorm/json_parser)
![GitHub Stars](https://img.shields.io/github/stars/diffstorm/json_parser?style=social)
![Platforms](https://img.shields.io/badge/Platform-Linux%20%7C%20Windows%20%7C%20macOS-lightgrey)

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

    ```bash
	sudo apt-get install -y build-essential cmake libgtest-dev googletest
    git clone https://github.com/diffstorm/json_parser.git
    cd json_parser
    mkdir build
    cd build
    cmake ..
    make
	./demo
	./demo_cpp
    ./json_parser_test
    ```

## Usage
1. **Initialize the Parser**: Provide the JSON input string and its length.
2. **Parse the Input**: Execute the parsing routine and check for errors.
3. **Retrieve Tokens**: Access the parsed tokens to read JSON structure and values.
4. **Cleanup**: Release parser resources after processing.


## API Reference

### Data Structures
#### `json_parser_t`
Manages the parser's state and configuration.
- **Fields**:
  - `const char *json`: Input JSON string (not copied; must remain valid during parsing).
  - `size_t length`: Length of the JSON input.
  - `size_t max_depth`: Maximum allowed nesting depth (default: `JSON_DEFAULT_MAX_DEPTH`).
  - `size_t max_string`: Maximum allowed string length (default: `JSON_DEFAULT_MAX_STRING`).
  - `json_error_t error`: Current error code (`JSON_ERROR_NONE` if no error).

#### `json_token_t`
Represents a parsed JSON token.
- **Fields**:
  - `json_token_type_t type`: Token type (e.g., `JSON_TOKEN_STRING`, `JSON_TOKEN_NUMBER`).
  - Union `value`:
    - `char *string`: String value (valid if `type` is `JSON_TOKEN_STRING`).
    - `double number`: Numeric value (valid if `type` is `JSON_TOKEN_NUMBER`).
  - `size_t start`, `end`: Start and end positions in the original JSON string.

#### `json_error_t`
Enumerates parsing error codes (e.g., `JSON_ERROR_INVALID_TOKEN`, `JSON_ERROR_ALLOCATION_FAILED`).

---

### Functions
#### `void json_parser_init(json_parser_t *parser, const char *json, size_t length)`
Initializes the parser with a JSON input string.
- `parser`: Uninitialized parser instance.
- `json`: Pointer to the JSON string.
- `length`: Length of the JSON string.
- **Note**: Sets default limits and allocates initial token memory.

#### `void json_parser_free(json_parser_t *parser)`
Releases all memory allocated by the parser (tokens, strings, etc.).
- Must be called after parsing to avoid leaks.

#### `json_error_t json_parser_parse(json_parser_t *parser)`
Parses the JSON input and populates tokens.
- Returns the first encountered error (or `JSON_ERROR_NONE` on success).
- Tokens are accessible via `json_get_tokens` after parsing.

#### `const char *json_error_string(json_error_t error)`
Converts an error code to a human-readable message (e.g., `JSON_ERROR_INVALID_NUMBER` ? `"Invalid number format"`).

#### `const json_token_t *json_get_tokens(const json_parser_t *parser, size_t *count)`
Retrieves the parsed tokens.
- `count`: Output parameter for the number of tokens.
- Returns a pointer to the token array (valid until `json_parser_free` is called).

---

### Enums
#### `json_token_type_t`
Defines token types:
- `JSON_TOKEN_OBJECT`, `JSON_TOKEN_ARRAY`, `JSON_TOKEN_STRING`, `JSON_TOKEN_NUMBER`,
  `JSON_TOKEN_TRUE`, `JSON_TOKEN_FALSE`, `JSON_TOKEN_NULL`, `JSON_TOKEN_INVALID`.

---

### Configuration Macros
- `JSON_DEFAULT_MAX_TOKENS`: Initial token array capacity.
- `JSON_DEFAULT_MAX_DEPTH`: Default maximum nesting depth.
- `JSON_DEFAULT_MAX_STRING`: Default maximum string length.

## :snowman: Author

Eray Öztürk ([@diffstorm](https://github.com/diffstorm))

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
