#include <gtest/gtest.h>
#include <fstream>
#include "json_parser.h"

class JsonStructureTest : public ::testing::Test
{
protected:
    json_parser_t parser;
    size_t token_count;
    const json_token_t *tokens;
    std::string json_str;

    void SetUp() override
    {
        std::ifstream file("../large_json_file.json", std::ios::binary);

        if(!file.is_open())
        {
            throw std::runtime_error("Can't open file");
        }

        json_str = std::string(std::istreambuf_iterator<char>(file),
                               std::istreambuf_iterator<char>());
        file.close();
        json_parser_init(&parser, json_str.data(), json_str.size());

        if(json_parser_parse(&parser) != JSON_ERROR_NONE)
        {
            throw std::runtime_error("Parsing failed");
        }

        tokens = json_get_tokens(&parser, &token_count);
    }

    void TearDown() override
    {
        json_parser_free(&parser);
    }

    void validateMetadata(const json_token_t *&token, const json_token_t *tokens_end)
    {
        // Verify metadata object structure
        ASSERT_LT(token, tokens_end);
        ASSERT_EQ(token->type, JSON_TOKEN_OBJECT);
        const json_token_t *metadata_obj = token;
        token++; // Move into the object
        bool found_version = false, found_author = false;

        while(token < tokens_end && token->start < metadata_obj->end)
        {
            if(token->type == JSON_TOKEN_STRING)
            {
                if(strcmp(token->value.string, "version") == 0)
                {
                    token++;
                    ASSERT_LT(token, tokens_end);
                    ASSERT_EQ(token->type, JSON_TOKEN_STRING);
                    ASSERT_STREQ(token->value.string, "1.0");
                    found_version = true;
                }
                else if(strcmp(token->value.string, "author") == 0)
                {
                    token++;
                    ASSERT_LT(token, tokens_end);
                    ASSERT_EQ(token->type, JSON_TOKEN_STRING);
                    ASSERT_STREQ(token->value.string, "AutoGen");
                    found_author = true;
                }
            }

            token++;
        }

        ASSERT_TRUE(found_version);
        ASSERT_TRUE(found_author);
    }


    void validateEntry(const json_token_t *&token, const json_token_t *tokens_end)
    {
        ASSERT_EQ(token->type, JSON_TOKEN_OBJECT); // Explicit check
        const json_token_t *entry_obj = token;
        token++; // Move past the object token to process fields
        bool has_id = false, has_name = false, has_price = false;

        while(token < tokens_end && token->start < entry_obj->end)
        {
            if(token->type == JSON_TOKEN_STRING)
            {
                const char *key = token->value.string;
                token++; // Move to the value token
                bool advanced_in_branch = false;

                if(strcmp(key, "id") == 0)
                {
                    ASSERT_EQ(token->type, JSON_TOKEN_NUMBER);
                    has_id = true;
                }
                else if(strcmp(key, "name") == 0)
                {
                    ASSERT_EQ(token->type, JSON_TOKEN_STRING);
                    has_name = true;
                }
                else if(strcmp(key, "price") == 0)
                {
                    ASSERT_EQ(token->type, JSON_TOKEN_NUMBER);
                    has_price = true;
                }
                else if(strcmp(key, "dimensions") == 0)
                {
                    validateDimensions(token, tokens_end);
                    advanced_in_branch = true;
                }

                if(!advanced_in_branch)
                {
                    // If the value is an object or array we want to generically skip:
                    const json_token_t *current_value_token = token;

                    if(current_value_token->type == JSON_TOKEN_OBJECT || current_value_token->type == JSON_TOKEN_ARRAY)
                    {
                        token++; // Move past the object/array token itself

                        while(token < tokens_end && token->start < current_value_token->end)
                        {
                            token++; // Skip children
                        }

                        // Now token is at the next key or end of entry_obj
                    }
                    else
                    {
                        // Simple value, just advance once to get to the next key
                        token++;
                    }
                }
            }
            else
            {
                token++; // Skip non-string tokens (e.g., commas, brackets)
            }
        }

        ASSERT_TRUE(has_id);
        ASSERT_TRUE(has_name);
        ASSERT_TRUE(has_price);
    }

    void validateDimensions(const json_token_t *&token, const json_token_t *tokens_end)
    {
        ASSERT_EQ(token->type, JSON_TOKEN_OBJECT);
        const json_token_t *dimensions_obj = token;
        token++; // Move into the object
        bool has_length = false, has_width = false, has_height = false;

        while(token < tokens_end && token->start < dimensions_obj->end)
        {
            if(token->type == JSON_TOKEN_STRING)
            {
                const char *key = token->value.string;
                token++; // Move to value

                if(strcmp(key, "length") == 0)
                {
                    ASSERT_EQ(token->type, JSON_TOKEN_NUMBER);
                    has_length = true;
                }
                else if(strcmp(key, "width") == 0)
                {
                    ASSERT_EQ(token->type, JSON_TOKEN_NUMBER);
                    has_width = true;
                }
                else if(strcmp(key, "height") == 0)
                {
                    ASSERT_EQ(token->type, JSON_TOKEN_NUMBER);
                    has_height = true;
                }

                token++; // Move to next key
            }
            else
            {
                token++; // Skip non-string tokens
            }
        }

        ASSERT_TRUE(has_length);
        ASSERT_TRUE(has_width);
        ASSERT_TRUE(has_height);
    }
};

TEST_F(JsonStructureTest, ValidateFullStructure)
{
    const json_token_t *current = tokens;
    const json_token_t *tokens_end = tokens + token_count;
    // Root object
    ASSERT_LT(current, tokens_end);
    ASSERT_EQ(current->type, JSON_TOKEN_OBJECT);
    const json_token_t *root_obj = current;
    current++;
    // Metadata validation
    ASSERT_EQ(current->type, JSON_TOKEN_STRING);
    ASSERT_STREQ(current->value.string, "metadata");
    current++;
    validateMetadata(current, tokens_end);
    // Entries array
    ASSERT_EQ(current->type, JSON_TOKEN_STRING);
    ASSERT_STREQ(current->value.string, "entries");
    current++;
    ASSERT_EQ(current->type, JSON_TOKEN_ARRAY);
    const json_token_t *entries_array = current;
    current++; // Now at the first element of the array (should be an object)
    // Validate entries
    size_t entry_count = 0;

    while(current < tokens_end && current->start < entries_array->end)
    {
        // Ensure the current token is an object before validating
        ASSERT_EQ(current->type, JSON_TOKEN_OBJECT) << "Entry is not an object at position " << (current - tokens);
        validateEntry(current, tokens_end);
        entry_count++;
    }

    ASSERT_EQ(entry_count, 100); // Ensure 100 entries
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}