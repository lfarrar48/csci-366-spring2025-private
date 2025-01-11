#include "lang.h"

#include "msulib/parser.h"

#include "lmsm/sea.h"
#include "endpoints.h"
#include "lmsm/asm.h"
#include "lmsm/emulator.h"
#include "main.h"
#include "lmsm/firth.h"
#include "lmsm/zortran.h"

const msu_str_t *build_memory_view();
const msu_str_t *build_register_view();

bool find_and_report_errors(
    http_conn_t *conn,
    http_error_t *errout,
    const msu_str_t *src,
    const parsenode_t *program
) {
    list_of_parsenodes_t *errors = list_of_parsenodes_new();
    parsenode_collect_errors(program, errors);
    if (errors->len == 0) return false;

    msu_str_builder_t sb = msu_str_builder_new();
    msu_str_builder_pushs(sb, "<div hx-swap-oob='innerHTML:#code-output'>");
    for (size_t i = 0; i < errors->len; i++) {
        parsenode_t *node = list_of_parsenodes_get(errors, i);
        if (node->token) {
            size_t line = msu_str_get_lineno_for_index(src, node->token->index);
            size_t column = msu_str_get_lineoff_for_index(src, node->token->index);
            msu_str_builder_printf(sb, "(at %zu:%zu) ", line, column);
        } else {
            msu_str_builder_pushs(sb, "(at ??\?) ");
        }
        msu_str_builder_pushstr(sb, node->error);
        msu_str_builder_push(sb, '\n');
    }
    msu_str_builder_pushs(sb, "</div>");
    reply_html(conn, errout, HTTP_STATUS_OK, msu_str_builder_into_string_and_free(sb));
    return true;
}

void reply_err_asm(http_conn_t *conn, http_error_t *errout, asm_error_t *asm_err, const msu_str_t *bytecode) {
    const char *el = (
            "<div hx-swap-oob='innerHTML:#code-output'>%s</div>"
            "<div hx-swap-oob='innerHTML:#assembly-code'>%s</div>"
    );
    const msu_str_t *res = msu_str_printf(el, msu_str_data(asm_err->message), msu_str_data(bytecode));
    reply_html(conn, errout, HTTP_STATUS_OK, res);
}

void reply_err_compilation(http_conn_t *conn, http_error_t *errout, const char *lang, const msu_str_t *error_msg) {
    const char *el = (
            "<div hx-swap-oob='innerHTML:#code-output'>"
            "$ %sc pgm.%s\n"
            "error: %s\n"
            "</div>"
    );
    const msu_str_t *res = msu_str_printf(el, lang, lang, msu_str_data(error_msg));
    reply_html(conn, errout, HTTP_STATUS_OK, res);
}

void reply_success(http_conn_t *conn, http_error_t *errout, const char *lang, const msu_str_t *bytecode) {
    const char *el = (
            "<div hx-swap-oob='innerHTML:#code-output'>%s</div>"
            "<div hx-swap-oob='innerHTML:#assembly-code'>%s</div>"
            "<div hx-swap-oob='outerHTML:#memory-table'>%s</div>"
            "<div hx-swap-oob='outerHTML:#register-table'>%s</div>"
    );

    const msu_str_t *memory_view = build_memory_view();
    const msu_str_t *register_view = build_register_view();
    const msu_str_t *output = msu_str_printf("$ %sc pgm.%s\nok", lang, lang);

    const msu_str_t *res = msu_str_printf(el,
                                          msu_str_data(output),
                                          msu_str_data(bytecode),
                                          msu_str_data(memory_view),
                                          msu_str_data(register_view));
    reply_html(conn, errout, HTTP_STATUS_OK, res);

    msu_str_free(memory_view);
    msu_str_free(register_view);
    msu_str_free(output);
}

bool compile_sea(http_conn_t *conn, http_error_t *errout, const msu_str_t *src, bool load) {
    bool out = true;
    parsenode_t *program = NULL;
    const msu_str_t *bytecode = NULL;
    sea_error_t *sea_err = NULL;
    asm_error_t *asm_err = NULL;
    int *code = NULL;

    program = sea_parse(src);
    if (find_and_report_errors(conn, errout, src, program)) {
        out = false;
        goto end;
    }

    bytecode = sea_compile(program, &sea_err);
    if (sea_err) {
        reply_err_compilation(conn, errout, "sea", sea_err->message);
        out = false;
        goto end;
    }

    code = asm_assemble(bytecode, &asm_err);
    if (asm_err) {
        reply_err_asm(conn, errout, asm_err, bytecode);
        out = false;
        goto end;
    }

    if (load) {
        emulator_load(the_one_emulator, code, MIDDLE_OF_MEMORY);
    }

    reply_success(conn, errout, "sea", bytecode);
end:
    parsenode_free(program);
    msu_str_free(bytecode);
    sea_error_free(sea_err);
    asm_error_free(asm_err);
    free(code);
    return out;
}

bool compile_firth(http_conn_t *conn, http_error_t *errout, const msu_str_t *src, bool load) {
    bool out = true;
    parsenode_t *program = NULL;
    const msu_str_t *bytecode = NULL;
    asm_error_t *asm_err = NULL;
    int *code = NULL;

    program = fr_parse(src);
    if (find_and_report_errors(conn, errout, src, program)) {
        out = false;
        goto end;
    }

    bytecode = fr_compile_program(program);

    code = asm_assemble(bytecode, &asm_err);
    if (asm_err) {
        reply_err_asm(conn, errout, asm_err, bytecode);
        out = false;
        goto end;
    }

    if (load) {
        emulator_load(the_one_emulator, code, MIDDLE_OF_MEMORY);
    }

    reply_success(conn, errout, "firth", bytecode);
end:
    parsenode_free(program);
    msu_str_free(bytecode);
    asm_error_free(asm_err);
    free(code);
    return out;
}

bool compile_zortran(http_conn_t *conn, http_error_t *errout, const msu_str_t *src, bool load) {
    bool out = true;
    parsenode_t *program = NULL;
    const msu_str_t *bytecode = NULL;
    asm_error_t *asm_err = NULL;
    int *code = NULL;

    program = zt_parse(src);
    if (find_and_report_errors(conn, errout, src, program)) {
        out = false;
        goto end;
    }

    bytecode = zt_compile(program);

    code = asm_assemble(bytecode, &asm_err);
    if (asm_err) {
        reply_err_asm(conn, errout, asm_err, bytecode);
        out = false;
        goto end;
    }

    if (load) {
        emulator_load(the_one_emulator, code, MIDDLE_OF_MEMORY);
    }

    reply_success(conn, errout, "zt", bytecode);
end:
    parsenode_free(program);
    msu_str_free(bytecode);
    asm_error_free(asm_err);
    free(code);
    return out;
}

bool compile_asm(http_conn_t *conn, http_error_t *errout, const msu_str_t *src, bool load) {
    bool out = true;
    asm_error_t *asm_err = NULL;
    int *code = NULL;

    code = asm_assemble(src, &asm_err);
    if (asm_err) {
        reply_err_asm(conn, errout, asm_err, src);
        out = false;
        goto end;
    }

    if (load) {
        emulator_load(the_one_emulator, code, MIDDLE_OF_MEMORY);
    }

    reply_success(conn, errout, "asm", src);
end:
    asm_error_free(asm_err);
    free(code);
    return out;
}
