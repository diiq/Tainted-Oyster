#define NEW(y, x) GC_MALLOC(sizeof(x)*y);
//#define NEW(y, x) malloc(sizeof(x)*y)

typedef struct{
    int flag;
    void* obj;
    int size;
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
};

parser make_character(char c);
parser make_one_of(char *cs);
parser make_any();
