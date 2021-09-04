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
        char *redir_in = scommand_get_redir_in(pipe_front); // <
        if(redir_in){
            //int open(const char *pathname, int flags, mode_t mode);
            //O_RDONLY la persona que llama tiene permiso de lectura sobre el objeto
            int in = open(redir_in, O_RDONLY, S_IRWXU);
            //S_IRUSR: Leer bit
            //S_IWUSR: escribit bit
            //S_IXUSR: ejecutar bit
            dup2(in, STDIN_FILENO); //duplica descriptores de archivos y apunta al archivo (in.txt) (dump no apunta)
            close(in);
        }
        char *redir_out = scommand_get_redir_out(pipe_front); // > mando a un archivo
        if(redir_out){
            int out = open(redir_out,  O_CREAT | O_WRONLY | O_TRUNC , S_IRWXU);
            //O_CREAT: sino existe se crea
            //O_WRONLY: para escritura
            //S_IRWXU: permisos de lectura, escritura y ejecucion
            //S_IRWXU: el usuario (el propietario del fichero) tiene permisos de lectura, escritura y ejecución
            dup2(out, STDOUT_FILENO);
            close(out);
        }
    }
}

static void execute_simple_scommand(pipeline apipe){
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
        }
        for(unsigned int i = 0u; i < length_command; ++i){
            free(cmd[i]);
            cmd[i] = NULL;
        }
        free(cmd);
        cmd = NULL;
    }
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
            execute_simple_scommand(apipe);
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
        int **vec_pipe = calloc(1, sizeof(int*));
        for(unsigned int i = 0u; i < 1; ++i){
            vec_pipe[i] = calloc(2, sizeof(int));
            pipe(vec_pipe[i]); 
        }
        unsigned int *copy_fork = calloc(2, sizeof(int));
        for(unsigned int i = 0u; i < 2; ++i){
            pid_t pid = fork();
            if (pid == 0){
                if(i < 1){ 
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
                execute_simple_scommand(apipe);
            }
            else if(pid == -1){
                fprintf(stderr, "Error con el fork\n");
            } 
            else {
                if (i > 0){
                    close(vec_pipe[i-1][0]);
                }
                if(i < 1){
                    close(vec_pipe[i][1]);
                }
                copy_fork[i] = pid;
                pipeline_pop_front(apipe);
            }
        }
        if (pipe_wait){
            for (unsigned int i = 0u; i < 2; ++i){
                waitpid(copy_fork[i], NULL, 0);
            }
        }
        free(copy_fork);
        for (unsigned int i = 0u; i < 1; ++i){
            free(vec_pipe[i]);
            vec_pipe[i] = NULL;
        }
        free(vec_pipe);
        vec_pipe = NULL;
    }
}
