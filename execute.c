#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <sys/wait.h>

#include "builtin.h"
#include "execute.h"
#include "tests/syscall_mock.h"

static void config_redir(pipeline apipe){
    if(apipe != NULL){
        scommand pipe_front = pipeline_front(apipe);
        char *redir_in = scommand_get_redir_in(pipe_front);
        if(redir_in){
            int in = open(redir_in, O_RDONLY, S_IRWXU);
            dup2(in, STDIN_FILENO);
            close(in);
        }
        char *redir_out = scommand_get_redir_out(pipe_front);
        if(redir_out){
            int out = open(redir_out,  O_CREAT | O_WRONLY | O_TRUNC , S_IRWXU);
            dup2(out, STDOUT_FILENO);
            close(out);
        }
    }
}

static int execute_simple_scommand(pipeline apipe){
    int status_execute = 0;
    if(builtin_is_internal(apipe)){
        builtin_exec(apipe);
    }
    else{
        scommand pipe_front = pipeline_front(apipe);
        unsigned int length_command = scommand_length(pipe_front);
        char **cmd = calloc(length_command + 1, sizeof(char *));
        for(unsigned int i = 0u; i < length_command; ++i){
            char *tmp = scommand_front(pipe_front);     
            cmd[i] = strdup(tmp);                       
            scommand_pop_front(pipe_front);             
        }
        if(execvp(cmd[0], cmd) == -1){
            fprintf(stderr, "Comando inexistente\n");
            status_execute = -1;
        }
        for(unsigned int i = 0u; i < length_command; ++i){
            free(cmd[i]);
            cmd[i] = NULL;
        }
        free(cmd);
        cmd = NULL;
    }
    return status_execute;
}

void execute_pipeline(pipeline apipe){
    assert(apipe != NULL);
    bool pipe_wait = pipeline_get_wait(apipe);

    if(pipeline_is_empty(apipe)){
        return;
    }
    if(builtin_is_internal(apipe)){
        builtin_exec(apipe);
        return;
    }
    if(pipeline_length(apipe) == 1){
        pid_t pid = fork();
        if(pid == 0){ 
            config_redir(apipe);
            if(execute_simple_scommand(apipe) == -1){
                exit(1);
            }
        }
        else if(pid == -1){
            fprintf(stderr, "Error con el fork\n");
        }
        else{
            if(pipe_wait){
                waitpid(pid, NULL, 0);
            }
        }
        return;
    } 
    else {
        unsigned int len_pipeline = pipeline_length(apipe);
        int **vec_pipe = calloc(len_pipeline - 1, sizeof(int*));
        for(unsigned int i = 0u; i < len_pipeline - 1; ++i){
            vec_pipe[i] = calloc(2, sizeof(int));
            pipe(vec_pipe[i]); 
        }
        unsigned int *copy_fork = calloc(len_pipeline, sizeof(int));
        for(unsigned int i = 0u; i < len_pipeline; ++i){
            pid_t pid = fork();
            if (pid == 0){
                if(i < len_pipeline -1){ 
                    close(vec_pipe[i][0]); 
                    dup2(vec_pipe[i][1],STDOUT_FILENO);
                    close(vec_pipe[i][1]);
                }
                if(i > 0){ 
                    close(vec_pipe[i-1][1]); 
                    dup2(vec_pipe[i-1][0],STDIN_FILENO);
                    close(vec_pipe[i-1][0]); 
                }

                config_redir(apipe);
                if(execute_simple_scommand(apipe) == -1){
                    exit(1);
                }
            }
            else if(pid == -1){
                fprintf(stderr, "Error con el fork\n");
            } 
            else {
                if (i > 0){
                    close(vec_pipe[i-1][0]);
                }
                if(i < len_pipeline -1){
                    close(vec_pipe[i][1]);
                }
                copy_fork[i] = pid;
                pipeline_pop_front(apipe);
            }
        }
        if (pipe_wait){
            for (unsigned int i = 0u; i < len_pipeline; ++i){
                waitpid(copy_fork[i], NULL, 0);
            }
        }
        free(copy_fork);
        for (unsigned int i = 0u; i < len_pipeline - 1; ++i){
            free(vec_pipe[i]);
            vec_pipe[i] = NULL;
        }
        free(vec_pipe);
        vec_pipe = NULL;
    }
}
