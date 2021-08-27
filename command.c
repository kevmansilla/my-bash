#include <stdio.h>
#include <glib.h>
#include <assert.h>
#include <stdbool.h>

#include "command.h"

/********** COMANDO SIMPLE **********/

/* Estructura correspondiente a un comando simple.
 * Es una 3-upla del tipo ([char*], char* , char*).
 */

struct scommand_s {
    GSList *args;
    char * redir_in;
    char * redir_out;
};


scommand scommand_new(void){
    scommand new_command = malloc(sizeof(struct scommand_s));
	new_command->args = NULL;
	new_command->redir_in = NULL;
	new_command->redir_out = NULL;
	assert(new_command != NULL && scommand_is_empty(new_command) && scommand_get_redir_in(new_command) == NULL && scommand_get_redir_out(new_command) == NULL);
	return (new_command);
}

scommand scommand_destroy(scommand self){
	assert(self != NULL);
	g_slist_free(self->args);
	if (self->redir_in != NULL){
		free(self->redir_in);
		self->redir_in = NULL;
	}
	if (self->redir_out != NULL){
		free(self->redir_out);
		self->redir_out = NULL;
	}
	free(self);
	self = NULL;

	return self;
}

void scommand_push_back(scommand self, char * argument){
    assert(self != NULL && argument != NULL);
    self->args = g_slist_append(self->args, argument);
    assert(!scommand_is_empty(self));
}

void scommand_pop_front(scommand self){
	assert(self != NULL && !scommand_is_empty(self));
	self->args = g_slist_delete_link(self->args, self->args);
}

void scommand_set_redir_in(scommand self, char * filename){
	assert (self != NULL);
	if (self->redir_in == NULL){
		self->redir_in = filename;
	} else {
		free(self->redir_in);
		self->redir_in = filename;
	}
}

void scommand_set_redir_out(scommand self, char * filename){
    assert (self != NULL);
	if (self->redir_out == NULL){
        self->redir_out = filename;
    } else {
        free(self->redir_out);
        self->redir_out = filename;
    }
}

bool scommand_is_empty(const scommand self){
	assert(self != NULL);
	return (g_slist_length(self->args) == 0);
}

unsigned int scommand_length(const scommand self){
	assert(self != NULL);
	return (g_slist_length(self->args));
}

char * scommand_front(const scommand self){
    assert(self != NULL && !scommand_is_empty(self));
	char *front = (char *)g_slist_nth_data(self->args, 0);
	assert(front != NULL);
	return front;
}

char * scommand_get_redir_in(const scommand self){
    assert(self != NULL);
	return (self->redir_in);
}

char * scommand_get_redir_out(const scommand self){
    assert(self != NULL);
	return (self->redir_out);
}

char * scommand_to_string(scommand comando){

    unsigned int length_command = 0;
    // Primero contamos la cantidad de caracteres de todo el comando:
    for (unsigned int i = 0; i < g_slist_length(comando->args); i++){
        char *current_scommand = g_slist_nth_data(comando->args, i);
        length_command += strlen(current_scommand) + 1; 
    }
    if(comando->redir_out != NULL){
        // Hay que sumar a length 2 por "> " + el len(redir_out) + 1 espacio:
        length_command += strlen(comando->redir_out) + 3;
    }
    if(comando->redir_in != NULL){
        // Hay que sumar a length 2 por "< " + el len(redir_in) + el caracter nulo \0:
        length_command += strlen(comando->redir_in) + 3;
    }

    // Asignamos memoria al nuevo string:
    char *res_command = (char *)calloc(length_command, sizeof(char));

    // Concatenamos los strings: char *strcat(char *dest, const char *src)
    for (unsigned int i = 0; i < g_slist_length(comando->args); i++){
        char *current_scommand = g_slist_nth_data(comando->args, i);
        strcat(res_command, current_scommand);
        strcat(res_command, " ");
    }
    if(comando->redir_out != NULL){
        strcat(res_command, "> ");
        strcat(res_command, comando->redir_out);
    }
    if(comando->redir_in != NULL){
        strcat(res_command, " < ");
        strcat(res_command, comando->redir_in);
    }

    return res_command;
}



/********** COMANDO PIPELINE **********/

/* Estructura correspondiente a un comando pipeline.
 * Es un 2-upla del tipo ([scommand], bool)
 */

struct pipeline_s {
    GSList *scmds;
    bool wait;
};


pipeline pipeline_new(void){
	pipeline new_pipeline = malloc(sizeof(struct pipeline_s));
	new_pipeline -> scmds = NULL;
	new_pipeline -> wait = true;

	assert(new_pipeline != NULL && pipeline_is_empty(new_pipeline) && pipeline_get_wait(new_pipeline));

	return new_pipeline;
}

pipeline pipeline_destroy(pipeline self){
	assert(self != NULL);

	while(self->scmds != NULL){
		pipeline_pop_front(self);
	}

	free(self);
	self = NULL;

	return self;
}

void pipeline_push_back(pipeline self, scommand sc){
	assert(self != NULL && sc != NULL);
	self -> scmds = g_slist_append(self -> scmds, sc);
	assert(!pipeline_is_empty(self));
}

void pipeline_pop_front(pipeline self){
    assert(self != NULL && !pipeline_is_empty(self));
	self->scmds = g_slist_delete_link(self->scmds, self->scmds);
}

void pipeline_set_wait(pipeline self, const bool w){
    assert(self);
    self->wait = w;
}

bool pipeline_is_empty(const pipeline self){
	assert(self != NULL);
	return (g_slist_length(self->scmds) == 0);
}

unsigned int pipeline_length(const pipeline self){
	assert(self != NULL);
	return g_slist_length(self->scmds);
}

scommand pipeline_front(const pipeline self){
    assert(self != NULL && !pipeline_is_empty(self));
	scommand front = g_slist_nth_data(self->scmds, 0);
	assert(front != NULL);

	return front;
}

bool pipeline_get_wait(const pipeline self){
	return (self->wait);
}

char * pipeline_to_string(const pipeline self){
	// ls -l | grep -i glibc | grep -i glibc & -> 39
	// ls -l | grep -i glibc & -> 23
	// ls -l -> 6
	// grep -i glibc -> 14

	// ls -l ej1.c > out.txt < in.txt

	assert(self != NULL);
	
	unsigned int length_command = 0;

	char *current_scommand = NULL;
    
	// Primero contamos la cantidad de caracteres de todo el self:
    for (unsigned int i = 0; i < g_slist_length(self->scmds); i++){
        current_scommand = g_slist_nth_data(self->scmds, i);
        length_command += strlen(current_scommand) + 1;
    }

	// Si wait es true
	if (self->wait){
		length_command += strlen(current_scommand) + 1;
	}

	// Suma el | + el espacio entre | y el comando a la derecha.
	length_command += (g_slist_length(self->scmds) - 1) * 2;

    // Asignamos memoria al nuevo string:
    char *res_command = (char *)calloc(length_command, sizeof(char));

    // Concatenamos los strings: char *strcat(char *dest, const char *src)
    for (unsigned int i = 0; i < g_slist_length(self->scmds); i++){
		char *current_scommand = g_slist_nth_data(self->scmds, i);
		strcat(res_command, current_scommand);
        strcat(res_command, " ");
		
		if (g_slist_length(self->scmds) > 1 && i < g_slist_length(self->scmds) - 1){
			strcat(res_command, "| ");
		}
    }

    if(self->wait){
        strcat(res_command, "&");
    }

    return res_command;
}
