import os
import sys
import subprocess

cwd = os.getcwd()
assert os.path.basename(cwd) == "lmsm" and os.path.exists("./lmsm/"), "must be run in (project_root)/lmsm"

HASH_ASM_INSR = """

size_t asm_insr_hash(const asm_insr_t *cur, size_t seed) {
    hash_t hashbean[5];
    hashbean[0] = msu_str_hash(cur->label, seed);
    hashbean[1] = msu_str_hash(cur->instruction, seed);
    hashbean[2] = murmurhash(&cur->value, sizeof(cur->value), seed);
    hashbean[3] = msu_str_hash(cur->label_reference, seed);
    hashbean[4] = murmurhash(&cur->error, sizeof(asm_error_t *), seed);
    return (size_t) murmurhash(hashbean, sizeof(hashbean), seed);
}

"""

# generate list of asm instructions
subprocess.run([
    sys.executable, # fancy way of saying use this python version
    "./lib/msulib/script/generate_list_impl.py",
    "--element-type", "asm_insr_t *",
    "--list-name", "list_of_asm_insrs",
    "--hpath", "./lmsm/inc/asm_insrlist.h",
    "--hdef", "typedef struct asm_insr asm_insr_t;",
    "--hdef", "size_t asm_insr_hash(const asm_insr_t *insr, size_t seed);",
    "--cdef", HASH_ASM_INSR,
    "--cpath", "./lmsm/src/asm_insrlist.c",
    "--cinclude", "lmsm/asm.h",
    "--free-fn", "asm_insr_free",
    "--hash-fn", "asm_insr_hash",
], check=True, stdout=subprocess.DEVNULL)

subprocess.run([
    sys.executable, # fancy way of saying use this python version
    "./lib/msulib/script/generate_list_impl.py",
    "--element-type", "const msu_str_t *",
    "--list-name", "list_of_msu_strs",
    "--hpath", "./lib/msulib/inc/msulib/strlist.h",
    "--hdef", "typedef struct msu_str msu_str_t;",
    "--cpath", "./lib/msulib/src/strlist.c",
    "--cinclude", "msulib/str.h",
    "--free-fn", "msu_str_free",
    "--hash-fn", "msu_str_hash",
    "--eq-fn", "msu_str_eq"
], check=True, stdout=subprocess.DEVNULL)

subprocess.run([
    sys.executable, # fancy way of saying use this python version
    "./lib/msulib/script/generate_list_impl.py",
    "--element-type", "parsenode_t *",
    "--list-name", "list_of_parsenodes",
    "--hpath", "./lib/msulib/inc/msulib/parsenodelist.h",
    "--hdef", "typedef struct parsenode parsenode_t;",
    "--cpath", "./lib/msulib/src/parsenodelist.c",
    "--cinclude", "msulib/parsenodelist.h",
    "--cinclude", "msulib/parser.h",
    "--free-fn", "parsenode_free",
], check=True, stdout=subprocess.DEVNULL)
