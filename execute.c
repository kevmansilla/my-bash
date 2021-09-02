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

static void simple_ex_command(pipeline apipe){
    if (builtin_is_internal(apipe)){
        builtin_exec(apipe); //ejecuto comendo interno
    } else {
        //traigo el comando simple de adelante de la seguencia
        scommand pipe_front = pipeline_front(apipe);
        unsigned int length_command = scommand_length(pipe_front);
        //array (puntero a *char), pido memoria
        char **cmd = calloc(length_command + 1, sizeof(char *));// +1
        for (unsigned int i = 0u; i < length_command; ++i){
            char *tmp = scommand_front(pipe_front); //tomo de adelante
            cmd[i] = strdup(tmp);      //copio
            scommand_pop_front(pipe_front); // quito para poder copiar la otra
        }
        if (execvp(cmd[0], cmd) == -1){ //rear un nuevo proceso y ejecutar el código del programa
            //dado (-1 en caso de fallo), una vez que el proceso hijo es creado, ejecuta un código
            //diferente y el proceso padre espera hasta que el hijo salga
            fprintf(stderr, "Comando inexistente");
            exit(1);
        }
        //liberar memoria
        for (unsigned int i = 0u; i < length_command; ++i){
            free(cmd[i]);
            cmd[i] = NULL;
        }
        free(cmd);
        cmd = NULL;
    }
}

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
            // (luego borrar comentarios -> ls -l ej1.c > out < in (comando simple)
            // cuando tengo que redirigir, se hace con la entrada y la salida
            // tenemos inicialmente un fork, luego un open() y close() con dup2() -duplicamos-
            // execvp para correr el hijo y wait para que el padre espere.
            scommand pipe_front = pipeline_front(apipe);
            if (apipe != NULL){
                char *redir_in = scommand_get_redir_in(pipe_front); // <
                if (redir_in){
                    //int open(const char *pathname, int flags, mode_t mode);
                    //O_RDONLY la persona que llama tiene permiso de lectura sobre el objeto
                    int in = open(redir_in, O_RDONLY, S_IRUSR | S_IWUSR | S_IXUSR);
                    //S_IRUSR: Leer bit
                    //S_IWUSR: escribit bit
                    //S_IXUSR: ejecutar bit
                    dup2(in, STDIN_FILENO); //duplica descriptores de archivos y apunta al archivo (dump no apunta)
                    close(in);
                }
                char *redir_out = scommand_get_redir_out(pipe_front); //> mando a un archivo
                if (redir_in){
                    int out = open(redir_out,  O_CREAT | O_WRONLY , S_IRWXU);
                    //O_CREAT: sino existe se crea
                    //O_WRONLY: para escritura
                    //S_IRWXU: permisos de lectura, escritura y ejecucion
                    //S_IRWXU: el usuario (el propietario del fichero) tiene permisos de lectura, escritura y ejecución
                    dup2(out, STDOUT_FILENO);
                    close(out);
                }
            }
            simple_ex_command(apipe);
        }else if(pid == -1){
            fprintf(stderr, "Error con el fork");
            exit(1);
        }else{ /* El proceso es padre */
            waitpid(pid, NULL, 0);
        }

        return;
    }
}
