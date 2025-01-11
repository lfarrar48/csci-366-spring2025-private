#include "gtest/gtest.h"

extern "C" {
#include "msulib/str.h"
#include "msulib/alloc.h"

#define BT_IMPL
#define BT_NAME btree
#define BT_KEY const msu_str_t *
#define BT_VALUE int
#define BT_HASHFUNC(x) msu_str_hash(x, 42)
#define BT_EQFUNC msu_str_eq
#include "templates/btree.h"
}

TEST(test_btree, test_creation) {
    const msu_str_t *key1 = msu_str_new("hello");
    const msu_str_t *key2 = msu_str_new("world");

    allocator_t alloc = managed_heap_alloc_new();

    btree_t *tree = btree_new_in(alloc);
    btree_insert(tree, key1, 12);
    btree_insert(tree, key2, 13);

    btree_iter_t iter = btree_iter(tree, BT_TRAVERSE_INSERTION);
    while (btree_iter_has_next(&iter)) {
        btree_entry_t *entry = btree_iter_next(&iter);
        const msu_str_t *key = btree_entry_key(entry);
        int value = btree_entry_value(entry);
        printf("got %s %d\n", msu_str_data(key), value);
    }

    putchar('\n');

    btree_update(tree, key1, 14);
    iter = btree_iter(tree, BT_TRAVERSE_INSERTION);
    while (btree_iter_has_next(&iter)) {
        btree_entry_t *entry = btree_iter_next(&iter);
        const msu_str_t *key = btree_entry_key(entry);
        int value = btree_entry_value(entry);
        printf("got %s %d\n", msu_str_data(key), value);
    }

    putchar('\n');

    btree_remove(tree, key1, NULL);
    iter = btree_iter(tree, BT_TRAVERSE_INSERTION);
    while (btree_iter_has_next(&iter)) {
        btree_entry_t *entry = btree_iter_next(&iter);
        const msu_str_t *key = btree_entry_key(entry);
        int value = btree_entry_value(entry);
        printf("got %s %d\n", msu_str_data(key), value);
    }

    btree_free(tree);

    size_t alive = managed_heap_alloc_debug(&alloc);
    ASSERT_EQ(alive, 0);
    managed_heap_alloc_free(&alloc);
}

#define random(cap) ((((size_t) rand()) * ((size_t) rand())) % (cap))

TEST(test_btree, test_stuff) {
    const size_t n = 1000;
    auto strings = new const msu_str_t *[n];

    btree_t *tree = btree_new();
    for (size_t i = 0; i < n; i++) {
        const msu_str_t *s = msu_str_printf("%zu", i);
        strings[i] = s;
        btree_insert(tree, s, (int) i);
    }

    ASSERT_EQ(btree_size(tree), n);

    btree_iter_t iter = btree_iter(tree, BT_TRAVERSE_INSERTION);
    for (size_t i = 0; i < n; i++) {
        ASSERT_TRUE(btree_iter_has_next(&iter));
        btree_entry_t *entry = btree_iter_next(&iter);
        int value = btree_entry_value(entry);
        ASSERT_EQ(i, (size_t) value);
    }
    ASSERT_FALSE(btree_iter_has_next(&iter));

    size_t n_removed = 0;
    for (size_t i = 0; i < n; i += 2) {
        ASSERT_TRUE(btree_remove(tree, strings[i], nullptr));
        n_removed += 1;
    }
    ASSERT_EQ(btree_size(tree), n - n_removed);

    printf("\n");

    iter = btree_iter(tree, BT_TRAVERSE_INSERTION);
    for (size_t i = 1; i < n; i += 2) {
        ASSERT_TRUE(btree_iter_has_next(&iter));
        btree_entry_t *entry = btree_iter_next(&iter);
        int value = btree_entry_value(entry);
        ASSERT_EQ(value, i);
    }
    ASSERT_FALSE(btree_iter_has_next(&iter));
    fflush(stdout);

    for (int i = 0; i < n; ++i) {
        msu_str_free(strings[i]);
    }
    delete[] strings;
}