
#ifndef BT_KEY
#error "expected 'BT_KEY' to specify bt key type"
#endif

#ifndef BT_VALUE
#error "expected 'BT_VALUE' to specify bt value type"
#endif

#ifndef BT_HASHFUNC
#error "expected 'BT_HASHFUNC' to specify hash function for key"
#endif

#ifndef BT_EQFUNC
#error "expected 'BT_EQFUNC' to specify equality function for two keys"
#endif

#ifndef BT_NAME
#error "expected 'BT_NAME' to specify bt type name"
#endif

#ifndef BT_FREE_KEY
#define BT_FREE_KEY(k)
#endif

#ifndef BT_FREE_VALUE
#define BT_FREE_VALUE(v)
#endif

#define BT_IDENT2(a, b) a ## b
#define BT_IDENT(a, b) BT_IDENT2(a, b)

#include <stdbool.h>
#include <msulib/alloc.h>

#ifndef BTREE_H
#define BTREE_H

typedef enum bt_traversal {
    BT_TRAVERSE_INSERTION,
    BT_TRAVERSE_BREADTH_FIRST,
    BT_TRAVERSE_DEPTH_FIRST,
} bt_traversal_t;

#endif // BTREE_H

#define bt_t BT_IDENT(BT_NAME, _t)
#define BT_NODE BT_IDENT(BT_NAME, _node)
#define bt_node_t BT_IDENT(BT_NAME, _node_t)
#define BT_ENTRY BT_IDENT(BT_NAME, _entry)
#define bt_entry_t BT_IDENT(BT_NAME, _entry_t)
#define BT_ITER BT_IDENT(BT_NAME, _iter)
#define bt_iter_t BT_IDENT(BT_NAME, _iter_t)
#define bt_hash_fn BT_HASHFUNC
#define bt_eq_fn BT_EQFUNC

#define bt_new BT_IDENT(BT_NAME, _new)
#define bt_new_in BT_IDENT(BT_NAME, _new_in)
#define bt_get BT_IDENT(BT_NAME, _get)
#define bt_getv BT_IDENT(BT_NAME, _getv)
#define bt_insert BT_IDENT(BT_NAME, _insert)
#define bt_update BT_IDENT(BT_NAME, _update)
#define bt_remove BT_IDENT(BT_NAME, _remove)
#define bt_contains BT_IDENT(BT_NAME, _contains)
#define bt_size BT_IDENT(BT_NAME, _size)
#define bt_iter BT_IDENT(BT_NAME, _iter)

#define bt_free BT_IDENT(BT_NAME, _free)
#define bt_node_free BT_IDENT(BT_NODE, _free)
#define bt_entry_free_all BT_IDENT(BT_ENTRY, _free_all)
#define bt_entry_free_one BT_IDENT(BT_ENTRY, _free_one)

#define bt_iter_has_next BT_IDENT(BT_ITER, _has_next)
#define bt_iter_next BT_IDENT(BT_ITER, _next)

#define bt_entry_key BT_IDENT(BT_ENTRY, _key)
#define bt_entry_value BT_IDENT(BT_ENTRY, _value)

typedef struct BT_NAME bt_t;
typedef struct BT_NODE bt_node_t;
typedef struct BT_ENTRY bt_entry_t;
typedef struct BT_ITER {
    bt_t *bt;
    bt_entry_t *entry;
    bt_traversal_t order;
} bt_iter_t;

bt_t *bt_new();
bt_t *bt_new_in(allocator_t allocator);
bool bt_contains(const bt_t *bt, BT_KEY key);
bt_entry_t *bt_get(bt_t *bt, BT_KEY key);
BT_VALUE *bt_getv(bt_t *bt, BT_KEY key);
bool bt_insert(bt_t *bt, BT_KEY key, BT_VALUE value);
BT_VALUE bt_update(bt_t *bt, BT_KEY key, BT_VALUE value);
bool bt_remove(bt_t *bt, BT_KEY key, BT_VALUE *value_out);
size_t bt_size(const bt_t *bt);
void bt_free(bt_t *bt);

BT_KEY bt_entry_key(bt_entry_t *entry);
BT_VALUE bt_entry_value(bt_entry_t *entry);

bt_iter_t bt_iter(bt_t *bt, bt_traversal_t ordering);
bool bt_iter_has_next(const bt_iter_t *iter);
bt_entry_t *bt_iter_next(bt_iter_t *iter);

#ifdef BT_IMPL

#include <stdlib.h>
#include <assert.h>

struct BT_ENTRY {
    BT_KEY key;
    BT_VALUE value;
    bt_entry_t *next, *prev; // bucket chain
    bt_entry_t *forward, *reverse; // order chain
};

struct BT_NODE {
    void **pointers;
    size_t *keys;
    bt_node_t *parent;
    bool is_leaf;
    size_t num_keys;
    bt_entry_t *entry;
};

struct BT_NAME {
    allocator_t allocator;
    bt_node_t *root;
    bt_entry_t *first;
    bt_entry_t *last;
    size_t size;
};

#define BT_PRIVATE(n) BT_IDENT(BT_IDENT(BT_IDENT(__, BT_NAME), _), n)

#define bt_find_leaf BT_PRIVATE(find_leaf)
#define bt_find BT_PRIVATE(find)

#define bt_get_left_index BT_PRIVATE(get_left_index)
#define bt_cut BT_PRIVATE(cut)

#define bt_make_node BT_PRIVATE(make_node)
#define bt_make_leaf BT_PRIVATE(make_leaf)
#define bt_make_entry BT_PRIVATE(make_entry)
#define bt_start_new_tree BT_PRIVATE(start_new_tree)

#define bt_insert_into_leaf BT_PRIVATE(insert_into_leaf)
#define bt_insert_into_new_root BT_PRIVATE(insert_into_new_root)
#define bt_insert_into_node BT_PRIVATE(insert_into_node)
#define bt_insert_into_parent BT_PRIVATE(insert_into_parent)
#define bt_insert_into_node_after_splitting BT_PRIVATE(insert_into_node_after_splitting)
#define bt_insert_into_parent BT_PRIVATE(insert_into_parent)
#define bt_insert_into_leaf_after_splitting BT_PRIVATE(insert_into_leaf_after_splitting)

#define bt_adjust_root BT_PRIVATE(adjust_root)
#define bt_coalesce_nodes BT_PRIVATE(coalesce_nodes)
#define bt_redistribute_nodes BT_PRIVATE(redistribute_nodes)
#define bt_delete_entry BT_PRIVATE(delete_entry)
#define bt_remove_entry_from_node BT_PRIVATE(remove_entry_from_node)
#define bt_get_neighbor_index BT_PRIVATE(get_neighbor_index)

#define BT_ORDER BT_PRIVATE(BT_SIZE)
const size_t BT_ORDER = 4;
bt_node_t *bt_find_leaf(bt_node_t *root, size_t key);
bt_entry_t *bt_find(bt_node_t *root, size_t has, BT_KEY key, bt_node_t **leaf_out);
bt_node_t *bt_make_node(allocator_t allocator);
bt_node_t *bt_make_leaf(allocator_t allocator);
bt_entry_t *bt_make_entry(bt_t *bt, BT_KEY key, BT_VALUE value);
bt_node_t *bt_start_new_tree(allocator_t allocator, size_t key, bt_entry_t *entry);
void bt_insert_into_leaf(bt_node_t *leaf, size_t key, bt_entry_t *entry);
size_t bt_cut(size_t length);
bt_node_t *bt_insert_into_new_root(allocator_t allocator, bt_node_t *left, size_t key, bt_node_t *right);
bt_node_t *bt_insert_into_node(bt_node_t *root, bt_node_t *node, size_t left_index, size_t key, bt_node_t *right);
int bt_get_left_index(bt_node_t *parent, bt_node_t *left);
bt_node_t *bt_insert_into_node_after_splitting(allocator_t allocator, bt_node_t *root, bt_node_t *old, size_t left_index, size_t key, bt_node_t *right);
bt_node_t* bt_insert_into_parent(allocator_t allocator, bt_node_t *root, bt_node_t *left, size_t key, bt_node_t *right);
bt_node_t *bt_insert_into_leaf_after_splitting(allocator_t allocator, bt_node_t *root, bt_node_t *leaf, size_t key, bt_entry_t *entry);

bt_node_t *bt_adjust_root(allocator_t allocator, bt_node_t *root);
bt_node_t *bt_coalesce_nodes(allocator_t allocator, bt_node_t *root, bt_node_t *node, bt_node_t *neighbor, size_t neighbor_index, size_t k_prime);
bt_node_t *bt_redistribute_nodes(bt_node_t *root, bt_node_t *node, bt_node_t *neighbor, size_t neighbor_index, size_t k_prime_index, size_t k_prime);
bt_node_t *bt_delete_entry(allocator_t allocator, bt_node_t *root, bt_node_t *node, size_t key, void *pointer);
bt_node_t *bt_remove_entry_from_node(bt_node_t *node, size_t key, bt_node_t *pointer);
size_t bt_get_neighbor_index(bt_node_t *node);

void bt_entry_free_all(allocator_t allocator, bt_entry_t *entry);
void bt_entry_free_one(allocator_t allocator, bt_entry_t *entry);
void bt_node_free(allocator_t allocator, bt_node_t *node);

bt_t *bt_new() {
    return bt_new_in(DEFAULT_ALLOCATOR);
}

bt_t *bt_new_in(allocator_t allocator) {
    bt_t *out = (bt_t *) MSU_ALLOC(allocator, sizeof(bt_t));
    assert(out && "out of memory!\n");
    out->allocator = allocator;
    out->root = NULL;
    out->first = NULL;
    out->last = NULL;
    out->size = 0;
    return out;
}

bool bt_contains(const bt_t *bt, BT_KEY key) {
    size_t hash = bt_hash_fn(key);
    bt_entry_t *entry = bt_find(bt->root, hash, key, NULL);
    return entry != NULL;
}

bt_entry_t *bt_get(bt_t *bt, BT_KEY key) {
    size_t hash = bt_hash_fn(key);
    bt_entry_t *entry = bt_find(bt->root, hash, key, NULL);
    return entry;
}

BT_VALUE *bt_getv(bt_t *bt, BT_KEY key) {
    size_t hash = bt_hash_fn(key);
    bt_entry_t *entry = bt_find(bt->root, hash, key, NULL);
    if (!entry) return NULL;
    return &entry->value;
}

BT_KEY bt_entry_key(bt_entry_t *entry) {
    return entry->key;
}

BT_VALUE bt_entry_value(bt_entry_t *entry) {
    return entry->value;
}

bt_iter_t bt_iter(bt_t *bt, bt_traversal_t order) {
    return (bt_iter_t) {
        .bt = bt,
        .entry = bt->first,
        .order = order,
    };
}

bool bt_iter_has_next(const bt_iter_t *iter) {
    return iter->entry != NULL;
}

bt_entry_t *bt_iter_next(bt_iter_t *iter) {
    assert(bt_iter_has_next(iter));
    if (iter->order == BT_TRAVERSE_INSERTION) {
        bt_entry_t *out = iter->entry;
        iter->entry = iter->entry->forward;
        return out;
    } else if (iter->order == BT_TRAVERSE_DEPTH_FIRST) {
        assert(0 && "unimplemented");
    } else if (iter->order == BT_TRAVERSE_BREADTH_FIRST) {
        assert(0 && "unimplemented");
    } else {
        assert(0 && "bad ordering");
    }
}

bool bt_insert(bt_t *bt, BT_KEY key, BT_VALUE value) {
    size_t hashkey = bt_hash_fn(key);
    bt_entry_t *entry = bt_find(bt->root, hashkey, key, NULL);
    if (entry) {
        bt_entry_t *last = NULL;
        while (entry) {
            if (bt_eq_fn(entry->key, key)) {
                entry->value = value;
                return false;
            }
            last = entry;
            entry = entry->next;
        }

        entry = bt_make_entry(bt, key, value);
        last->next = entry;
        entry->prev = last;
        bt->size += 1;
        return true;
    }

    entry = bt_make_entry(bt, key, value);
    if (!bt->root) {
        bt->root = bt_start_new_tree(bt->allocator, hashkey, entry);
        bt->size += 1;
        return true;
    }

    bt_node_t *leaf = bt_find_leaf(bt->root, hashkey);
    if (leaf->num_keys < BT_ORDER - 1) {
        bt_insert_into_leaf(leaf, hashkey, entry);
        bt->size += 1;
        return true;
    }

    bt->root = bt_insert_into_leaf_after_splitting(bt->allocator, bt->root, leaf, hashkey, entry);
    bt->size += 1;
    return true;
}

BT_VALUE bt_update(bt_t *bt, BT_KEY key, BT_VALUE value) {
    size_t hash = bt_hash_fn(key);
    bt_entry_t *entry = bt_find(bt->root, hash, key, NULL);
    assert(entry && "no such key");
    BT_VALUE out = entry->value;
    entry->value = value;
    return out;
}

bool bt_remove(bt_t *bt, BT_KEY key, BT_VALUE *out) {
    size_t hash = bt_hash_fn(key);
    bt_node_t *leaf = NULL;
    bt_entry_t *entry = bt_find(bt->root, hash, key, &leaf);
    if (entry && leaf) {
        if (!entry->prev && !entry->next) {
            bt->root = bt_delete_entry(bt->allocator, bt->root, leaf, hash, entry);
        } else {
            if (entry->prev) entry->prev->next = entry->next;
            else leaf->entry = entry->next;
            if (entry->next) entry->next->prev = entry->prev;
        }
        if (bt->first == entry) bt->first = entry->forward;
        if (bt->last == entry) bt->last = entry->reverse;
        if (entry->forward) entry->forward->reverse = entry->reverse;
        if (entry->reverse) entry->reverse->forward = entry->forward;

        if (out) *out = entry->value;
        bt_entry_free_one(bt->allocator, entry);
        bt->size -= 1;
        return true;
    }
    return false;
}

size_t bt_size(const bt_t *bt) {
    return bt->size;
}

void bt_entry_free_one(allocator_t allocator, bt_entry_t *entry) {
    if (entry) {
        BT_FREE_KEY(entry->key);
        BT_FREE_VALUE(entry->value);
        MSU_FREE(allocator, entry);
    }
}

void bt_entry_free_all(allocator_t allocator, bt_entry_t *entry) {
    while (entry) {
        bt_entry_t *next = entry->next;
        bt_entry_free_one(allocator, entry);
        entry = next;
    }
}

void bt_node_free(allocator_t allocator, bt_node_t *node) {
    if (!node) return;
    if (node->is_leaf) {
        for (size_t i = 0; i < node->num_keys; i++) {
            bt_entry_free_all(allocator, (bt_entry_t *) node->pointers[i]);
        }
    } else {
        for (size_t i = 0; i < node->num_keys; i++) {
            bt_node_free(allocator, (bt_node_t *) node->pointers[i]);
        }
    }

    MSU_FREE(allocator, node->pointers);
    MSU_FREE(allocator, node->keys);
    MSU_FREE(allocator, node);
}

void bt_free(bt_t *bt) {
    if (!bt) return;
    bt_node_free(bt->allocator, bt->root);
    MSU_FREE(bt->allocator, bt);
}

// INNER METHODS

bt_node_t *bt_find_leaf(bt_node_t *root, size_t key) {
    if (!root) return NULL;
    bt_node_t *node = root;
    while (!node->is_leaf) {
        size_t i;
        for (i = 0; i < node->num_keys; i++) {
            if (key < node->keys[i]) {
                break;
            }
        }
        node = (bt_node_t *) node->pointers[i];
    }
    return node;
}

bt_entry_t *bt_find(bt_node_t *root, size_t hash, BT_KEY key, bt_node_t **leaf_out) {
    if (!root) return NULL;

    bt_node_t *leaf = bt_find_leaf(root, hash);
    size_t i;
    for (i = 0; i < leaf->num_keys; i++) {
        if (leaf->keys[i] == hash) {
            break;
        }
    }
    if (i == leaf->num_keys) return NULL;

    bt_entry_t *entry = (bt_entry_t *) leaf->pointers[i];
    while (entry) {
        if (bt_eq_fn(entry->key, key)) {
            if (leaf_out) *leaf_out = leaf;
            return entry;
        }
        entry = entry->next;
    }
    if (leaf_out) *leaf_out = NULL;
    return NULL;
}

bt_node_t *bt_make_node(allocator_t allocator) {
    bt_node_t *node = (bt_node_t *) MSU_ALLOC(allocator, sizeof(bt_node_t));
    assert(node && "out of memory!\n");
    node->keys = (size_t *) MSU_ALLOC(allocator, sizeof(size_t) * (BT_ORDER - 1));
    assert(node->keys && "out of memory!\n");
    node->pointers = (void **) MSU_ALLOC(allocator, sizeof(void *) * BT_ORDER);
    assert(node->pointers && "out of memory!\n");
    node->is_leaf = false;
    node->num_keys = 0;
    node->parent = NULL;
    node->entry = NULL;
    return node;
}

bt_node_t *bt_make_leaf(allocator_t allocator) {
    bt_node_t *node = bt_make_node(allocator);
    node->is_leaf = true;
    return node;
}

bt_entry_t *bt_make_entry(bt_t *bt, BT_KEY key, BT_VALUE value) {
    bt_entry_t *entry = (bt_entry_t *) MSU_ALLOC(bt->allocator, sizeof(bt_entry_t));
    assert(entry && "out of memory!\n");
    entry->key = key;
    entry->value = value;
    entry->next = NULL;
    entry->prev= NULL;
    entry->forward = NULL;

    entry->reverse = bt->last;
    if (bt->last) {
        bt->last->forward = entry;
    } else {
        bt->first = entry;
    }
    bt->last = entry;

    return entry;
}

bt_node_t *bt_start_new_tree(allocator_t allocator, size_t key, bt_entry_t *entry) {
    bt_node_t *root = bt_make_leaf(allocator);
    root->keys[0] = key;
    root->pointers[0] = entry;
    root->pointers[BT_ORDER - 1] = NULL;
    root->parent = NULL;
    root->num_keys = 1;
    return root;
}

void bt_insert_into_leaf(bt_node_t *leaf, size_t key, bt_entry_t *entry) {
    size_t insertion_point;
    for (insertion_point = 0; insertion_point < leaf->num_keys; insertion_point++) {
        if (leaf->keys[insertion_point] >= key) {
            break;
        }
    }

    size_t i;
    for (i = leaf->num_keys; i > insertion_point; i--) {
        leaf->keys[i] = leaf->keys[i - 1];
        leaf->pointers[i] = leaf->pointers[i - 1];
    }
    leaf->keys[insertion_point] = key;
    leaf->pointers[insertion_point] = entry;
    leaf->num_keys += 1;
}

size_t bt_cut(size_t length) {
    if (length % 2 == 0) {
        return length / 2;
    } else {
        return 1 + length / 2;
    }
}

bt_node_t *bt_insert_into_new_root(allocator_t allocator, bt_node_t *left, size_t key, bt_node_t *right) {
    bt_node_t *node = bt_make_node(allocator);
    node->keys[0] = key;
    node->pointers[0] = left;
    node->pointers[1] = right;
    node->num_keys = 1;
    node->parent = NULL;
    left->parent = node;
    right->parent = node;
    return node;
}

bt_node_t *bt_insert_into_node(bt_node_t *root, bt_node_t *node, size_t left_index, size_t key, bt_node_t *right) {
    size_t i;
    for (i = node->num_keys; i > left_index; i--) {
        node->pointers[i + 1] = node->pointers[i];
        node->keys[i] = node->keys[i - 1];
    }
    node->pointers[left_index + 1] = right;
    node->keys[left_index] = key;
    node->num_keys += 1;
    return root;
}

int bt_get_left_index(bt_node_t *parent, bt_node_t *left) {
    size_t left_index = 0;
    for (left_index = 0; left_index <= parent->num_keys; left_index++) {
        if (parent->pointers[left_index] == left) {
            break;
        }
    }
    return left_index;
}

bt_node_t *bt_insert_into_node_after_splitting(allocator_t allocator, bt_node_t *root, bt_node_t *old, size_t left_index, size_t key, bt_node_t *right) {
    void **pointers = (void **) MSU_ALLOC(allocator, sizeof(void *) * (BT_ORDER + 1));
    assert(pointers && "out of memory!\n");
    size_t *keys = (size_t *) MSU_ALLOC(allocator, sizeof(size_t) * BT_ORDER);
    assert(keys && "out of memory!\n");

    size_t i;
    for (i = 0; i < old->num_keys + 1; i++) {
        size_t j = i >= left_index + 1 ? i + 1 : i;
        pointers[j] = old->pointers[i];
    }

    for (i = 0; i < old->num_keys; i++) {
        size_t j = i >= left_index ? i + 1 : i;
        keys[j] = old->keys[i];
    }

    pointers[left_index + 1] = right;
    keys[left_index] = key;

    size_t split = bt_cut(BT_ORDER);
    size_t k_prime = keys[split - 1];

    bt_node_t *new_node = bt_make_node(allocator);
    old->num_keys = 0;
    for (i = 0; i < split - 1; i++) {
        old->pointers[i] = pointers[i];
        old->keys[i] = keys[i];
        old->num_keys += 1;
    }
    old->pointers[i] = pointers[i]; // this is correct, i++ makes `i = split - 1` at the end

    for (i = split; i < BT_ORDER; i++) {
        new_node->pointers[i - split] = pointers[i];
        new_node->keys[i - split] = keys[i];
        new_node->num_keys += 1;
    }
    new_node->pointers[i - split] = pointers[i]; // also correct, see comment line 7 above

    MSU_FREE(allocator, pointers);
    MSU_FREE(allocator, keys);

    new_node->parent = old->parent;
    for (i = 0; i <= new_node->num_keys; i++) {
        bt_node_t *child = (bt_node_t *) new_node->pointers[i];
        child->parent = new_node;
    }

    return bt_insert_into_parent(allocator, root, old, k_prime, new_node);
}

bt_node_t* bt_insert_into_parent(allocator_t allocator, bt_node_t *root, bt_node_t *left, size_t key, bt_node_t *right) {
    bt_node_t *parent = left->parent;
    if (!parent) {
        return bt_insert_into_new_root(allocator, left, key, right);
    }

    size_t left_index = bt_get_left_index(parent, left);
    if (parent->num_keys < BT_ORDER - 1) {
        return bt_insert_into_node(root, parent, left_index, key, right);
    }

    return bt_insert_into_node_after_splitting(allocator, root, parent, left_index, key, right);
}

bt_node_t *bt_insert_into_leaf_after_splitting(allocator_t allocator, bt_node_t *root, bt_node_t *leaf, size_t key, bt_entry_t *entry) {
    void **pointers = (void **) MSU_ALLOC(allocator, sizeof(void *) * BT_ORDER);
    assert(pointers && "out of memory!\n");
    size_t *keys = (size_t *) MSU_ALLOC(allocator, sizeof(size_t) * BT_ORDER);
    assert(keys && "out of memory!\n");

    size_t insertion_index;
    for (insertion_index = 0; insertion_index < BT_ORDER - 1; insertion_index++) {
        if (leaf->keys[insertion_index] >= key) {
            break;
        }
    }

    size_t i;
    for (i = 0; i < leaf->num_keys; i++) {
        size_t j = i >= insertion_index ? i + 1 : i;
        keys[j] = leaf->keys[i];
        pointers[j] = leaf->pointers[i];
    }
    keys[insertion_index] = key;
    pointers[insertion_index] = entry;
    leaf->num_keys = 0;

    size_t split = bt_cut(BT_ORDER - 1);
    for (i = 0; i < split; i++) {
        leaf->pointers[i] = pointers[i];
        leaf->keys[i] = keys[i];
        leaf->num_keys += 1;
    }

    bt_node_t *new_leaf = bt_make_leaf(allocator);
    for (i = split; i < BT_ORDER; i++) {
        new_leaf->pointers[i - split] = pointers[i];
        new_leaf->keys[i - split] = keys[i];
        new_leaf->num_keys += 1;
    }

    MSU_FREE(allocator, pointers);
    MSU_FREE(allocator, keys);

    new_leaf->pointers[BT_ORDER - 1] = leaf->pointers[BT_ORDER - 1];
    leaf->pointers[BT_ORDER - 1] = new_leaf;

    for (i = leaf->num_keys; i < BT_ORDER - 1; i++) {
        leaf->pointers[i] = NULL;
    }
    for (i = new_leaf->num_keys; i < BT_ORDER - 1; i++) {
        new_leaf->pointers[i] = NULL;
    }

    new_leaf->parent = leaf->parent;
    size_t new_key = new_leaf->keys[0];
    return bt_insert_into_parent(allocator, root, leaf, new_key, new_leaf);
}

bt_node_t *bt_adjust_root(allocator_t allocator, bt_node_t *root) {
    if (root->num_keys > 0)
        return root;

    bt_node_t *new_root;
    if (!root->is_leaf) {
        new_root = (bt_node_t *) root->pointers[0];
        new_root->parent = NULL;
    } else {
        new_root = NULL;
    }

    MSU_FREE(allocator, root->keys);
    MSU_FREE(allocator, root->pointers);
    MSU_FREE(allocator, root);

    return new_root;
}

bt_node_t *bt_coalesce_nodes(allocator_t allocator, bt_node_t *root, bt_node_t *node, bt_node_t *neighbor, size_t neighbor_index,
                             size_t k_prime)
{
    if (neighbor_index == -1) {
        bt_node_t *tmp = node;
        node = neighbor;
        neighbor = tmp;
    }

    size_t neighbor_insertion_index = neighbor->num_keys;
    if (!node->is_leaf) {
        neighbor->keys[neighbor_insertion_index] = k_prime;
        neighbor->num_keys += 1;

        size_t n_end = node->num_keys;
        size_t i, j;
        for (i = neighbor_insertion_index + 1, j = 0; j < n_end; i++, j++) {
            neighbor->keys[i] = node->keys[j];
            neighbor->pointers[i] = node->pointers[j];
            neighbor->num_keys += 1;
            node->num_keys -= 1;
        }
        neighbor->pointers[i] = node->pointers[j];

        for (i = 0; i < neighbor->num_keys + 1; i++) {
            ((bt_node_t *) neighbor->pointers[i])->parent = neighbor;
        }
    } else {
        size_t i, j;
        for (i = neighbor_insertion_index, j = 0; j < node->num_keys; i++, j++) {
            neighbor->keys[i] = node->keys[j];
            neighbor->pointers[i] = node->pointers[j];
            neighbor->num_keys += 1;
        }
        neighbor->pointers[BT_ORDER - 1] = node->pointers[BT_ORDER - 1];
    }

    root = bt_delete_entry(allocator, root, node->parent, k_prime, node);
    MSU_FREE(allocator, node->keys);
    MSU_FREE(allocator, node->pointers);
    MSU_FREE(allocator, node);
    return root;
}

bt_node_t *bt_redistribute_nodes(bt_node_t *root, bt_node_t *node, bt_node_t *neighbor, size_t neighbor_index,
                                 size_t k_prime_index, size_t k_prime)
{
    if (neighbor_index != -1) {
        if (!node->is_leaf) {
            node->pointers[node->num_keys + 1] = node->pointers[node->num_keys];
        }

        for (size_t i = node->num_keys; i > 0; i--) {
            node->keys[i] = node->keys[i - 1];
            node->pointers[i] = node->pointers[i - 1];
        }

        if (!node->is_leaf) {
            node->pointers[0] = neighbor->pointers[neighbor->num_keys];
            bt_node_t *tmp = (bt_node_t *) node->pointers[0];
            tmp->parent = node;
            neighbor->pointers[neighbor->num_keys] = NULL;
            node->keys[0] = k_prime;
            node->parent->keys[k_prime_index] = neighbor->keys[neighbor->num_keys - 1];
        } else {
            node->pointers[0] = neighbor->pointers[neighbor->num_keys - 1];
            neighbor->pointers[neighbor->num_keys - 1] = NULL;
            node->keys[0] = neighbor->keys[neighbor->num_keys - 1];
            node->parent->keys[k_prime_index] = node->keys[0];
        }
    } else {
        if (node->is_leaf) {
            node->keys[node->num_keys] = neighbor->keys[0];
            node->pointers[node->num_keys] = neighbor->pointers[0];
            node->parent->keys[k_prime_index] = neighbor->keys[1];
        } else {
            node->keys[node->num_keys] = k_prime;
            node->pointers[node->num_keys + 1] = neighbor->pointers[0];
            bt_node_t *tmp = (bt_node_t *) node->pointers[node->num_keys + 1];
            tmp->parent = node;
            node->parent->keys[k_prime_index] = neighbor->keys[0];
        }
        size_t i;
        for (i = 0; i < neighbor->num_keys - 1; i++) {
            neighbor->keys[i] = neighbor->keys[i + 1];
            neighbor->pointers[i] = neighbor->pointers[i + 1];
        }
        if (!node->is_leaf) {
            neighbor->pointers[i] = neighbor->pointers[i + 1];
        }
    }

    node->num_keys += 1;
    neighbor->num_keys -= 1;

    return root;
}

bt_node_t *bt_delete_entry(allocator_t allocator, bt_node_t *root, bt_node_t *node, size_t key, void *pointer) {
    node = bt_remove_entry_from_node(node, key, (bt_node_t *) pointer);
    if (node == root)
        return bt_adjust_root(allocator, root);

    size_t min_keys = node->is_leaf ? bt_cut(BT_ORDER - 1) : bt_cut(BT_ORDER) - 1;
    if (node->num_keys >= min_keys)
        return root;

    size_t neighbor_index = bt_get_neighbor_index(node);
    size_t k_prime_index = neighbor_index == -1 ? 0 : neighbor_index;
    size_t k_prime = node->parent->keys[k_prime_index];
    bt_node_t *neighbor = neighbor_index == -1 ? (bt_node_t *) node->parent->pointers[1] :
                          (bt_node_t *) node->parent->pointers[neighbor_index];
    size_t capacity = node->is_leaf ? BT_ORDER : BT_ORDER - 1;

    if (neighbor->num_keys + node->num_keys < capacity)
        return bt_coalesce_nodes(allocator, root, node, neighbor, neighbor_index, k_prime);
    else
        return bt_redistribute_nodes(root, node, neighbor, neighbor_index, k_prime_index, k_prime);
}

bt_node_t *bt_remove_entry_from_node(bt_node_t *node, size_t key, bt_node_t *pointer) {
    size_t i = 0;
    while (node->keys[i] != key) {
        i++;
    }
    for (++i; i < node->num_keys; i++) {
        node->keys[i - 1] = node->keys[i];
    }

    size_t num_pointers = node->is_leaf ? node->num_keys : node->num_keys + 1;
    i = 0;
    while (node->pointers[i] != pointer) {
        i += 1;
    }
    for (++i; i < num_pointers; i++) {
        node->pointers[i - 1] = node->pointers[i];
    }

    node->num_keys -= 1;

    if (node->is_leaf) {
        for (i = node->num_keys; i < BT_ORDER - 1; i++)
            node->pointers[i] = NULL;
    } else {
        for (i = node->num_keys + 1; i < BT_ORDER; i++) {
            node->pointers[i] = NULL;
        }
    }
    return node;
}

size_t bt_get_neighbor_index(bt_node_t *node) {
    size_t i;
    for (i = 0; i <= node->parent->num_keys; i++) {
        if (node->parent->pointers[i] == node) {
            return i - 1;
        }
    }

    fprintf(stderr, "Search for nonexistent pointer to node in parent.\n");
    fprintf(stderr, "Node:  %#zx\n", (size_t) node);
    exit(EXIT_FAILURE);
}

#undef BT_PRIVATE
#undef bt_find_leaf
#undef bt_find
#undef bt_get_left_index
#undef bt_cut
#undef bt_make_node
#undef bt_make_leaf
#undef bt_make_entry
#undef bt_start_new_tree
#undef bt_insert_into_leaf
#undef bt_insert_into_new_root
#undef bt_insert_into_node
#undef bt_insert_into_parent
#undef bt_insert_into_node_after_splitting
#undef bt_insert_into_parent
#undef bt_insert_into_leaf_after_splitting
#undef bt_adjust_root
#undef bt_coalesce_nodes
#undef bt_redistribute_nodes
#undef bt_delete_entry
#undef bt_remove_entry_from_node
#undef bt_get_neighbor_index
#undef BT_ORDER

#endif // BT_IMPL

#undef BT_KEY
#undef BT_VALUE
#undef BT_HASHFUNC
#undef BT_EQFUNC
#undef BT_NAME
#undef BT_FREE_KEY
#undef BT_FREE_VALUE
#undef BT_IDENT2
#undef BT_IDENT
#undef bt_t
#undef BT_NODE
#undef bt_node_t
#undef BT_ENTRY
#undef bt_entry_t
#undef BT_ITER
#undef bt_iter_t
#undef bt_hash_fn
#undef bt_eq_fn
#undef bt_new
#undef bt_new_in
#undef bt_get
#undef bt_insert
#undef bt_update
#undef bt_remove
#undef bt_contains
#undef bt_size
#undef bt_iter
#undef bt_free
#undef bt_node_free
#undef bt_entry_free_all
#undef bt_entry_free_one
#undef bt_iter_has_next
#undef bt_iter_next
#undef bt_entry_key
#undef bt_entry_value
