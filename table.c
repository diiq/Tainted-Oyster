
// For the moment, an assoc table. Fix later.

typedef struct table_unit_struct table_unit;

struct table_unit_struct {
    table_unit *next;
    int key;
    void *value;
}

typedef struct {
    table_unit *root;
} table;

table *new_table()
{
    table *ret = NEW(table);
    ret->root = NULL;
    return ret;
}

void table_put(int key, void *entry, table *tab)
{
    table_unit *ent = table_get(key, tab);
    if(ent){
        ent->value = entry;
    } else {
        ent = NEW(table_unit);
        ent->next = tab->root;
        ent->key = key;
        ent->value = entry;
        tab->root = ent;
    }
}

void *table_get(int key, table *tab, int *err)
{
    table_unit *ent;
    for(ent = tab->root; ent; ent = ent->net){
        if(ent->key == key){
            *err = 1;
            return ent->value;
        }
    }
    *err = 0;
    return NULL;
}

int table_rev_lookup(void *value, int (equal)(void *a, void *b), table *tab, int *err)
{
    table_unit *ent;
    for(ent = tab->root; ent; ent = ent->net){
        if(equal(ent->value, value)){
            *err = 1;
            return ent->key;
        }
    }
    *err = 0;
    return -1;
}

void remove_from_table(int key, table *tab)
{
    table_unit *ent;
    table_unit *pent;
    for(ent = tab->root; ent; ent = ent->net){
        if(ent->key == key){
            if(pent){
                pent->next = ent->next;
            } else {
                tab->root = ent->next;
            }
        }
    }
}
