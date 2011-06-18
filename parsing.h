#define NEW(y, x) GC_MALLOC(sizeof(x)*y);
//#define NEW(y, x) malloc(sizeof(x)*y)

enum PARSE_TYPES {
    CHAR,
    SYMBOL,
    NEWLINE
};

typedef struct{
    int type;
    void* x;
} parsed_value;

typedef struct{
    int flag;
    parsed_value* objs;
    int count;
    int index;
} parsed;

typedef struct parser_struct parser;

struct parser_struct {
    parsed (*parse)(char* stream, int index, parser p);
    union {
        char c;
        char *cs;
        parser *pser;
    };
    int count;
};

parser character(char c);
parser one_of(char *cs);
parser any();
parser many(parser p);
parser parse_not(parser p);
parser sequence(parser *ps, int count);
