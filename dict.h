#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

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
	
	void (*key_free_fn)(void *);
	void (*value_free_fn)(void *);
};

struct dict_iterator {
    struct dict *dict;
    struct dict_node *cur;
    uint32_t idx;
};

struct dict *dict_new(uint32_t seed, uint32_t capacity, void (*key_free_fn)(void *), void (*value_free_fn)(void *));
int dict_resize(struct dict *dict, uint32_t capacity);
struct dict *dict_clone(struct dict *to_clone, void *(*key_clone_fn)(void *), void *(*value_clone_fn)(void *));
void dict_clear(struct dict *dict);
void dict_delete(struct dict *dict);

int dict_set(struct dict *dict, char *key, void *value);
struct dict_node *dict_get(struct dict *dict, char *key);
int dict_del(struct dict *dict, char *key);
int dict_contains(struct dict *dict, char *key);

void dict_iterate_start(struct dict *dict, struct dict_iterator *it);
struct dict_node *dict_iterate_next(struct dict_iterator *it);

struct dict_node *dict_iterate_difference(struct dict *b, struct dict_iterator *it);
struct dict_node *dict_iterate_intersection(struct dict *b, struct dict_iterator *it);

#ifdef __cplusplus
}
#endif
