#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h> //open()
#include <assert.h>
#include <unistd.h> //api dup
#include <sys/wait.h> //wait

#include "builtin.h"
#include "execute.h"
#include "tests/syscall_mock.h"

static void execute_simple_scommand(pipeline apipe){
    if(builtin_is_internal(apipe)){
        builtin_exec(apipe); // Ejecuto el comando interno.
    }else{
        // Traigo el comando simple de adelante de la secuencia.
        scommand pipe_front = pipeline_front(apipe);
        unsigned int length_command = scommand_length(pipe_front);
        char **cmd = calloc(length_command + 1, sizeof(char *)); // Array (puntero a *char), pido memoria con tamaño del comando + 1.
        for(unsigned int i = 0u; i < length_command; ++i){
            char *tmp = scommand_front(pipe_front);     // Tomo de adelante.
            cmd[i] = strdup(tmp);                       // Copio.
            scommand_pop_front(pipe_front);             // Quito para poder copiar la otra.
        }
        if(execvp(cmd[0], cmd) == -1){
            /*
                Se crea un nuevo proceso y se ejecuta el código del programa
                dado (-1 en caso de fallo), una vez que el proceso hijo es creado, ejecuta un código diferente
                y el proceso padre espera hasta que el hijo salga.
            */
            fprintf(stderr, "Comando inexistente\n");
        }
        //liberar memoria
        for(unsigned int i = 0u; i < length_command; ++i){
            free(cmd[i]);
            cmd[i] = NULL;
        }
        free(cmd);
        cmd = NULL;
    }
}


static void config_redir_in(scommand command){
    assert(command != NULL);

    if(command != NULL){
        char *redir_in = scommand_get_redir_in(command); // <
        if(redir_in){
            //int open(const char *pathname, int flags, mode_t mode);
            //O_RDONLY la persona que llama tiene permiso de lectura sobre el objeto
            int in = open(redir_in, O_RDONLY, S_IRWXU);
            //S_IRUSR: Leer bit
            //S_IWUSR: escribit bit
            //S_IXUSR: ejecutar bit
            dup2(in, STDIN_FILENO); // Duplica descriptores de archivos y apunta al archivo (in.txt) (dump no apunta)
            close(in);
        }
    }
}


static void config_redir_out(scommand command){
    assert(command != NULL);

    char *redir_out = scommand_get_redir_out(command); // > mando a un archivo
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


void execute_pipeline(pipeline apipe){
    assert(apipe != NULL);
    bool pipe_wait = pipeline_get_wait(apipe); // Tomo el valor de wait.

    /* Si el pipeline es vacio */
    if(pipeline_is_empty(apipe)){
        return;
    }

    /* Si es un comando interno */
    if(builtin_is_internal(apipe)){
        builtin_exec(apipe);
        return;
    }

    /* Si solamente es un comando, no hace falta usar pipe */
    if(pipeline_length(apipe) == 1){
        pid_t pid = fork();
        if(pid == 0){   /* Proceso hijo */
            scommand pipe_front = pipeline_front(apipe);
            if(apipe != NULL){
                config_redir_in(pipe_front);
                config_redir_out(pipe_front);
            }
            execute_simple_scommand(apipe);
        }else if(pid == -1){ /* Error con fork */
            fprintf(stderr, "Error con el fork\n");
        }else{ /* Proceso padre */
            if(pipe_wait){
                waitpid(pid, NULL, 0);
            }
        }
        return;

    }else{ //mas de un comando simple -> uso pipe
        int **vec_pipe = calloc(1, sizeof(int*)); // pipeline_length(apipe) - 1 porque necesito n-1 tuverias
        
        for(unsigned int i = 0u; i < 1; ++i){
            vec_pipe[i] = calloc(1, sizeof(int));
            pipe(vec_pipe[i]); //crea la tuveria FIFO
        }

        unsigned int *copy_fork = calloc(1, sizeof(int));
        for(unsigned int i = 0u; i < 2; ++i){
            pid_t pid = fork();
            if (pid == 0){ // ls | grep (filtro)                
                if(i < 1){ // Hijo 1 (primer comando ls)
                    dup2(vec_pipe[i][1], STDOUT_FILENO); // Copio (1) y apunto STDOUT
                    close(vec_pipe[i][0]); // cierro stdin(0)
                    close(vec_pipe[i][1]); // cierro stdout
                }
                
                if(i > 0){ // Hijo 2 (segundo comando filtro)
                    dup2(vec_pipe[i-1][0],STDIN_FILENO); // copio (0) y apunto a STDIN
                    close(vec_pipe[i-1][0]); // cierro stdin(0)
                    close(vec_pipe[i-1][1]); // cierro stdout
                }
                
                scommand pipe_front = pipeline_front(apipe);
                if(apipe != NULL){
                    config_redir_in(pipe_front);
                    config_redir_out(pipe_front);
                }
                execute_simple_scommand(apipe);
            } else if(pid == -1){
                fprintf(stderr, "Error con el fork\n");
            } else {
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
        for (unsigned int i = 0u; i < 1; ++i){
            free(vec_pipe[i]);
            vec_pipe[i] = NULL;
        }
        free(vec_pipe);
        vec_pipe = NULL;
    }
}