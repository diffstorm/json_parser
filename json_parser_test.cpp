/**
    @brief JSON Parser Library

    A lightweight, single-header C library for parsing JSON data. Designed for simplicity and portability, this parser provides a low-footprint solution to decode JSON-formatted strings into structured tokens while adhering to core JSON specifications.

    @date 2025-05-03
    @version 1.0
    @author Eray Ozturk | erayozturk1@gmail.com
    @url github.com/diffstorm
    @license MIT License
*/

#include <gtest/gtest.h>
#include "json_parser.h"
#include <string>
#include <vector>

class JsonParserTest : public ::testing::Test
{
protected:
    void TearDown() override
    {
        json_parser_free(&parser);
    }

    json_parser_t parser;
};

TEST_F(JsonParserTest, EmptyObject)
{
    const char *json = "{}";
    json_parser_init(&parser, json, strlen(json));
    ASSERT_EQ(json_parser_parse(&parser), JSON_ERROR_NONE);
    size_t count;
    const json_token_t *tokens = json_get_tokens(&parser, &count);
    ASSERT_EQ(count, 1);
    EXPECT_EQ(tokens[0].type, JSON_TOKEN_OBJECT);
}

TEST_F(JsonParserTest, EmptyArray)
{
    const char *json = "[]";
    json_parser_init(&parser, json, strlen(json));
    ASSERT_EQ(json_parser_parse(&parser), JSON_ERROR_NONE);
    size_t count;
    const json_token_t *tokens = json_get_tokens(&parser, &count);
    ASSERT_EQ(count, 1);
    EXPECT_EQ(tokens[0].type, JSON_TOKEN_ARRAY);
}

TEST_F(JsonParserTest, BasicString)
{
    const char *json = "\"Hello, World!\"";
    json_parser_init(&parser, json, strlen(json));
    ASSERT_EQ(json_parser_parse(&parser), JSON_ERROR_NONE);
    size_t count;
    const json_token_t *tokens = json_get_tokens(&parser, &count);
    ASSERT_EQ(count, 1);
    EXPECT_EQ(tokens[0].type, JSON_TOKEN_STRING);
    EXPECT_STREQ(tokens[0].value.string, "Hello, World!");
}

TEST_F(JsonParserTest, UnicodeEscape)
{
    const char *json = "\"John\\u00D0e\"";
    json_parser_init(&parser, json, strlen(json));
    ASSERT_EQ(json_parser_parse(&parser), JSON_ERROR_NONE);
    size_t count;
    const json_token_t *tokens = json_get_tokens(&parser, &count);
    ASSERT_EQ(count, 1);
    EXPECT_EQ(tokens[0].type, JSON_TOKEN_STRING);
    EXPECT_STREQ(tokens[0].value.string, "JohnÐe");
}

TEST_F(JsonParserTest, ValidNumbers)
{
    std::vector<std::string> test_cases =
    {
        "123", "123.45", "-123", "1e3", "1E+3", "1E-3", "0.123"
    };

    for(const auto &json : test_cases)
    {
        json_parser_init(&parser, json.c_str(), json.size());
        ASSERT_EQ(json_parser_parse(&parser), JSON_ERROR_NONE) << "Failed for: " << json;
        size_t count;
        const json_token_t *tokens = json_get_tokens(&parser, &count);
        ASSERT_EQ(count, 1) << "Failed for: " << json;
        EXPECT_EQ(tokens[0].type, JSON_TOKEN_NUMBER) << "Failed for: " << json;
        json_parser_free(&parser); // Explicit free since TearDown is per-test
    }
}

TEST_F(JsonParserTest, BooleanAndNull)
{
    const char *json = "[true, false, null]";
    json_parser_init(&parser, json, strlen(json));
    ASSERT_EQ(json_parser_parse(&parser), JSON_ERROR_NONE);
    size_t count;
    const json_token_t *tokens = json_get_tokens(&parser, &count);
    ASSERT_EQ(count, 4); // ARRAY, TRUE, FALSE, NULL
    EXPECT_EQ(tokens[0].type, JSON_TOKEN_ARRAY);
    EXPECT_EQ(tokens[1].type, JSON_TOKEN_TRUE);
    EXPECT_EQ(tokens[2].type, JSON_TOKEN_FALSE);
    EXPECT_EQ(tokens[3].type, JSON_TOKEN_NULL);
}

TEST_F(JsonParserTest, NestedStructures)
{
    const char *json = "{\"a\": [1, {\"b\": true}]}";
    json_parser_init(&parser, json, strlen(json));
    ASSERT_EQ(json_parser_parse(&parser), JSON_ERROR_NONE);
    size_t count;
    const json_token_t *tokens = json_get_tokens(&parser, &count);
    // Expected tokens: OBJECT, STRING(a), ARRAY, NUMBER(1), OBJECT, STRING(b), TRUE
    ASSERT_GE(count, 7);
    EXPECT_EQ(tokens[0].type, JSON_TOKEN_OBJECT);
    EXPECT_EQ(tokens[2].type, JSON_TOKEN_ARRAY);
    EXPECT_EQ(tokens[3].type, JSON_TOKEN_NUMBER);
    EXPECT_EQ(tokens[4].type, JSON_TOKEN_OBJECT);
}

TEST_F(JsonParserTest, InvalidEscape)
{
    const char *json = "\"\\x\"";
    json_parser_init(&parser, json, strlen(json));
    EXPECT_EQ(json_parser_parse(&parser), JSON_ERROR_INVALID_ESCAPE);
}

TEST_F(JsonParserTest, TrailingCharacters)
{
    const char *json = "123abc";
    json_parser_init(&parser, json, strlen(json));
    EXPECT_EQ(json_parser_parse(&parser), JSON_ERROR_TRAILING_CHARS);
}

TEST_F(JsonParserTest, ExceedMaxDepth)
{
    std::string json(33, '[');
    json.append(33, ']');
    json_parser_init(&parser, json.c_str(), json.size());
    parser.max_depth = 32;
    EXPECT_EQ(json_parser_parse(&parser), JSON_ERROR_NESTING_DEPTH);
}

TEST_F(JsonParserTest, StringTooLong)
{
    std::string long_str(JSON_DEFAULT_MAX_STRING, 'a');
    std::string json = "\"" + long_str + "\"";
    json_parser_init(&parser, json.c_str(), json.size());
    EXPECT_EQ(json_parser_parse(&parser), JSON_ERROR_STRING_TOO_LONG);
}

TEST_F(JsonParserTest, InvalidNumberFormat)
{
    const char *json = "12.34.56";
    json_parser_init(&parser, json, strlen(json));
    EXPECT_EQ(json_parser_parse(&parser), JSON_ERROR_TRAILING_CHARS);
}

TEST_F(JsonParserTest, MissingQuote)
{
    const char *json = "\"unclosed";
    json_parser_init(&parser, json, strlen(json));
    EXPECT_EQ(json_parser_parse(&parser), JSON_ERROR_UNEXPECTED_CHAR);
}

TEST_F(JsonParserTest, MismatchedBrackets)
{
    const char *json = "{]";
    json_parser_init(&parser, json, strlen(json));
    EXPECT_EQ(json_parser_parse(&parser), JSON_ERROR_UNEXPECTED_CHAR);
}

// Test: Reject numbers with leading zeros (e.g., "0123" is invalid JSON)
TEST_F(JsonParserTest, InvalidNumberLeadingZero)
{
    const char *json = "0123";
    json_parser_init(&parser, json, strlen(json));
    EXPECT_EQ(json_parser_parse(&parser), JSON_ERROR_INVALID_NUMBER);
}

// Test: Reject hexadecimal numbers (not valid JSON)
TEST_F(JsonParserTest, InvalidNumberHex)
{
    const char *json = "0x1F";
    json_parser_init(&parser, json, strlen(json));
    EXPECT_EQ(json_parser_parse(&parser), JSON_ERROR_INVALID_NUMBER);
}

// Test: Reject numbers starting with a decimal point (e.g., ".123")
TEST_F(JsonParserTest, InvalidNumberLeadingDecimal)
{
    const char *json = ".123";
    json_parser_init(&parser, json, strlen(json));
    EXPECT_EQ(json_parser_parse(&parser), JSON_ERROR_INVALID_NUMBER);
}

// Test: Reject invalid exponents (e.g., "123e" or "1e")
TEST_F(JsonParserTest, InvalidNumberExponentNoDigits)
{
    const char *json = "123e";
    json_parser_init(&parser, json, strlen(json));
    EXPECT_EQ(json_parser_parse(&parser), JSON_ERROR_INVALID_NUMBER);
}

// Test: Accept valid exponents (e.g., "123e45")
TEST_F(JsonParserTest, ValidNumberExponent)
{
    const char *json = "123e45";
    json_parser_init(&parser, json, strlen(json));
    ASSERT_EQ(json_parser_parse(&parser), JSON_ERROR_NONE);
    size_t count;
    const json_token_t *tokens = json_get_tokens(&parser, &count);
    ASSERT_EQ(count, 1);
    EXPECT_EQ(tokens[0].type, JSON_TOKEN_NUMBER);
    EXPECT_EQ(tokens[0].value.number, 123e45);
}

// Test: Reject trailing commas in arrays (e.g., "[1,]")
TEST_F(JsonParserTest, InvalidArrayTrailingComma)
{
    const char *json = "[1,]";
    json_parser_init(&parser, json, strlen(json));
    EXPECT_EQ(json_parser_parse(&parser), JSON_ERROR_UNEXPECTED_CHAR);
}

// Test: Reject non-string keys in objects (e.g., "{123: \"value\"}")
TEST_F(JsonParserTest, InvalidObjectKeyNonString)
{
    const char *json = "{123: \"value\"}";
    json_parser_init(&parser, json, strlen(json));
    EXPECT_EQ(json_parser_parse(&parser), JSON_ERROR_UNEXPECTED_CHAR);
}

// Test: Reject literals with incorrect casing (e.g., "True" instead of "true")
TEST_F(JsonParserTest, InvalidLiteralCasing)
{
    const char *json = "True";
    json_parser_init(&parser, json, strlen(json));
    EXPECT_EQ(json_parser_parse(&parser), JSON_ERROR_INVALID_TOKEN);
}

// Test: Reject JSON with comments (comments are not part of JSON)
TEST_F(JsonParserTest, InvalidComment)
{
    const char *json = "// Comment\n{}";
    json_parser_init(&parser, json, strlen(json));
    EXPECT_EQ(json_parser_parse(&parser), JSON_ERROR_INVALID_TOKEN);
}

// Test: Accept empty strings (e.g., "")
TEST_F(JsonParserTest, ValidEmptyString)
{
    const char *json = "\"\"";
    json_parser_init(&parser, json, strlen(json));
    ASSERT_EQ(json_parser_parse(&parser), JSON_ERROR_NONE);
    size_t count;
    const json_token_t *tokens = json_get_tokens(&parser, &count);
    ASSERT_EQ(count, 1);
    EXPECT_STREQ(tokens[0].value.string, "");
}

// Test: Handle escaped characters in strings (e.g., "\"\\\/\b\f\n\r\t")
TEST_F(JsonParserTest, ValidStringWithEscapes)
{
    const char *json = "\"\\\"\\\\\\/\\b\\f\\n\\r\\t\"";
    json_parser_init(&parser, json, strlen(json));
    ASSERT_EQ(json_parser_parse(&parser), JSON_ERROR_NONE);
    size_t count;
    const json_token_t *tokens = json_get_tokens(&parser, &count);
    ASSERT_EQ(count, 1);
    EXPECT_STREQ(tokens[0].value.string, "\"\\/\b\f\n\r\t");
}

// Test: Validate maximum allowed nesting depth
TEST_F(JsonParserTest, ValidMaxNestingDepth)
{
    std::string json;

    for(int i = 0; i < JSON_DEFAULT_MAX_DEPTH; ++i)
    {
        json += "[";
    }

    for(int i = 0; i < JSON_DEFAULT_MAX_DEPTH; ++i)
    {
        json += "]";
    }

    json_parser_init(&parser, json.c_str(), json.size());
    parser.max_depth = JSON_DEFAULT_MAX_DEPTH;
    ASSERT_EQ(json_parser_parse(&parser), JSON_ERROR_NONE);
}

// Test: Accept strings at the exact maximum allowed length
TEST_F(JsonParserTest, ValidStringExactMaxLength)
{
    std::string str(JSON_DEFAULT_MAX_STRING - 2, 'a'); // 254 chars if max_string=256
    std::string json = "\"" + str + "\"";
    json_parser_init(&parser, json.c_str(), json.size());
    ASSERT_EQ(json_parser_parse(&parser), JSON_ERROR_NONE);
}

TEST_F(JsonParserTest, ValidSurrogatePair)
{
    const char *json = "\"\\uD800\\uDC00\""; // Valid surrogate pair (U+10000)
    json_parser_init(&parser, json, strlen(json));
    ASSERT_EQ(json_parser_parse(&parser), JSON_ERROR_NONE);
}

// Test: Reject invalid Unicode surrogate pairs (parser does not handle them)
TEST_F(JsonParserTest, InvalidSurrogatePair)
{
    const char *json = "\"\\uD800\""; // Unpaired high surrogate
    json_parser_init(&parser, json, strlen(json));
    EXPECT_EQ(json_parser_parse(&parser), JSON_ERROR_INVALID_UNICODE);
}

// Test: Parse a complex JSON structure (real-world scenario)
TEST_F(JsonParserTest, ValidComplexStructure)
{
    const char *json = R"({
        "name": "John",
        "age": 30,
        "scores": [90.5, 80.0],
        "active": true,
        "data": null,
        "config": {
            "key": "value",
            "nested": [1, {"deep": false}]
        }
    })";
    json_parser_init(&parser, json, strlen(json));
    ASSERT_EQ(json_parser_parse(&parser), JSON_ERROR_NONE);
    size_t count;
    const json_token_t *tokens = json_get_tokens(&parser, &count);
    ASSERT_GE(count, 10); // Verify token count matches structure
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
