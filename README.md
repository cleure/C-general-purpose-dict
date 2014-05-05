C-general-purpose-dict
======================

A small, zero-dependency, general purpose hash table which uses CRC32 as a hashing mechanism, and separate chaining for collision resolution. It takes a string as the hash key, and a pointer to anything as the value.

TODO
====

- More complete code comments.
- More complete documentation.
- Thoroughly test free() logic.

Data Structures
===============

struct dict_node {
    uint32_t hash;
    char *key;
    void *value;
    struct dict_node *next;
};

struct dict {
    size_t used;
    struct dict_node *table;
    uint32_t capacity;
    uint32_t seed;
};

struct dict_iterator {
    struct dict *dict;
    struct dict_node *cur;
    uint32_t idx;
};

API
===

struct dict *dict_new(uint32_t seed, size_t capacity);

TODO: DESCRIPTION

int dict_resize(struct dict *dict, uint32_t capacity);

TODO: DESCRIPTION

void dict_clear(struct dict *dict);

TODO: DESCRIPTION

void dict_delete(struct dict *dict);

TODO: DESCRIPTION

int dict_set(struct dict *dict, char *key, void *value);

TODO: DESCRIPTION

struct dict_node *dict_get(struct dict *dict, char *key);

TODO: DESCRIPTION

int dict_del(struct dict *dict, char *key);

TODO: DESCRIPTION

int dict_contains(struct dict *dict, char *key);

TODO: DESCRIPTION

void dict_iterate_start(struct dict *dict, struct dict_iterator *it);

TODO: DESCRIPTION

struct dict_node *dict_iterate_next(struct dict_iterator *it);

TODO: DESCRIPTION

struct dict_node *dict_iterate_difference(struct dict *b, struct dict_iterator *it);

TODO: DESCRIPTION

struct dict_node *dict_iterate_intersection(struct dict *b, struct dict_iterator *it);

TODO: DESCRIPTION
