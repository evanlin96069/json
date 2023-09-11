#include <stdio.h>
#include <stdlib.h>

#include "json.h"

static void jsonPrintInner(const JsonValue* value, int indent) {
    switch (value->type) {
        case JSON_ERROR:
            printf("Syntax Error: %s\n", value->string);
            break;
        case JSON_NULL:
            printf("%*snull\n", indent * 2, "");
            break;
        case JSON_BOOLEAN:
            printf("%*s%s\n", indent * 2, "",
                   value->boolean ? "true" : "false");
            break;
        case JSON_NUMBER:
            printf("%*s%lf\n", indent * 2, "", value->number);
            break;
        case JSON_STRING:
            printf("%*s\"%s\"\n", indent * 2, "", value->string);
            break;
        case JSON_ARRAY: {
            const JsonArray* curr = value->array;
            printf("%*s[\n", indent * 2, "");
            while (curr) {
                jsonPrintInner(curr->value, indent + 1);
                curr = curr->next;
            }
            printf("%*s]\n", indent * 2, "");
        } break;
        case JSON_OBJECT: {
            const JsonObject* curr = value->object;
            printf("%*s{\n", indent * 2, "");
            while (curr) {
                printf("%*s\"%s\" :\n", (indent + 1) * 2, "", curr->key);
                jsonPrintInner(curr->value, indent + 1);
                curr = curr->next;
            }
            printf("%*s}\n", indent * 2, "");
        } break;
    }
}

void jsonPrint(const JsonValue* value) { jsonPrintInner(value, 0); }

int main(int argc, char** argv) {
    FILE* fp;
    size_t size;
    char* buffer;

    Arena arena;
    arenaInit(&arena, 1 << 24);

    for (int i = 1; i < argc; i++) {
        printf("File: %s\n", argv[i]);
        fp = fopen(argv[i], "r");
        if (!fp) {
            fprintf(stderr, "Cannot open file\n");
            continue;
        }
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        rewind(fp);
        buffer = calloc(1, size + 1);
        if (!buffer) {
            fclose(fp);
            fprintf(stderr, "Failed to allocate memory\n");
            return 1;
        }
        if (fread(buffer, size, 1, fp) != 1) {
            fprintf(stderr, "Failed to read file\n");
            fclose(fp);
            free(buffer);
            continue;
        }
        fclose(fp);
        arenaReset(&arena);
        JsonValue* value = jsonParse(buffer, &arena);
        jsonPrint(value);
        if (value->type == JSON_ERROR) {
            printf("Syntax Error: %s\n", value->string);
        } else {
            printf("Parse succeeded\n");
        }
        free(buffer);
    }

    arenaDeinit(&arena);
    return 0;
}
