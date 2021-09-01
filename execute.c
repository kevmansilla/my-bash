#include "command.h"
#include "builtin.h"
#include "execute.h"
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <sys/wait.h>

void execute_pipeline(pipeline apipe){
    assert(apipe != NULL);

    /* Si el pipeline es vacio */
    if(pipeline_is_empty(apipe)){
        return;
    }

    /* Si es un comando interno */
    if(builtin_is_internal(apipe)){
        builtin_exec(apipe);
        return;
    }

    /* Si solamente es un comando */
    /* no hace falta usar pipe */
    if(pipeline_length(apipe) == 1){
        pid_t pid = fork();

        if(pid == 0){   /* El proceso es hijo */
            /* execvp(); */
        }else if(pid == -1){
            printf("Error con el fork");
        }else{ /* El proceso es padre */
            waitpid(pid, NULL, 0);
        }

        return;
    }
}
