#ifndef JSON_H
#define JSON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum JsonType {
    JSON_ERROR,
    JSON_NULL,
    JSON_BOOLEAN,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT,
} JsonType;

typedef struct JsonValue JsonValue;
typedef struct JsonArray JsonArray;
typedef struct JsonObject JsonObject;

struct JsonValue {
    JsonType type;
    union {
        double number;
        bool boolean;
        char* string;
        JsonArray* array;
        JsonObject* object;
    };
};

typedef struct JsonArray {
    struct JsonArray* next;
    JsonValue* value;
} JsonArray;

typedef struct JsonObject {
    struct JsonObject* next;
    char* key;
    JsonValue* value;
} JsonObject;

typedef struct Arena {
    size_t capacity;
    size_t size;
    uint8_t* data;
} Arena;

void arenaInit(Arena* arena, size_t capacity);
void* arenaAlloc(Arena* arena, size_t size);
void arenaReset(Arena* arena);
void arenaDeinit(Arena* arena);

JsonValue* jsonParse(const char* text, Arena* arena);

#endif
