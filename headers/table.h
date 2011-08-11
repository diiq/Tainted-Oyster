

struct table_entry {
    void (*decref) (table_entry * x);
    int ref;
    oyster *it;
};

struct table {
    void (*decref) (table * x);
    int ref;
    GHashTable *it;
    GHashTable *leaked;
};

void table_free(table *t);
void table_entry_free(table_entry *e);
