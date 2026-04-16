#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "base.h"
#include "list.h"
#include "io_utils.h"

using namespace x_list;

global const time_t _start_time = time(NULL) - (time_t)(clock() / CLOCKS_PER_SEC);

const char *create_dump_directory() {
    create_folder_if_not_exists("logs/");
    local char buf[128] = "";
    char start_time_string[32] = "";
    strftime(start_time_string, 32, "%FT%TZ", gmtime(&_start_time));
    snprintf(buf, 128, "logs/log%s", start_time_string);
    create_folder_if_not_exists(buf);

    return buf;
}

#define DO(code)                                                                                          \
    dump(list, logfile, dump_dirname, "Dump before doing <font style=\"color: blue;\">" #code "</font>"); \
    code                                                                                                  \
    dump(list, logfile, dump_dirname, "Dump after doing <font style=\"color: blue;\">" #code "</font>");  \
    fprintf(logfile, "<hr>\n\n\n");

int main() {
    printf("Hello, kitty!\n");
    const char * dump_dirname = create_dump_directory();

    char log_filename[128] = "";
    char start_time_string[32] = "";
    strftime(start_time_string, 32, "%T %F", gmtime(&_start_time));
    snprintf(log_filename, 128, "%s/log.html", dump_dirname);

    FILE *logfile = fopen(log_filename, "a");
    fprintf(logfile,
        "<html>\n"
        "<head>\n"
        "\t<title>%s</title>\n"
        "</head>\n"
        "<body>\n"
        "<pre>\n", start_time_string);

    x_list::list_t *list = x_list::constructor(11);

    DO(size_t idx1 = push_front(list, 5);)

    DO(push_front(list, 4);)
    char *str = dump_to_str(list, 0);
    printf("%s", str);
    free(str);
    DO(push_front(list, 3);)

    // DO(list->elements[1].next = 690;) // LIST_INVALID_INDEX - цепочка next не приводит к концу списка

    // DO(list->elements[1].next = 3;) // LIST_LOOP_IN_NEXT

    // DO(list->elements[5].next = 3;) // LIST_FREE_LIST_CORRUPT - занятый элемент в free-list

    // DO(list->elements[5].next = 4;) // LIST_LOOP_IN_FREE - цикл 5 -> 4; 4 -> 5

    // DO(list->size = 2;) // LIST_SIZE_MISMATCH

    DO(push_front(list, 3);)

    dump(list, logfile, dump_dirname, "Dump before insert 52 after %zu\n", idx1);
    insert(list, idx1, 52);
    dump(list, logfile, dump_dirname, "Dump after insert 52 after %zu\n", idx1);
    fprintf(logfile, "<hr>\n\n\n");


    dump(list, logfile, dump_dirname, "Dump before emplace 1000-7 before %zu\n", idx1);
    emplace(list, idx1, 1000-7);
    dump(list, logfile, dump_dirname, "Dump after emplace 1000-7 before %zu\n", idx1);
    fprintf(logfile, "<hr>\n\n\n");

    DO(erase(list, 2);)

    DO(push_back(list, 1);)

    for (int i = 3; i < 7; ++i) {
        DO(push_front(list, i*10);)
    }

    DO(pop_back(list);)
    DO(pop_front(list);)
    DO(erase(list, 4);)
    DO(erase(list, 6);)

    destructor(&list);

    fprintf(logfile, "</pre>\n</body>\n</html>");
    fclose(logfile);

    char command[128] = "";
    snprintf(command, sizeof(command), "open %s", log_filename);
    system(command);
    return 0;
}