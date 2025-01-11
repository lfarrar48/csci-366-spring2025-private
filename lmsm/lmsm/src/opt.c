
#include "lmsm/opt.h"
#include "lmsm/asm.h"

typedef struct label {
    const msu_str_t *name;
    const msu_str_t *alias;
    struct label *next;
} label_t;

void label_add(label_t **label, const msu_str_t *name, const msu_str_t *alias) {
    label_t *new_label = malloc(sizeof(label_t));
    new_label->name = msu_str_clone(name);
    new_label->alias = msu_str_clone(alias);
    new_label->next = *label;
    *label = new_label;
}

const msu_str_t *label_substitute(label_t *labels, const msu_str_t *name) {
    const msu_str_t *original_name = name;
    label_t *current = labels;
    while (current) {
        if (msu_str_eq(current->name, name)) {
            name = current->alias;
        }
        current = current->next;
    }
    if (name == original_name) {
        return NULL;
    } else {
        return name;
    }
}

void label_rename(label_t *label, asm_insr_t *insr) {
    if (!insr->label_reference) return;
    while (label) {
        const msu_str_t *new_label_ref = label_substitute(label, insr->label_reference);
        if (new_label_ref) {
            insr->label_reference = new_label_ref;
            return;
        }
        label = label->next;
    }
}

void label_free_one(label_t *label) {
    if (label) {
        msu_str_free(label->name);
        msu_str_free(label->alias);
    }
}

void label_free_all(label_t *label) {
    while (label) {
        label_free_one(label);
        label = label->next;
    }
}


void transfer_errors(const asm_insr_t *removed[], size_t len_removed, asm_insr_t *next) {
    for (int i = 0; i < len_removed; ++i) {
        const asm_insr_t *insr = removed[i];
        if (insr->error) {
            next->error = asm_error_clone(insr->error);
            return;
        }
    }
}

void transfer_labels(const asm_insr_t *removed[], size_t len_removed, asm_insr_t *next, label_t **subs) {
    for (int i = 0; i < len_removed; ++i) {
        const asm_insr_t *insr = removed[i];
        if (msu_str_is_empty(insr->label)) continue;

        if (msu_str_is_empty(next->label)) {
            next->label = msu_str_clone(insr->label);
        } else {
            label_add(subs, insr->label, next->label);
        }
    }
}

void push_pop_pass(const list_of_asm_insrs_t *insrs, list_of_asm_insrs_t *out) {
    label_t *subs = NULL;

    asm_insr_t *next = NULL;
    for (int i = 1; i < insrs->len; ++i) {
        const asm_insr_t *prev = list_of_asm_insrs_get_const(insrs, i - 1);
        const asm_insr_t *current = next ? next : list_of_asm_insrs_get_const(insrs, i);
        next = NULL;

        if (!(
                msu_str_eqs(prev->instruction, "SPUSH") && msu_str_eqs(current->instruction, "SPOP")
                || msu_str_eqs(prev->instruction, "SPOP") && msu_str_eqs(current->instruction, "SPUSH")
        )) {
            if (i == 1) {
                list_of_asm_insrs_append(out, asm_insr_clone(list_of_asm_insrs_get_const(insrs, 0)));
            }
            list_of_asm_insrs_append(out, asm_insr_clone(current));
            continue;
        }

        if (i + 1 < insrs->len) {
            next = asm_insr_clone(list_of_asm_insrs_get_const(insrs, i + 1));
        } else {
            next = malloc(sizeof(asm_insr_t));
            assert(next && "out of memory!\n");
            next->label = EMPTY_STRING;
            next->instruction = msu_str_new("NOP");
            next->label_reference = EMPTY_STRING;
            next->value = 0;
            next->error = NULL;
        }

        const asm_insr_t *removed[] = {prev, current};
        const size_t len_removed = sizeof(removed) / sizeof(removed[0]);

        transfer_errors(removed, len_removed, next);
        transfer_labels(removed, len_removed, next, &subs);
    }

    for (size_t i = 0; i < out->len; i++) {
        asm_insr_t *insr = list_of_asm_insrs_get(out, i);
        label_rename(subs, insr);
    }

    label_free_all(subs);
}

void pushi_pop_pass(const list_of_asm_insrs_t *insrs, list_of_asm_insrs_t *out) {
    label_t *subs = NULL;

    for (int i = 1; i < insrs->len; ++i) {
        const asm_insr_t *prev = list_of_asm_insrs_get_const(insrs, i - 1);
        const asm_insr_t *current = list_of_asm_insrs_get_const(insrs, i);

        if (!(msu_str_eqs(prev->instruction, "SPUSHI") && msu_str_eqs(current->instruction, "SPOP"))) {
            if (i == 1) {
                list_of_asm_insrs_append(out, asm_insr_clone(list_of_asm_insrs_get_const(insrs, 0)));
            }
            list_of_asm_insrs_append(out, asm_insr_clone(current));
            continue;
        }

        int value = prev->value;
        const msu_str_t *label_ref = prev->label_reference;

        asm_insr_t *new_insr = malloc(sizeof(asm_insr_t));
        assert(new_insr && "out of memory!\n");
        new_insr->label = EMPTY_STRING;
        new_insr->instruction = msu_str_new("LDI");
        new_insr->label_reference = msu_str_clone(label_ref);
        new_insr->value = value;
        new_insr->error = ASM_ERROR_NONE;
        list_of_asm_insrs_append(out, new_insr);

        const asm_insr_t *removed[] = {prev, current};
        const size_t len_removed = sizeof(removed) / sizeof(removed[0]);

        transfer_errors(removed, len_removed, new_insr);
        transfer_labels(removed, len_removed, new_insr, &subs);
    }

    for (size_t i = 0; i < out->len; i++) {
        asm_insr_t *insr = list_of_asm_insrs_get(out, i);
        label_rename(subs, insr);
    }

    label_free_all(subs);
}

typedef void (*optimizer_func)(const list_of_asm_insrs_t *, list_of_asm_insrs_t *);

list_of_asm_insrs_t *asm_optimize(const list_of_asm_insrs_t *insrs) {
    const size_t seed = 42;
    size_t hash = list_of_asm_insrs_hash(insrs, seed);

    const optimizer_func OPTIMIZERS[] = {push_pop_pass, pushi_pop_pass};
    const size_t LEN_OPTIMIZERS = sizeof(OPTIMIZERS) / sizeof(OPTIMIZERS[0]);

    list_of_asm_insrs_t *inp = list_of_asm_insrs_clone(insrs);
    list_of_asm_insrs_t *modified = list_of_asm_insrs_new();
    while (1) {
        for (size_t i = 0; i < LEN_OPTIMIZERS; i++) {
            optimizer_func func = OPTIMIZERS[i];
            func(inp, modified);

            // swap(inp, modified);
            list_of_asm_insrs_t *tmp = inp;
            inp = modified;
            modified = tmp;

            modified->len = 0;
        }

        size_t newhash = list_of_asm_insrs_hash(inp, seed);
        if (hash == newhash) break;
        hash = newhash;
    }

    list_of_asm_insrs_free(modified, true);
    return inp;
}