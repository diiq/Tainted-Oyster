#define NEW(x) GC_MALLOC(sizeof(x))
//#define NEW(x) malloc(sizeof(x))

enum {
    SYMBOL,
    NUMBER,
    OPEN,
    CLOSE,
    NEWLINE,
    END,
    FUNCTIONAL_OPEN
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

 
