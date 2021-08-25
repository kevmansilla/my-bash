#include <stdio.h>
#include <glib.h>
#include <assert.h>

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
	// Asignamos memoria.
    scommand new_command = malloc(sizeof(struct scommand_s));
	new_command -> args = NULL;
	new_command -> redir_in = NULL;
	new_command -> redir_out = NULL;

	//assert(new_command = NULL && scommand_is_empty(new_command) && scommand_get_redir_in(new_command) == NULL && scommand_get_redir_out(new_command) == NULL)
	
	return new_command;
}

scommand scommand_destroy(scommand self){
	//assert (self != NULL)

	if(self -> redir_in != NULL){
		free(self -> redir_in);
		self -> redir_in = NULL;
	}

	if(self -> redir_out != NULL){
		free(self -> redir_out);
		self -> redir_out = NULL;
	}

	free(self);
	self = NULL;
	
	return self;
}

void scommand_push_back(scommand self, char * argument){
}

void scommand_pop_front(scommand self){
}

void scommand_set_redir_in(scommand self, char * filename){
	//assert (self != NULL)
	if(self -> redir_in == NULL){
		self -> redir_in = filename;
	}else{
		free(self -> redir_in);
		self -> redir_in = filename;
	}
}

void scommand_set_redir_out(scommand self, char * filename){
    if (self->redir_out == NULL){
        self->redir_out = filename;
    } else {
        free(self->redir_out);
        self->redir_out = filename;
    }
}

bool scommand_is_empty(const scommand self){
	//assert(self != NULL);
	bool Bool = true;
	return Bool;
}

unsigned int scommand_length(const scommand self){
	//assert(self != NULL);
	int length = 0;
	//length = g_slist_length(list);
	return length;
}

char * scommand_front(const scommand self){
	return NULL;
}

char * scommand_get_redir_in(const scommand self){
	return (self->redir_in);
}

char * scommand_get_redir_out(const scommand self){
	return (self->redir_out);
}

char * scommand_to_string(const scommand self){
	return NULL;
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
	return NULL;
}

pipeline pipeline_destroy(pipeline self){
	return NULL;
}

void pipeline_push_back(pipeline self, scommand sc){
}

void pipeline_pop_front(pipeline self){
}

void pipeline_set_wait(pipeline self, const bool w){
}

bool pipeline_is_empty(const pipeline self){
	return true;
}

unsigned int pipeline_length(const pipeline self){
	return 0;
}

scommand pipeline_front(const pipeline self){
	return NULL;
}

bool pipeline_get_wait(const pipeline self){
	return true;
}

char * pipeline_to_string(const pipeline self){
	return NULL;
}
