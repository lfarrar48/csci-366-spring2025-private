#include "endpoints.h"
#include "main.h"

#include "msulib/fs.h"
#include "lang.h"

const msu_str_t *explain_insr(int insr);


const msu_str_t *replace(const msu_str_t *haystack, const msu_str_t *needle, const msu_str_t *thread) {
    const msu_str_t *out = msu_str_replace_all(haystack, needle, thread);
    msu_str_free(haystack);
    return out;
}

const msu_str_t *build_memory_view() {
    const msu_str_t *help = explain_insr(the_one_emulator->memory[the_one_emulator->program_counter]);
    const msu_str_t *status_line = NULL;

    const char *status;
    if (the_one_emulator->status == STATUS_READY) {
        status = "ready";
    } else if (the_one_emulator->status == STATUS_RUNNING) {
        status = "running";
    } else if (the_one_emulator->status == STATUS_HALTED) {
        status = "halted";
    } else {
        status = "(bad state)";
    }

    const char *err;
    if (the_one_emulator->error_code == ERROR_NONE) {
        err = "(nil)";
    } else if (the_one_emulator->error_code == ERROR_BAD_STACK) {
        err = "bad stack";
    } else if (the_one_emulator->error_code == ERROR_OUTPUT_EXHAUSTED) {
        err = "output exhausted";
    } else if (the_one_emulator->error_code == ERROR_UNKNOWN_INSTRUCTION) {
        err = "unknown instruction";
    } else {
        err = "(nil)";
    }
    status_line = msu_str_printf("status: %s | error: %s\n", status, err);

    msu_str_builder_t memory = msu_str_builder_new();
    msu_str_builder_pushs(memory, "<div id='memory-table'>");

    msu_str_builder_printf(memory, "<pre>%s</pre>", msu_str_data(status_line));
    msu_str_builder_printf(memory, "<pre>%s</pre>", msu_str_data(help));
    msu_str_builder_pushs(memory, "<table>");
    msu_str_builder_pushs(memory, "<thead><th></th>");
    for (size_t i = 0; i < 10; i++) {
        msu_str_builder_printf(memory, "<th>%d</th>", i);
    }
    msu_str_builder_pushs(memory, "</thead><tbody>");
    for (size_t i = 0; i < 20; i++) {
        msu_str_builder_pushs(memory, "<tr>");
        msu_str_builder_printf(memory, "<th>%d</th>", i * 10);
        for (size_t j = 0; j < 10; j++) {
            size_t idx = i * 10 + j;
            int value = the_one_emulator->memory[idx];

            bool is_on_stack = idx >= the_one_emulator->stack_pointer;
            bool is_insr = idx == the_one_emulator->program_counter;
            bool is_ra = idx == the_one_emulator->return_address;
            bool is_rp = idx >= MIDDLE_OF_MEMORY && idx < the_one_emulator->return_stack_pointer;

            msu_str_builder_t classes = msu_str_builder_new();
            if (is_on_stack) msu_str_builder_printf(classes, "is_on_stack ");
            if (is_insr) msu_str_builder_printf(classes, "is_insr ");
            if (is_ra) msu_str_builder_printf(classes, "is_ra ");
            if (is_rp) msu_str_builder_pushs(classes, "is_rp ");
            const msu_str_t *classlist = msu_str_builder_into_string_and_free(classes);

            const msu_str_t *title;
            if (idx < MIDDLE_OF_MEMORY) {
                const msu_str_t *expl = explain_insr(the_one_emulator->memory[idx]);
                title = msu_str_printf("title='%s'", msu_str_data(expl));
                msu_str_free(expl);
            } else {
                title = EMPTY_STRING;
            }

            const char *el = (
                    "<td id='memory-%d' contenteditable='true'"
                    " hx-post='emulator/memory/cell'"
                    " hx-trigger='keyup delay:500ms'"
                    " hx-vals='js:{\"value\": htmx.find(\"#memory-%d\").innerText, \"cell\": %d}'"
                    " hx-swap='none'"
                    " class='%s'"
                    " %s"
                    ">%d</td>");
            msu_str_builder_printf(memory, el, (int) idx, (int) idx, (int) idx, msu_str_data(classlist),
                                   msu_str_data(title), value);

            msu_str_free(title);
            msu_str_free(classlist);
        }
        msu_str_builder_pushs(memory, "</tr>");
    }
    msu_str_builder_pushs(memory, "</tbody></table></div>");

    msu_str_free(help);
    msu_str_free(status_line);
    return msu_str_builder_into_string_and_free(memory);
}

const msu_str_t *build_register_view() {
    typedef struct {
        const char *name;
        int value;
    } register_def_t;
    register_def_t register_names[] = {
            {"PC",  the_one_emulator->program_counter},
            {"ACC", the_one_emulator->accumulator},
            {"SP",  the_one_emulator->stack_pointer},
            {"RA",  the_one_emulator->return_address},
            {"RP",  the_one_emulator->return_stack_pointer},
    };
    const size_t register_names_count = sizeof(register_names) / sizeof(register_names[0]);

    msu_str_builder_t builder = msu_str_builder_new();
    msu_str_builder_pushs(builder, "<table id='register-table'><tbody>");
    for (size_t i = 0; i < register_names_count; i++) {
        register_def_t reg = register_names[i];

        const char *el = (
                "<tr id='reg-%s'>"
                "<td>%s</td>"
                "<td"
                " id='register-value-%s'"
                " contenteditable='true'"
                " hx-post='emulator/register/value'"
                " hx-trigger='keyup delay:500ms'"
                " hx-vals='js:{\"register\": \"%s\", \"value\": htmx.find(\"#register-value-%s\").innerText}'"
                ">%d</td>"
                "</tr>");

        msu_str_builder_printf(builder, el, reg.name, reg.name, reg.name, reg.name, reg.name, reg.value);
    }
    msu_str_builder_pushs(builder, "</table></tbody>");

    return msu_str_builder_into_string_and_free(builder);
}

void ep_assets_get(http_conn_t *conn, http_req_t req, http_error_t *errout) {
    const msu_str_t *asset_path = msu_str_slice_left(req.info.url, sizeof("/assets/") - 1);
    {
        const msu_str_t *full_path = msu_str_printf("assets/%s", msu_str_data(asset_path));
        msu_str_free(asset_path);
        asset_path = full_path;
    }

    fs_error_t fserr = FS_ERROR_NONE;
    bool is_relative = false;
    bool is_within = false;
    const msu_str_t *extension = NULL;
    strlit mime_type = {0};
    fs_stat_t stat = {0};
    const msu_str_t *content = NULL;

    fserr = fs_path_is_relative(asset_path, &is_relative);
    if (fserr) {
        internal_server_error(conn, errout);
        goto end;
    }

    if (!is_relative) {
        bad_request(conn, errout, "invalid path");
        goto end;
    }

    fserr = fs_path_within_dir(asset_path, STRLIT("./"), &is_within);
    if (fserr) {
        internal_server_error(conn, errout);
        goto end;
    }
    if (!is_within) {
        bad_request(conn, errout, "invalid path");
        goto end;
    }

    fserr = fs_path_extension(asset_path, &extension);
    if (fserr) {
        internal_server_error(conn, errout);
        goto end;
    }

    if (msu_str_eqs(extension, ".css")) {
        mime_type = STRLIT_VAL("text/css");
    } else if (msu_str_eqs(extension, ".ico")) {
        mime_type = STRLIT_VAL("image/x-icon");
    } else {
        bad_request(conn, errout, "invalid path");
        goto end;
    }

    fserr = fs_get_stat(asset_path, &stat);
    if (fserr) {
        internal_server_error(conn, errout);
        goto end;
    }
    if (!stat.is_file) {
        bad_request(conn, errout, "invalid path");
        goto end;
    }

    fserr = fs_read_to_string(asset_path, &content);
    if (fserr) {
        internal_server_error(conn, errout);
        goto end;
    }

    http_res_t *res = httpcon_make_response(conn);
    res->status_code = HTTP_STATUS_OK;
    httpheaders_set(res->headers, HH_CONTENT_TYPE, (const msu_str_t *) &mime_type);
    res->body = content;
    httpconn_send_response(conn, errout);

    end:
    msu_str_free(extension);
    msu_str_free(asset_path);
}

void ep_index_get(http_conn_t *conn, http_req_t req, http_error_t *errout) {
    http_res_t *res;
    fs_error_t fserr;

    const msu_str_t *body = NULL;
    const msu_str_t *regs = NULL;
    const msu_str_t *mem = NULL;

    (void) req;
    fserr = fs_read_to_string(STRLIT("assets/index.html.mustache"), &body);
    if (fserr) {
        internal_server_error(conn, errout);
        goto end;
    }

    regs = build_register_view();
    mem = build_memory_view();

    body = replace(body, STRLIT("{{lmsm.registers}}"), regs);
    body = replace(body, STRLIT("{{lmsm.memory}}"), mem);


    res = httpcon_make_response(conn);
    res->status_code = HTTP_STATUS_OK;
    httpheaders_set(res->headers, HH_CONTENT_TYPE, CT_HTML);
    res->body = body;
    httpconn_send_response(conn, errout);

    end:
    msu_str_free(regs);
    msu_str_free(mem);
}

void ep_favicon_get(http_conn_t *conn, http_req_t req, http_error_t *errout) {
    (void) req;
    http_res_t *res = httpcon_make_response(conn);

    const msu_str_t *content;
    fs_error_t fserr = fs_read_to_string(STRLIT("assets/LMSM.ico"), &content);
    if (fserr) {
        res->status_code = HTTP_STATUS_INTERNAL_SERVER_ERROR;
        goto end;
    }

    httpheaders_set(res->headers, HH_CONTENT_TYPE, CT_ICON);
    res->body = content;

    end:
    httpconn_send_response(conn, errout);
}

void ep_emulator_register_value_post(http_conn_t *conn, http_req_t req, http_error_t *errout) {
    kv_store_t *form_data = http_form_data_decode(req.body, errout);
    if (*errout) return bad_request(conn, errout, "invalid form data");

    const msu_str_t **register_name, **register_value_str;

    register_name = kv_store_get_one(form_data, STRLIT("register"));
    if (!register_name) {
        bad_request(conn, errout, "missing 'register' in form data");
        goto defer;
    }

    register_value_str = kv_store_get_one(form_data, STRLIT("value"));
    if (!register_value_str) {
        bad_request(conn, errout, "missing 'value' in form data");
        goto defer;
    }

    int *em_register_val;
    int min, max;
    if (msu_str_eqs(*register_name, "PC")) {
        em_register_val = &the_one_emulator->program_counter;
        min = 0;
        max = TOP_OF_MEMORY;
    } else if (msu_str_eqs(*register_name, "ACC")) {
        em_register_val = &the_one_emulator->accumulator;
        min = -999;
        max = 999;
    } else if (msu_str_eqs(*register_name, "SP")) {
        em_register_val = &the_one_emulator->stack_pointer;
        min = MIDDLE_OF_MEMORY;
        max = TOP_OF_MEMORY + 1;
    } else if (msu_str_eqs(*register_name, "RA")) {
        em_register_val = &the_one_emulator->return_address;
        min = 0;
        max = MIDDLE_OF_MEMORY;
    } else if (msu_str_eqs(*register_name, "RP")) {
        em_register_val = &the_one_emulator->return_stack_pointer;
        min = 0;
        max = TOP_OF_MEMORY;
    } else {
        bad_request(conn, errout, "unknown register");
        goto defer;
    }

    int tmp_value;
    if (msu_str_try_parse_int(*register_value_str, &tmp_value) && tmp_value >= min && tmp_value <= max) {
        *em_register_val = tmp_value;
    }

    const char *el = (
            "<div hx-swap-oob='outerHTML:#memory-table'>%s</div>"
            "<div hx-swap-oob='outerHTML:#register-table'>%s</div>"
    );

    const msu_str_t *memory_view = build_memory_view();
    const msu_str_t *reg_view = build_register_view();

    const msu_str_t *msg = msu_str_printf(el, msu_str_data(memory_view), msu_str_data(reg_view));
    reply_html(conn, errout, HTTP_STATUS_OK, msg);
    msu_str_free(memory_view);
    msu_str_free(reg_view);

    defer:
    kv_store_free(form_data);
}

void ep_emulator_memory_value_post(http_conn_t *conn, http_req_t req, http_error_t *errout) {
    kv_store_t *form_data = http_form_data_decode(req.body, errout);
    if (*errout) return bad_request(conn, errout, "invalid form data");

    const msu_str_t **valuev = kv_store_get_one(form_data, STRLIT("value"));
    if (!valuev) {
        bad_request(conn, errout, "missing 'value' in form data");
        goto defer;
    }

    const msu_str_t **cellv = kv_store_get_one(form_data, STRLIT("cell"));
    if (!cellv) {
        bad_request(conn, errout, "missing 'cell_idx' in form data");
        goto defer;
    }

    int cell_idx;
    if (!msu_str_try_parse_int(*cellv, &cell_idx) || cell_idx < 0 || cell_idx > TOP_OF_MEMORY) {
        const msu_str_t *table = build_memory_view();
        const msu_str_t *msg = msu_str_printf("<div hx-swap-oob='outerHTML:#memory-table'>%s</div>",
                                              msu_str_data(table));
        msu_str_free(table);
        reply_html(conn, errout, HTTP_STATUS_OK, msg);
        goto defer;
    }

    int value;
    if (!msu_str_try_parse_int(*valuev, &value)) {
        const msu_str_t *table = build_memory_view();
        const msu_str_t *msg = msu_str_printf("<div hx-swap-oob='outerHTML:#memory-table'>%s</div>",
                                              msu_str_data(table));
        msu_str_free(table);
        reply_html(conn, errout, HTTP_STATUS_OK, msg);
        goto defer;
    }


    the_one_emulator->memory[cell_idx] = value;

    const msu_str_t *table = build_memory_view();
    const msu_str_t *msg = msu_str_printf("<div hx-swap-oob='outerHTML:#memory-table'>%s</div>",
                                          msu_str_data(table));
    msu_str_free(table);
    reply_html(conn, errout, HTTP_STATUS_OK, msg);
    goto defer;

    defer:
    kv_store_free(form_data);
}

void ep_emulator_input_post(http_conn_t *conn, http_req_t req, http_error_t *errout) {
    kv_store_t *form_data = NULL;

    if (!wants_input) {
        reply_html(conn, errout, HTTP_STATUS_OK, EMPTY_STRING);
        goto end;
    }

    form_data = http_form_data_decode(req.body, errout);
    if (*errout) {
        bad_request(conn, errout, "invalid form data");
        goto end;
    }

    const msu_str_t **value_str = kv_store_get_one(form_data, STRLIT("value"));
    if (!value_str) {
        bad_request(conn, errout, "missing 'value' in form");
        goto end;
    }

    int value;
    if (!msu_str_try_parse_int(*value_str, &value)) {
        bad_request(conn, errout, "invalid 'value': could not parse");
        goto end;
    }

    if (value < -999 || value > 999) {
        bad_request(conn, errout, "invalid 'valid', not in range (-999, 999)");
        goto end;
    }

    wants_input = false;

    sprintf(input_buffer, "%d ", value);
    const msu_str_t *res = msu_str_printf("<div hx-swap-oob='innerHTML:#register-value-ACC'>%d</div>", value);
    reply_html(conn, errout, HTTP_STATUS_OK, res);

    end:
    kv_store_free(form_data);
}

void bad_request(http_conn_t *conn, http_error_t *errout, const char *msg) {
    fprintf(stdout, "ERROR: reply w/ BAD REQUEST: %s\n", msg);
    http_res_t *res = httpcon_make_response(conn);
    res->status_code = HTTP_STATUS_BAD_REQUEST;
    if (msg) {
        res->body = msu_str_new(msg);
    }
    httpconn_send_response(conn, errout);
}

void internal_server_error(http_conn_t *conn, http_error_t *errout) {
    fprintf(stdout, "ERROR: reply w/ INTERNAL SERVER ERROR\n");
    http_res_t *res = httpcon_make_response(conn);
    res->status_code = HTTP_STATUS_INTERNAL_SERVER_ERROR;
    res->body = NULL;
    httpconn_send_response(conn, errout);
}

void ok(http_conn_t *conn, http_error_t *errout) {
    http_res_t *res = httpcon_make_response(conn);
    res->status_code = HTTP_STATUS_OK;
    httpconn_send_response(conn, errout);
}

void reply_html(http_conn_t *conn, http_error_t *errout, http_status_t status, const msu_str_t *body) {
    http_res_t *res = httpcon_make_response(conn);
    res->status_code = status;
    res->body = body;
    httpheaders_set(res->headers, HH_CONTENT_TYPE, CT_HTML);
    httpconn_send_response(conn, errout);
}

void ep_emulator_code_post(http_conn_t *conn, http_req_t req, http_error_t *errout) {
    kv_store_t *form_data = NULL;

    form_data = http_form_data_decode(req.body, errout);
    if (*errout) {
        bad_request(conn, errout, "invalid form data");
        goto end;
    }

    const msu_str_t **srcv = kv_store_get_one(form_data, STRLIT("source"));
    if (!srcv) {
        bad_request(conn, errout, "missing 'source'");
        goto end;
    }
    const msu_str_t *src = *srcv;

    const msu_str_t **actionv = kv_store_get_one(form_data, STRLIT("action"));
    if (!actionv) {
        bad_request(conn, errout, "missing 'action'");
        goto end;
    }
    const msu_str_t *action = *actionv;

    const msu_str_t **langv = kv_store_get_one(form_data, STRLIT("language"));
    if (!langv) {
        bad_request(conn, errout, "missing 'language'");
        goto end;
    }
    const msu_str_t *lang = *langv;

    lang_compile_fn func;
    if (msu_str_eqs(lang, "sea")) {
        func = compile_sea;
    } else if (msu_str_eqs(lang, "firth")) {
        func = compile_firth;
    } else if (msu_str_eqs(lang, "zortran")) {
        func = compile_zortran;
    } else if (msu_str_eqs(lang, "lma")) {
        func = compile_asm;
    } else {
        bad_request(conn, errout, "unknown language");
        goto end;
    }

    if (msu_str_eqs(action, "Compile")) {
        if (!func(conn, errout, src, false)) {
            goto end;
        }
    } else if (msu_str_eqs(action, "Compile + Load")) {
        if (!func(conn, errout, src, true)) {
            goto end;
        }

        wants_input = the_one_emulator->memory[0] == 901;
    } else {
        bad_request(conn, errout, "invalid action");
        goto end;
    }

end:
    kv_store_free(form_data);
}

void ep_emulator_step_post(http_conn_t *conn, http_req_t req, http_error_t *errout) {
    if (the_one_emulator->status == STATUS_HALTED) return;
    if (wants_input) return;

    emulator_step(the_one_emulator);


    const char *el = (
            "<div hx-swap-oob='outerHTML:#memory-table'>%s</div>"
            "<div hx-swap-oob='outerHTML:#register-table'>%s</div>"
            "<div hx-swap-oob='innerHTML:#emulator-output'>%s</div>"
    );

    const msu_str_t *memory_view = build_memory_view();
    const msu_str_t *reg_view = build_register_view();

    const msu_str_t *msg = msu_str_printf(el, msu_str_data(memory_view), msu_str_data(reg_view),
                                          the_one_emulator->output_buffer);
    reply_html(conn, errout, HTTP_STATUS_OK, msg);
    msu_str_free(memory_view);
    msu_str_free(reg_view);

    wants_input = the_one_emulator->memory[the_one_emulator->program_counter] == 901;
}

void ep_emulator_restart_post(http_conn_t *conn, http_req_t req, http_error_t *errout) {
    the_one_emulator->program_counter = 0;
    the_one_emulator->accumulator = 0;
    the_one_emulator->stack_pointer = TOP_OF_MEMORY + 1;
    the_one_emulator->return_stack_pointer = MIDDLE_OF_MEMORY;
    the_one_emulator->return_address = 0;
    memset(the_one_emulator->output_buffer, 0, sizeof(the_one_emulator->output_buffer));
    the_one_emulator->input_buffer = input_buffer;
    input_buffer[0] = 0;
    the_one_emulator->status = STATUS_READY;

    const char *el = (
            "<div hx-swap-oob='outerHTML:#memory-table'>%s</div>"
            "<div hx-swap-oob='outerHTML:#register-table'>%s</div>"
            "<div hx-swap-oob='innerHTML:#emulator-output'></div>"
    );

    const msu_str_t *memory_view = build_memory_view();
    const msu_str_t *reg_view = build_register_view();

    const msu_str_t *msg = msu_str_printf(el, msu_str_data(memory_view), msu_str_data(reg_view));
    reply_html(conn, errout, HTTP_STATUS_OK, msg);
    msu_str_free(memory_view);
    msu_str_free(reg_view);
}

void ep_emulator_reset_post(http_conn_t *conn, http_req_t req, http_error_t *errout) {
    (void) req;

    emulator_reset(the_one_emulator);
    the_one_emulator->input_buffer = input_buffer;
    input_buffer[0] = 0;

    http_res_t *res = httpcon_make_response(conn);
    res->status_code = HTTP_STATUS_OK;
    httpheaders_set(res->headers, STRLIT("HX-Refresh"), STRLIT("true"));
    res->body = EMPTY_STRING;
    httpconn_send_response(conn, errout);
}

void ep_help_get(http_conn_t *conn, http_req_t req, http_error_t *errout) {
    (void) req;
    const msu_str_t *content;
    fs_error_t fs_err = FS_ERROR_NONE;

    http_res_t *res = httpcon_make_response(conn);

    fs_err = fs_read_to_string(STRLIT("assets/help.html"), &content);
    if (fs_err) {
        res->status_code = HTTP_STATUS_INTERNAL_SERVER_ERROR;
        goto end;
    }

    httpheaders_set(res->headers, HH_CONTENT_TYPE, CT_HTML);
    res->body = content;

end:
    httpconn_send_response(conn, errout);
}

const msu_str_t *explain_insr(int insr) {
    if (insr >= -500 && insr <= -401) {
        int value = -401 - insr;
        return msu_str_printf("SSTA %03d - Stack Store: mem[$sp+%d+1] = mem[$sp], $sp--", value, value);
    } else if (insr >= -300 && insr <= -201) {
        int value = -201 - insr;
        return msu_str_printf("SLDA %03d - Stack Load: mem[$sp-1]=mem[$sp+d], $sp--", value, value);
    } else if (insr >= -200 && insr <= -101) {
        int value = -101 - insr;
        return msu_str_printf("SPSUB %03d - SP Sub: $sp -= 1 + %d", value, value);
    } else if (insr >= -100 && insr <= -001) {
        int value = -001 - insr;
        return msu_str_printf("SPADD %03d - SP Add: $sp += 1 + %d", value, value);
    } else if (insr == 0) {
        return msu_str_new("HLT/COB - Halt the machine");
    } else if (insr >= 100 && insr <= 199) {
        int value = insr - 100;
        return msu_str_printf("ADD %03d - Add: $acc += mem[%d]", value, value);
    } else if (insr >= 200 && insr <= 299) {
        int value = insr - 200;
        return msu_str_printf("SUB %03d - Sub: $acc -= mem[%d]", value, value);
    } else if (insr >= 300 && insr <= 399) {
        int value = insr - 300;
        return msu_str_printf("STA %03d - Store: mem[%d] = $acc", value, value);
    } else if (insr >= 400 && insr <= 499) {
        int value = insr - 400;
        return msu_str_printf("LDI %03d - Load Immediate: $acc = %d", value, value);
    } else if (insr >= 500 && insr <= 599) {
        int value = insr - 500;
        return msu_str_printf("LDA %03d - Load: acc = $mem[%d]", value, value);
    } else if (insr >= 600 && insr <= 699) {
        int value = insr - 600;
        return msu_str_printf("BRA %03d - Branch Always: jump to %d", value, value);
    } else if (insr >= 700 && insr <= 799) {
        int value = insr - 700;
        return msu_str_printf("BRZ %03d - Branch If Zero: if $acc == 0, jump to %d", value, value);
    } else if (insr >= 800 && insr <= 899) {
        int value = insr - 800;
        return msu_str_printf("BRP %03d - Branch If Positive: if $acc >= 0, jump to %d", value, value);
    } else if (insr == 901) {
        return msu_str_new("INP - Input: $acc = input()");
    } else if (insr == 902) {
        return msu_str_new("OUT - Output: print($acc)");
    } else if (insr == 910) {
        return msu_str_new("JAL - Jump And Link: $ra = $pc, $pc = $acc");
    } else if (insr == 911) {
        return msu_str_new("RET - Return: $pc = $ra");
    } else if (insr == 920) {
        return msu_str_new("SPUSH - Stack Push: mem[$sp] = $acc, $sp--");
    } else if (insr == 921) {
        return msu_str_new("SPOP - Stack Pop: $acc = mem[$sp], $sp++");
    } else if (insr == 922) {
        return msu_str_new("SDUP - Stack Duplicate: mem[$sp-1] = mem[$sp], $sp--");
    } else if (insr == 923) {
        return msu_str_new("SDROP - Stack Drop: $sp += 1");
    } else if (insr == 924) {
        return msu_str_new("SSWAP - Stack Swap: mem[$sp] <> mem[$sp+1]");
    } else if (insr == 925) {
        return msu_str_new("RPUSH - Return Stack Push: mem[$rp] = $acc, rp++");
    } else if (insr == 926) {
        return msu_str_new("RPOP - Return Stack Pop: $acc = mem[$rp], $rp--");
    } else if (insr == 930) {
        return msu_str_new("SADD - Stack Add: mem[$sp+1]=mem[$sp]+mem[$sp+1], $sp++");
    } else if (insr == 931) {
        return msu_str_new("SSUB - Stack Subtract: mem[$sp+1]=mem[$sp]-mem[$sp+1], $sp++");
    } else if (insr == 932) {
        return msu_str_new("SMUL - Stack Multiply: mem[$sp+1]=mem[$sp]*mem[$sp+1], $sp++");
    } else if (insr == 933) {
        return msu_str_new("SDIV - Stack Divide: mem[$sp+1]=mem[$sp]*mem[$sp+1], $sp++");
    } else if (insr == 934) {
        return msu_str_new("SMAX - Stack Max: mem[$sp+1]=max(mem[$sp], mem[$sp+1]), $sp++");
    } else if (insr == 935) {
        return msu_str_new("SMIN - Stack Min: mem[$sp+1]=min(mem[$sp], mem[$sp+1]), $sp++");
    } else if (insr == 936) {
        return msu_str_new("SCMPGT - Stack Compare Greater: mem[$sp+1]=mem[$sp]>mem[$sp+1], $sp++");
    } else if (insr == 937) {
        return msu_str_new("SCMPLT - Stack Compare Less:  mem[$sp+1]=mem[$sp]<mem[$sp+1], $sp++");
    } else if (insr == 938) {
        return msu_str_new("SNOT - Stack Not: mem[$sp] = mem[$sp]==0");
    } else {
        return msu_str_new("unknown instruction");
    }
}
