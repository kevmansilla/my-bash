#include <stdio.h>
#include <stdbool.h>

#include "command.h"
#include "execute.h"
#include "parser.h"
#include "builtin.h"
#include "prompt.h"


int main(int argc, char *argv[]){
    scommand new = scommand_new();
    scommand_push_back(new, "ls -l");

    pipeline new_p = pipeline_new();
    pipeline_push_back(new_p, new);

    printf("%d\n", builtin_is_cd(new_p));

    /* parser = parser_new(stdin);
    while (!quit) {
        show_prompt();
        pipe = parse_pipeline(parser);
        quit = parser_at_eof(parser); 

        if (pipe != NULL) {
            quit = quit || builtin_is_exit(pipe);
            execute_pipeline(pipe);
            pipeline_destroy(pipe);
        } else if (!quit) {
            printf("Comando no valido\n");
        }
    }
    parser_destroy(parser);
    parser = NULL; */
    return 0;
}
