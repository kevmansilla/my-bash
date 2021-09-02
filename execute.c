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

static void execute_simple_command(pipeline apipe){
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
            fprintf(stderr, "Comando inexistente");
            exit(1);
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
    
    char *redir_in = scommand_get_redir_in(command);
    if(redir_in){
        //int open(const char *pathname, int flags, mode_t mode);
        //O_RDONLY la persona que llama tiene permiso de lectura sobre el objeto
        int in = open(redir_in, O_RDONLY, S_IRUSR | S_IWUSR | S_IXUSR);
        //S_IRUSR: Leer bit
        //S_IWUSR: escribit bit
        //S_IXUSR: ejecutar bit
        dup2(in, STDIN_FILENO);
        close(in);
    }
}


static void config_redir_out(scommand command){
    assert(command != NULL);
    
    char *redir_out = scommand_get_redir_out(command);
    if(redir_out){
        int in = open(redir_out, O_RDONLY, S_IRUSR | S_IWUSR | S_IXUSR);
        //O_CREAT: sino existe se crea
        //O_WRONLY: para escritura
        //S_IRWXU: permisos de lectura, escritura y ejecucion
        //S_IRWXU: el usuario (el propietario del fichero) tiene permisos de lectura, escritura y ejecución
        dup2(in, STDIN_FILENO);
        close(in);
    }
}


void execute_pipeline(pipeline apipe){
    assert(apipe != NULL);
    bool pipe_wait = pipeline_get_wait(apipe); // Me fijo si wait es true.

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
        if(pid == 0){   /* El proceso es hijo */
            // (luego borrar comentarios -> ls -l ej1.c > out.txt < in.txt (comando simple)
            // cuando tengo que redirigir, se hace con la entrada y la salida
            // tenemos inicialmente un fork, luego un open() y close() con dup2() -duplicamos-
            // execvp para correr el hijo y waitpid para que el padre espere.
            scommand pipe_front = pipeline_front(apipe);
            if(apipe != NULL){
                config_redir_in(pipe_front);
                config_redir_out(pipe_front);
            }
            execute_simple_command(apipe);
        }else if(pid == -1){
            fprintf(stderr, "Error con el fork");
            exit(1);
        }else{ /* El proceso es padre */
            if(pipe_wait){
                waitpid(pid, NULL, 0);
            }
        }

        return;
    }
}
