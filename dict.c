#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <memory.h>

#include "crc32.h"
#include "dict.h"

// Dummy clone function.
static void *_dummy_clone_fn(void *p)
{
    return p;
}

/**
 * Creates a new dict object.
 *
 * @param   uint32_t seed - CRC32 Seed.
 * @param   uint32_t capacity - Number of buckets to allocate.
 * @param   void (*key_free_fn)(void *) - Either a function pointer, or NULL.
 * @param   void (*value_free_fn)(void *) - Either a function pointer, or NULL.
 *
 * @return  struct dict *
 *
 * Returns a newly allocated struct dict pointer, or NULL on error.
 **/
struct dict *dict_new(uint32_t seed, uint32_t capacity, void (*key_free_fn)(void *), void (*value_free_fn)(void *))
{
    struct dict *dict;
    struct dict_node *table;
    
    dict = malloc(sizeof(*dict));
    if (dict == NULL) {
        return NULL;
    }
    
    table = malloc(sizeof(*table) * capacity);
    if (table == NULL) {
        free(dict);
        return NULL;
    }
    
    memset(dict, 0, sizeof(*dict));
    memset(table, 0, sizeof(*table) * capacity);
    
    dict->capacity = capacity;
    dict->used = 0;
    dict->seed = seed;
    dict->table = table;
    dict->key_free_fn = key_free_fn;
    dict->value_free_fn = value_free_fn;
    
    return dict;
}

/**
 * Clears all key/value pairs in dict object.
 *
 * @param   struct dict *dict
 * @return  void
 **/
void dict_clear(struct dict *dict)
{
    struct dict_node *cur;
    struct dict_node *prev;
    uint32_t i;
    
    for (i = 0; i < dict->capacity; i++) {
        if (dict->table[i].key != NULL) {
            cur = dict->table[i].next;
            while (cur) {
                prev = cur;
                cur = cur->next;
				
                // Free key?
                if (dict->key_free_fn != NULL) {
                    dict->key_free_fn(prev->key);
                }
				
                // Free value?
                if (dict->value_free_fn != NULL) {
                    dict->value_free_fn(prev->value);
                }
				
                free(prev);
            }
            
            // Free key?
            if (dict->key_free_fn != NULL) {
                dict->key_free_fn(dict->table[i].key);
            }
			
            // Free value?
            if (dict->value_free_fn != NULL) {
                dict->value_free_fn(dict->table[i].value);
            }
			
            dict->table[i].hash = 0;
            dict->table[i].key = NULL;
            dict->table[i].value = NULL;
            dict->table[i].next = NULL;
        }
    }
    
    dict->used = 0;
}

/**
 * Deletes a dict object, and frees all associated memory.
 *
 * @param   struct dict *dict
 * @return  void
 **/
void dict_delete(struct dict *dict)
{
    dict_clear(dict);
    free(dict->table);
    free(dict);
}

/**
 * Resizes a dict object.
 *
 * @param   struct dict *dict
 * @param   uint32_t capacity
 * @return  int
 *
 * Returns 1 on success, and 0 on error.
 **/
int dict_resize(struct dict *dict, uint32_t capacity)
{
    struct dict *dict_tmp;
    struct dict_node *cur;
    struct dict_node *table_tmp;
    uint32_t capacity_tmp;
    size_t i;
    
    // We pass in NULL as the free functions, otherwise we'll destroy memory
    // that we want to keep around.
    dict_tmp = dict_new(dict->seed, capacity, NULL, NULL);
    if (dict_tmp == NULL) {
        return 0;
    }
    
    for (i = 0; i < dict->capacity; i++) {
        cur = &dict->table[i];
        
        while (cur && cur->key != NULL) {
            if (!dict_set(dict_tmp, cur->key, cur->value)) {
                dict_delete(dict_tmp);
                return 0;
            }
            
            cur = cur->next;
        }
    }
    
    table_tmp = dict->table;
    capacity_tmp = dict->capacity;
    
    dict->table = dict_tmp->table;
    dict->capacity = dict_tmp->capacity;
    
    dict_tmp->table = table_tmp;
    dict_tmp->capacity = capacity_tmp;
    
    dict_delete(dict_tmp);
    return 1;
}

/**
 * Clone dict object, using key_clone_fn and value_clone_fn, function pointers.
 *
 * If the clone function pointer for key/value is NULL, then it will simply do a
 * static copy of the pointer. Doing this on heap allocated memory may lead to
 * memory errors.
 *
 * @param   struct dict *to_clone
 * @param   void *(*key_clone_fn)(void *)
 * @param   void *(*value_clone_fn)(void *)
 * @return  struct dict *
 *
 * Returns NULL on error.
 **/
struct dict *dict_clone(struct dict *to_clone, void *(*key_clone_fn)(void *), void *(*value_clone_fn)(void *))
{
    size_t i;
    void *key_clone;
    void *value_clone;
    struct dict_node *cur;
    struct dict *clone = dict_new(
        to_clone->seed,
        to_clone->capacity,
        to_clone->key_free_fn,
        to_clone->value_free_fn
    );
    
    if (clone == NULL) {
        return NULL;
    }
    
    if (key_clone_fn == NULL) {
        key_clone_fn = _dummy_clone_fn;
    }

    if (value_clone_fn == NULL) {
        value_clone_fn = _dummy_clone_fn;
    }
    
    for (i = 0; i < to_clone->capacity; i++) {
        cur = &to_clone->table[i];
        while (cur && cur->key) {
            key_clone = key_clone_fn(cur->key);
            value_clone = value_clone_fn(cur->value);
        
            if (dict_set(clone, key_clone, value_clone) == 0) {
                // Make sure key/value gets freed
                if (clone->key_free_fn) {
                    clone->key_free_fn(key_clone);
                }
                
                if (clone->value_free_fn) {
                    clone->value_free_fn(value_clone);
                }
                
                dict_delete(clone);
                return NULL;
            }
            
            cur = cur->next;
        }
    }
    
    return clone;
}

/**
 * Set an item on dict object. NOTE: If an insert fails, and you have
 * free functions set, this WONT call free() on the key/value. You are
 * responsible for maintaining that memory.
 *
 * @param   struct dict *dict
 * @param   char *key
 * @param   void *value
 * @return  int
 *
 * Returns 1 on success, and 0 on error.
 **/
int dict_set(struct dict *dict, char *key, void *value)
{
    struct dict_node *cur;
    struct dict_node *node;
    uint32_t hash;
    uint32_t idx;
    
    hash = crc32(dict->seed, key, strlen(key));
    idx = hash % dict->capacity;
    
    cur = &dict->table[idx];
    if (cur->key == NULL) {
        cur->hash = hash;
        cur->key = key;
        cur->value = value;
        dict->used++;
        return 1;
    } else if (cur->hash == hash && strcmp(cur->key, key) == 0) {
        // Free key?
        if (dict->key_free_fn) {
            dict->key_free_fn(cur->key);
        }

        // Free value?
        if (dict->value_free_fn) {
            dict->key_free_fn(cur->value);
        }
	
        cur->hash = hash;
        cur->key = key;
        cur->value = value;
        return 1;
    }
    
    node = malloc(sizeof(*node));
    if (node == NULL) {
        return 0;
    }
    
    node->hash = hash;
    node->key = key;
    node->value = value;
    node->next = cur->next;
    cur->next = node;
    dict->used++;
    
    return 1;
}

/**
 * Check if dict contains key.
 *
 * @param    struct dict *dict
 * @param    char *key
 * @return   int
 *
 * Returns 1 if dict contains key, and 0 otherwise.
 **/
int dict_contains(struct dict *dict, char *key)
{
    struct dict_node *cur;
    uint32_t hash;
    uint32_t idx;
    
    hash = crc32(dict->seed, key, strlen(key));
    idx = hash % dict->capacity;
    cur = &dict->table[idx];
    
    if (cur->key == NULL) {
        return 0;
    }
    
    do {
        if (cur->hash == hash && strcmp(cur->key, key) == 0) {
            return 1;
        }
        
        cur = cur->next;
    } while (cur);
    
    return 0;
}

/**
 * Get an item from dict.
 *
 * @param   struct dict *dict
 * @param   char *key
 * @return  struct dict_node *
 *
 * Returns a pointer to the specified node, or NULL if it is not found.
 **/
struct dict_node *dict_get(struct dict *dict, char *key)
{
    struct dict_node *cur;
    uint32_t hash;
    uint32_t idx;
    
    hash = crc32(dict->seed, key, strlen(key));
    idx = hash % dict->capacity;
    cur = &dict->table[idx];
    
    if (cur->key == NULL) {
        return NULL;
    }
    
    do {
        if (cur->hash == hash && strcmp(cur->key, key) == 0) {
            break;
        }
        
        cur = cur->next;
    } while (cur);
    
    return cur;
}

/**
 * Deletes an item from dict.
 *
 * @param   struct dict *dict
 * @param   char *key
 * @return  int
 *
 * Returns 1 on successful delete, and 0 otherwise.
 **/
int dict_del(struct dict *dict, char *key)
{
    struct dict_node *cur;
    struct dict_node *prev = NULL;
    uint32_t hash;
    uint32_t idx;
    int status = 0;
    
    hash = crc32(dict->seed, key, strlen(key));
    idx = hash % dict->capacity;
    cur = &dict->table[idx];
    
    if (cur->key == NULL) {
        return status;
    }
    
    do {
        if (cur->hash == hash && strcmp(cur->key, key) == 0) {
            if (dict->key_free_fn) {
                dict->key_free_fn(cur->key);
            }
			
            if (dict->value_free_fn) {
                dict->value_free_fn(cur->value);
            }
		
            if (prev != NULL) {
                prev->next = cur->next;
                free(cur);
                cur = prev;
            } else {
                if (cur->next != NULL) {
                    cur->hash = cur->next->hash;
                    cur->key = cur->next->key;
                    cur->value = cur->next->value;
                    cur->next = cur->next->next;
                } else {
                    cur->hash = 0;
                    cur->key = NULL;
                    cur->value = NULL;
                }
            }
            
            dict->used--;
            status = 1;
        }
        
        prev = cur;
        cur = cur->next;
    } while (cur);
    
    return status;
}

/**
 * Start iterating over dict object.
 *
 * @param   struct dict *dict
 * @param   struct dict_iterator *it
 * @return  void
 **/
void dict_iterate_start(struct dict *dict, struct dict_iterator *it)
{
    it->dict = dict;
    it->idx = 0;
    it->cur = NULL;
}

/**
 * Iterate to next item in dict. Modifying a dict, while using this method has
 * undefined behavior.
 *
 * @param   struct dict_iterator *it
 * @return  struct dict_node *
 *
 * Returns a pointer to struct dict_node *, or NULL if iteration has
 * completed.
 **/
struct dict_node *dict_iterate_next(struct dict_iterator *it)
{
    struct dict_node *prev;

    if (it->cur != NULL) {
        prev = it->cur;
        it->cur = prev->next;
        return prev;
    }

    while (it->idx < it->dict->capacity) {
        if (it->dict->table[it->idx].key != NULL) {
            prev = &it->dict->table[it->idx];
            it->cur = prev->next;
            it->idx++;
            return prev;
        }
        
        it->idx++;
    }
    
    return NULL;
}

/**
 * Iterate, using difference. Modifying a dict, while using this method has
 * undefined behavior.
 *
 * @param   struct dict *b
 * @param   struct dict_iterator *it
 * @return  struct dict_node *
 *
 * Returns a pointer to struct dict_node *, or NULL if iteration has
 * completed.
 **/
struct dict_node *dict_iterate_difference(struct dict *b, struct dict_iterator *it)
{
    struct dict_node *node;
    
    while (1) {
        node = dict_iterate_next(it);
        if (node == NULL) {
            return NULL;
        }
        
        if (!dict_contains(b, node->key)) {
            return node;
        }
    }
}

/**
 * Iterate, using intersection. Modifying a dict, while using this method has
 * undefined behavior.
 *
 * @param   struct dict *b
 * @param   struct dict_iterator *it
 * @return  struct dict_node *
 *
 * Returns a pointer to struct dict_node *, or NULL if iteration has
 * completed.
 **/
struct dict_node *dict_iterate_intersection(struct dict *b, struct dict_iterator *it)
{
    struct dict_node *node;
    
    while (1) {
        node = dict_iterate_next(it);
        if (node == NULL) {
            return NULL;
        }
        
        if (dict_contains(b, node->key)) {
            return node;
        }
    }
}
