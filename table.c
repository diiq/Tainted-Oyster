
#ifndef TABLE
#define TABLE
// For the moment, an assoc table. Fix later.

typedef struct table_unit_struct table_unit;

struct table_unit_struct {
    table_unit *next;
    int key;
    void *value;
};

typedef struct {
    table_unit *root;
} table;

table *make_table()
{
    table *ret = NEW(table);
    ret->root = NULL;
    return ret;
}

void *table_get(int key, table *tab, int *err)
{
    table_unit *ent;
    for(ent = tab->root; ent; ent = ent->next){
        if(ent->key == key){
            *err = 1;
            return ent->value;
        }
    }
    *err = 0;
    return NULL;
}

void table_put(int key, void *entry, table *tab)
{
    table_unit *ent;
    int i = 0;
    for(ent = tab->root; ent; ent = ent->next){
        if(ent->key == key){
            ent->value = entry;
            i = 1;
        }
    }
    if (!i){
        ent = NEW(table_unit);
        ent->next = tab->root;
        ent->key = key;
        ent->value = entry;
        tab->root = ent;
    }
}


int table_rev_lookup(void *value, int (equal)(void *a, void *b), table *tab, int *err)
{
    table_unit *ent;
    for(ent = tab->root; ent; ent = ent->next){
        if(equal(ent->value, value)){
            *err = 1;
            return ent->key;
        }
    }
    *err = 0;
    return -1;
}

void table_remove(int key, table *tab)
{
    table_unit *ent;
    table_unit *pent = NULL;
    for(ent = tab->root; ent; ent = ent->next){
        if(ent->key == key){
            if(pent){
                pent->next = ent->next;
            } else {
                tab->root = ent->next;
            }
        }
        pent = ent;
    }
}

int table_empty(table *tab){
    return tab->root == NULL;
}

#define table_loop(k, v, tab) {                                         \
    table_unit *table_ent;                                              \
    for(table_ent = tab->root; table_ent; table_ent = table_ent->next){ \
    k = table_ent->key;                                                 \
    v = table_ent->value;                                                       


#define table_end_loop }} do{}while(0)

#endif
