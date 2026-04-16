#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <stdarg.h>

#include "list.h"
#include "base.h"

#include <stddef.h>

namespace x_list {

void print_centered(FILE * fp, const char *str, uint32_t field_width) {
    uint32_t str_len = (uint32_t) strlen(str);
    if (str_len >= field_width) {
        fprintf(fp, "%s", str); // If string is too long, print as-is
        return;
    }

    uint32_t left_padding = (field_width - str_len) / 2;
    uint32_t right_padding = field_width - str_len - left_padding;

    fprintf(fp, "%*s%s%*s", left_padding, "", str, right_padding, "");
}

int snprint_centered(char * dst, size_t max_size, const char *str, uint32_t field_width) {
    uint32_t str_len = (uint32_t) strlen(str);
    size_t written = 0;
    if (str_len >= field_width) {
        written += snprintf(dst, max_size, "%s", str); // If string is too long, print as-is
        return written;
    }

    uint32_t left_padding = (field_width - str_len) / 2;
    uint32_t right_padding = field_width - str_len - left_padding;

    written += snprintf(dst + written, max_size - written, "%*s%s%*s", left_padding, "", str, right_padding, "");
    return written;
}

void _generate_dot_dump(list_t *list, FILE * fp) {
    verifier(list);
    fprintf(fp,
        "digraph DoublyLinkedList {\n"
        "\t// Настройки графа\n"
        "\trankdir=\"LR\";\n");
    fprintf(fp, "\tranksep=0.0\n"
        "\tnodesep=0.08\n"
        "\n"
        "\tnode [shape=record, height=0.5, color = \"red\", fontcolor = \"red\", style = \"filled\", fillcolor = \"#ffc5c5\"];\n"
        "\tedge [arrowsize=0.8, minlen=4];\n"
        "\n");

    list_element_t *elements = list->elements;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat"
    fprintf(fp, "\tnode0 [label=\"<index> index = 0 | <data> data = PSN | { <prev> prev = %lld | <next> next = %zu}\", color = \"black\", fontcolor = \"black\", fillcolor = \"#adeda7ff\"];\n", elements[0].prev, elements[0].next);

    for (size_t i = 1; i < list->capacity; ++i) {
        if (elements[i].prev == SIZE_T_MAX) { // Это free элемент
            // printf("free node%zu\n", i);
            fprintf(fp, "\tnode%zu [label=\"<index> index = %zu | <data> data = empty | { <prev> prev = %lld | <next> next = %zu}\", color = \"#f67c92ff\", fontcolor = \"black\", fillcolor = \"#d0ceceff\"];\n", // color = \"#ff9500ff\", fontcolor = \"black\", fillcolor = \"#ffd997\"
                i, i, elements[i].prev, elements[i].next);
        } else { // Это занятый элемент
            // printf("node%zu\n", i);
            fprintf(fp, "\tnode%zu [label=\"<index> index = %zu | <data> data = %d | { <prev> prev = %lld | <next> next = %zu}\", color = \"#ff9500ff\", fontcolor = \"black\", fillcolor = \"#ffd997\"];\n",
                        i, i, elements[i].data, elements[i].prev, elements[i].next);
        }
    }
#pragma clang diagnostic pop

    fprintf(fp,
        "\n"
        "\t// Выравнивание\n"
    );

    for (size_t i = 1; i < list ->capacity; ++i) {
        fprintf(fp, "\tnode%zu -> node%zu [weight=100, color = \"#ffffff\"];\n", i-1, i);
    }

    fprintf(fp, "\n\t// Связи next\n");

    size_t el1 = 0, el2 = get_next_element(list, el1);

    for (size_t i = 0; i < list->size; ++i) {
        // printf("MEOW\n");
        // printf("i = %lld\n", i);
        if (el1 == get_prev_element(list, el2)) {
            fprintf(fp, "\tnode%zu -> node%zu [color = \"#000000\", constraint=false, dir=both];\n", el1, el2);
        } else {
            // --i;
            fprintf(fp, "\tnode%zu -> node%zu [color = \"#cc0c0cff\", constraint=false];\n", el1, el2);
            fprintf(fp, "\tnode%zu -> node%zu [color = \"#3dad3d\", constraint=false];\n", get_prev_element(list, el2), el1);
        }
        el1 = el2;
        el2 = get_next_element(list, el2);
    }
    // }

    fprintf(fp, "\n\t// Связи free\n");

    el1 = list->free_idx;
    for (size_t i = 0; i < list->capacity - list->size; ++i) {
        el2 = get_next_element(list, el1);
        if (get_prev_element(list, el2) == SIZE_T_MAX || el2 == 0) {
            fprintf(fp, "\tnode%zu -> node%zu [color = \"#f67c92ff\", constraint=false];\n", el1, el2);
        } else { // Попали в занятый элемент
            fprintf(fp, "\tnode%zu -> node%zu [color = \"#cc0c0cff\", constraint=false];\n", el1, el2);
            break; // Если пойдем дальше, то пойдем по занятым элементам, а это не то, что должен печатать этот цикл
        }
        el1 = el2;
    }
    fprintf(fp, "}");
}

const char *_generate_image(list_t *list, const char *dir_name) {
    local size_t iter;

    char tmp_dot_filename[128] = "";
    if (dir_name[strlen(dir_name) - 1] == '/')
        snprintf(tmp_dot_filename, 128, "%slist_dump_%zu.dot.tmp", dir_name, iter);
    else
        snprintf(tmp_dot_filename, 128, "%s/list_dump_%zu.dot.tmp", dir_name, iter);

    FILE *tmp_dot_file = fopen(tmp_dot_filename, "w");
    _generate_dot_dump(list, tmp_dot_file);
    fclose(tmp_dot_file);

    local char image_filename[128] = "";
    char image_path[128] = "";

    snprintf(image_filename, 128, "image%zu.svg", iter++);

    if (dir_name[strlen(dir_name) - 1] == '/')
        snprintf(image_path, 128, "%s%s", dir_name, image_filename);
    else
        snprintf(image_path, 128, "%s/%s", dir_name, image_filename);

    char command[128] = "";
    snprintf(command, 128, "dot -Tsvg %s -o %s", tmp_dot_filename, image_path);
    system(command);

    return image_filename;
}

void _dump_impl(list_t *list, FILE *logfile, const char *log_dirname,
        int line, const char *func, const char *file,
        const char *fmt, ...) {
    verifier(list);
    // fprintf(logfile, "<h3>-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- "
        // "LIST DUMP"" -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-</h3>\n");

    // format prompt (printf-like)
    char prompt_buf[2048] = "";
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(prompt_buf, sizeof(prompt_buf), fmt, ap);
    va_end(ap);

    fprintf(logfile, "<h2>reason: %s</h2>\n", prompt_buf);
    fprintf(logfile, "dump from %s:%d at %s\n", file, line, func);
    fprintf(logfile, "\n");
    fprintf(logfile, "list struct at <font style=\"color :#03CFCF;\">%p</font>\n", list);
    fprintf(logfile, "size is %zu\t capacity is %zu\n", list->size, list->capacity);
    fprintf(logfile, "front is %zu\t back is %zu\n", list->elements[0].next, list->elements[0].prev);
    fprintf(logfile, "free_idx is %zu\n", list->free_idx);
    if (list->errno != LIST_NO_PROBLEM) {
        fprintf(logfile, "<font style=\"color :red;\">LIST in invalid state</font>, errno is %s\n", error(list));
    }
    fprintf(logfile, "elements at <font style=\"color :#03CFCF;\">%p</font>\n\n", list->elements);

    list_element_t *elements = list->elements;

    const size_t DLINA_CTPOKI = 128;
    char CTPOKA[DLINA_CTPOKI] = "";
    print_centered(logfile, "", 6);
    fprintf(logfile, "|");
    for (size_t i = 0; i < list->capacity; ++i) {
        snprintf(CTPOKA, sizeof(CTPOKA), "[%zu]", i);
        print_centered(logfile, CTPOKA, 9);
        fprintf(logfile, "|");
    }
    fprintf(logfile, "\n");

    size_t free_idx = list->free_idx;
    fprintf(logfile, "data: |");
    for (size_t i = 0; i < list->capacity; ++i) {
        if (i == 0)
            snprintf(CTPOKA, sizeof(CTPOKA), "PSN");
        else if (i == free_idx) {
            snprintf(CTPOKA, sizeof(CTPOKA), "empty");
            free_idx = elements[free_idx].next;
        }
        else
            snprintf(CTPOKA, sizeof(CTPOKA), "%d", elements[i].data);

        // fprintf(logfile, " [%zu] ", i);
        print_centered(logfile, CTPOKA, 9);
        fprintf(logfile, "|");
    }
    fprintf(logfile, "\n");

    fprintf(logfile, "next: |");
    for (size_t i = 0; i < list->capacity; ++i) {
        snprintf(CTPOKA, sizeof(CTPOKA), "%zu", elements[i].next);

        // fprintf(logfile, " [%zu] ", i);
        print_centered(logfile, CTPOKA, 9);
        fprintf(logfile, "|");
    }
    fprintf(logfile, "\n");

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat"
    fprintf(logfile, "prev: |");
    for (size_t i = 0; i < list->capacity; ++i) {
        snprintf(CTPOKA, sizeof(CTPOKA), "%lld", elements[i].prev);

        // fprintf(logfile, " [%zu] ", i);
        print_centered(logfile, CTPOKA, 9);
        fprintf(logfile, "|");
    }
    fprintf(logfile, "\n\n");
#pragma clang diagnostic pop

    const char * image_filename = _generate_image(list, log_dirname);

    fprintf(logfile, "<img src=\"%s\">\n", image_filename);
}

char *_dump_to_str(list_t *list,
        int line, const char *func, const char *file,
        const char *fmt, ...) {
    verifier(list);
    // fprintf(logfile, "<h3>-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- "
        // "LIST DUMP"" -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-</h3>\n");

    // format prompt (printf-like)
    char prompt_buf[2048] = "";
    if (fmt != NULL) {
        va_list ap;
        va_start(ap, fmt);
        vsnprintf(prompt_buf, sizeof(prompt_buf), fmt, ap);
        va_end(ap);
    }

    const size_t max_size = 4096;
    size_t size = max_size, written = 0;
    char *str = (char *) calloc(max_size, sizeof(char));

    if (fmt != NULL)
        written += snprintf(str + written, max_size - written, "reason: %s\n", prompt_buf);
    written += snprintf(str + written, max_size - written, "dump from %s:%d at %s\n", file, line, func);
    written += snprintf(str + written, max_size - written, "\n");
    written += snprintf(str + written, max_size - written, "list struct at %p\n", list);
    written += snprintf(str + written, max_size - written, "size is %zu\t capacity is %zu\n", list->size, list->capacity);
    written += snprintf(str + written, max_size - written, "front is %zu\t back is %zu\n", list->elements[0].next, list->elements[0].prev);
    written += snprintf(str + written, max_size - written, "free_idx is %zu\n", list->free_idx);
    if (list->errno != LIST_NO_PROBLEM) {
        written += snprintf(str + written, max_size - written, "LIST in invalid state, errno is %s\n", error(list));
    }
    written += snprintf(str + written, max_size - written, "elements at >%p\n\n", list->elements);

    list_element_t *elements = list->elements;

    const size_t DLINA_CTPOKI = 128;
    char CTPOKA[DLINA_CTPOKI] = "";
    written += snprint_centered(str + written, max_size - written, "", 6);
    written += snprintf(str + written, max_size - written, "|");
    for (size_t i = 0; i < list->capacity; ++i) {
        snprintf(CTPOKA, sizeof(CTPOKA), "[%zu]", i);
        written += snprint_centered(str + written, max_size - written, CTPOKA, 9);
        written += snprintf(str + written, max_size - written, "|");
    }
    written += snprintf(str + written, max_size - written, "\n");

    size_t free_idx = list->free_idx;
    written += snprintf(str + written, max_size - written, "data: |");
    for (size_t i = 0; i < list->capacity; ++i) {
        if (i == 0)
            snprintf(CTPOKA, sizeof(CTPOKA), "PSN");
        else if (i == free_idx) {
            snprintf(CTPOKA, sizeof(CTPOKA), "empty");
            free_idx = elements[free_idx].next;
        }
        else
            snprintf(CTPOKA, sizeof(CTPOKA), "%d", elements[i].data);

        // fprintf(logfile, " [%zu] ", i);
        written += snprint_centered(str + written, max_size - written, CTPOKA, 9);
        written += snprintf(str + written, max_size - written, "|");
    }
    written += snprintf(str + written, max_size - written, "\n");

    written += snprintf(str + written, max_size - written, "next: |");
    for (size_t i = 0; i < list->capacity; ++i) {
        snprintf(CTPOKA, sizeof(CTPOKA), "%zu", elements[i].next);

        // fprintf(logfile, " [%zu] ", i);
        written += snprint_centered(str + written, max_size - written, CTPOKA, 9);
        written += snprintf(str + written, max_size - written, "|");
    }
    written += snprintf(str + written, max_size - written, "\n");

    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wformat"
    written += snprintf(str + written, max_size - written, "prev: |");
    for (size_t i = 0; i < list->capacity; ++i) {
        snprintf(CTPOKA, sizeof(CTPOKA), "%lld", elements[i].prev);

        // fprintf(logfile, " [%zu] ", i);
        written += snprint_centered(str + written, max_size - written, CTPOKA, 9);
        written += snprintf(str + written, max_size - written, "|");
    }
    written += snprintf(str + written, max_size - written, "\n\n");
    #pragma clang diagnostic pop

    return str;
}

}