#ifndef PARSINGH
#define PARSINGH


enum {
    PARSED_SYMBOL,
    PARSED_NUMBER,
    PARSED_OPEN,
    PARSED_CLOSE,
    PARSED_NEWLINE,
    PARSED_END,
    PARSED_FUNCTIONAL_OPEN
};

typedef struct{
    int flag;
    char *value;
    int count;
} parsed_token;

typedef struct {
    int push_to;
    int pull_from;
    parsed_token** s;
} token_stream;

#endif
