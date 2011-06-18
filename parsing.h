#define NEW(x) GC_MALLOC(sizeof(x))

enum {
    SYMBOL,
    OPEN,
    CLOSE,
    NEWLINE
};

typedef struct{
    int flag;
    char *value;
    int count;
} parsed_token;
