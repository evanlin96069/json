#include "test.h"

#include <string.h>

#define JSON_IMPLEMENTATION
#include "../json.h"

TEST(arena1, "Arena alloc") {
    JsonArena arena;
    json_arena_init(&arena, 1 << 10);
    uint8_t* ptr1 = json_arena_alloc(&arena, 10);
    uint8_t* ptr2 = json_arena_alloc(&arena, 10);
    ASSERT(ptr1 != ptr2);
    for (int i = 0; i < 10; i++) {
        ptr1[i] = 87;
        ptr2[i] = 63;
    }
    for (int i = 0; i < 10; i++) {
        ASSERT(ptr1[i] == 87);
        ASSERT(ptr2[i] == 63);
    }
    json_arena_deinit(&arena);
}

TEST(arena2, "Arena reset") {
    JsonArena arena;
    json_arena_init(&arena, 1 << 10);
    uint8_t* ptr1 = json_arena_alloc(&arena, 10);
    uint8_t* ptr2 = json_arena_alloc(&arena, 10);
    ASSERT(ptr1 != ptr2);
    json_arena_reset(&arena);
    uint8_t* ptr3 = json_arena_alloc(&arena, 10);
    ASSERT(ptr1 == ptr3);
    json_arena_deinit(&arena);
}

TEST(arena3, "Arena realloc") {
    JsonArena arena;
    json_arena_init(&arena, 1 << 10);
    uint8_t* ptr1 = json_arena_alloc(&arena, 10);
    for (int i = 0; i < 10; i++) {
        ptr1[i] = 87;
    }
    // in-place realloc
    uint8_t* ptr2 = json_arena_realloc(&arena, ptr1, 10, 30);
    ASSERT(ptr1 == ptr2);
    json_arena_alloc(&arena, 10);
    // not in-place realloc
    uint8_t* ptr3 = json_arena_realloc(&arena, ptr1, 30, 40);
    ASSERT(ptr1 != ptr3);
    for (int i = 0; i < 10; i++) {
        ASSERT(ptr3[i] == 87);
    }
    json_arena_deinit(&arena);
}

Test* arena_tests[] = {
    &arena1,
    &arena2,
    &arena3,
};

TEST(good_json1, "Parse null") {
    const char* buffer = "null";
    JsonArena arena;
    json_arena_init(&arena, 1 << 10);
    JsonValue* value = json_parse(buffer, &arena);
    ASSERT(value->type == JSON_NULL);
    json_arena_deinit(&arena);
}

TEST(good_json2, "Parse array") {
    const char* buffer = "[null, null, null]";
    JsonArena arena;
    json_arena_init(&arena, 1 << 10);
    JsonValue* value = json_parse(buffer, &arena);
    ASSERT(value->type == JSON_ARRAY);
    ASSERT(value->array->size == 3);

    ASSERT(value->array->data[0]->type == JSON_NULL);
    ASSERT(value->array->data[1]->type == JSON_NULL);
    ASSERT(value->array->data[2]->type == JSON_NULL);
    json_arena_deinit(&arena);
}

#define EPSILON 1e-10
static int compareNumber(double a, double b, double epsilon) {
    double diff = (a > b) ? (a - b) : (b - a);

    if (diff < epsilon) {
        return 0;
    } else if (a < b) {
        return -1;
    } else {
        return 1;
    }
}

TEST(good_json3, "Parse number") {
    const char* buffer = "[3, 3.14, 1e5, -5.2, -1E-4, 3.1e+4]";
    double gold[] = {3, 3.14, 1e5, -5.2, -1E-4, 3.1e+4};
    JsonArena arena;
    json_arena_init(&arena, 1 << 10);
    JsonValue* value = json_parse(buffer, &arena);
    ASSERT(value->type == JSON_ARRAY);
    ASSERT(value->array->size == 6);
    for (size_t i = 0; i < value->array->size; i++) {
        ASSERT(value->array->data[i]->type == JSON_NUMBER);
        ASSERT(compareNumber(value->array->data[i]->number, gold[i], EPSILON) ==
               0);
    }
    json_arena_deinit(&arena);
}

TEST(good_json4, "Parse boolean") {
    const char* buffer = "[true, false]";
    JsonArena arena;
    json_arena_init(&arena, 1 << 10);
    JsonValue* value = json_parse(buffer, &arena);
    ASSERT(value->type == JSON_ARRAY);
    ASSERT(value->array->size == 2);

    ASSERT(value->array->data[0]->type == JSON_BOOLEAN);
    ASSERT(value->array->data[0]->boolean == true);
    ASSERT(value->array->data[1]->type == JSON_BOOLEAN);
    ASSERT(value->array->data[1]->boolean == false);
    json_arena_deinit(&arena);
}

TEST(good_json5, "Parse string") {
    const char* buffer = "[\"string\", \"newline\\n\", \"tab\\t\"]";
    const char* gold[] = {"string", "newline\n", "tab\t"};
    JsonArena arena;
    json_arena_init(&arena, 1 << 10);
    JsonValue* value = json_parse(buffer, &arena);
    ASSERT(value->type == JSON_ARRAY);
    ASSERT(value->array->size == 3);
    for (size_t i = 0; i < value->array->size; i++) {
        ASSERT(value->array->data[i]->type == JSON_STRING);
        ASSERT(strcmp(value->array->data[i]->string, gold[i]) == 0);
    }
    json_arena_deinit(&arena);
}

TEST(good_json6, "Parse object") {
    const char* buffer = "{\"key1\": \"string\", \"key2\": 5}";
    JsonArena arena;
    json_arena_init(&arena, 1 << 10);
    JsonValue* value = json_parse(buffer, &arena);
    ASSERT(value->type == JSON_OBJECT);
    JsonValue* key1 = json_object_find(value->object, "key1");
    ASSERT(key1->type == JSON_STRING);
    ASSERT(strcmp(key1->string, "string") == 0);
    JsonValue* key2 = json_object_find(value->object, "key2");
    ASSERT(key2->type == JSON_NUMBER);
    ASSERT(key2->number == 5);
    json_arena_deinit(&arena);
}

Test* json_good_tests[] = {
    &good_json1, &good_json2, &good_json3,
    &good_json4, &good_json5, &good_json6,
};

TEST(bad_json1, "Extra comma") {
    JsonArena arena;
    json_arena_init(&arena, 1 << 10);
    JsonValue* value;
    value = json_parse("{\"test\":3,}", &arena);
    ASSERT(value->type == JSON_ERROR);
    json_arena_reset(&arena);
    value = json_parse("[1,2,3,]", &arena);
    ASSERT(value->type == JSON_ERROR);
    json_arena_reset(&arena);
    value = json_parse("[1,2,3],", &arena);
    ASSERT(value->type == JSON_ERROR);
    json_arena_reset(&arena);
    value = json_parse("[,\"test\"]", &arena);
    ASSERT(value->type == JSON_ERROR);
    json_arena_reset(&arena);
    value = json_parse("[1,2,,]", &arena);
    ASSERT(value->type == JSON_ERROR);
    json_arena_deinit(&arena);
}

TEST(bad_json2, "Bad string") {
    JsonArena arena;
    json_arena_init(&arena, 1 << 10);
    JsonValue* value;
    value = json_parse("\'single qoute\'", &arena);
    ASSERT(value->type == JSON_ERROR);
    json_arena_reset(&arena);
    value = json_parse("\"tabs\tin\tstring\"", &arena);
    ASSERT(value->type == JSON_ERROR);
    json_arena_reset(&arena);
    value = json_parse("\"Invalid \\x32\"", &arena);
    ASSERT(value->type == JSON_ERROR);
    json_arena_deinit(&arena);
}

TEST(bad_json3, "Bad number") {
    JsonArena arena;
    json_arena_init(&arena, 1 << 10);
    JsonValue* value;
    value = json_parse("0x32", &arena);
    ASSERT(value->type == JSON_ERROR);
    json_arena_reset(&arena);
    value = json_parse("032", &arena);
    ASSERT(value->type == JSON_ERROR);
    json_arena_reset(&arena);
    value = json_parse("0e+-1", &arena);
    ASSERT(value->type == JSON_ERROR);
    json_arena_reset(&arena);
    value = json_parse("0e+", &arena);
    ASSERT(value->type == JSON_ERROR);
    json_arena_deinit(&arena);
}

TEST(bad_json4, "Missing token") {
    JsonArena arena;
    json_arena_init(&arena, 1 << 10);
    JsonValue* value;
    value = json_parse("{\"key\" 5}", &arena);
    ASSERT(value->type == JSON_ERROR);
    json_arena_reset(&arena);
    value = json_parse("[1,2,3", &arena);
    ASSERT(value->type == JSON_ERROR);
    json_arena_reset(&arena);
    value = json_parse("{\"key\": 5", &arena);
    ASSERT(value->type == JSON_ERROR);
    json_arena_reset(&arena);
    value = json_parse("[\"test]", &arena);
    ASSERT(value->type == JSON_ERROR);
    json_arena_deinit(&arena);
}

TEST(bad_json5, "Empty input") {
    JsonArena arena;
    json_arena_init(&arena, 1 << 10);
    JsonValue* value = json_parse("", &arena);
    ASSERT(value->type == JSON_ERROR);
    json_arena_deinit(&arena);
}

Test* json_bad_tests[] = {
    &bad_json1, &bad_json2, &bad_json3, &bad_json4, &bad_json5,
};

int main(void) {
    RUN_ALL_TESTS(arena_tests, "arena");
    RUN_ALL_TESTS(json_good_tests, "good JSON");
    RUN_ALL_TESTS(json_bad_tests, "bad JSON");
    return 0;
}
