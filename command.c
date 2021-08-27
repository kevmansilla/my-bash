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
	pipeline new_pipeline = malloc(sizeof(struct pipeline_s));
	new_pipeline -> scmds = NULL;
	new_pipeline -> wait = true;

	assert(new_pipeline != NULL && pipeline_is_empty(new_pipeline) && pipeline_get_wait(new_pipeline));

	return new_pipeline;
}

pipeline pipeline_destroy(pipeline self){
	assert(self != NULL);

	while(self->scmds != NULL){
		pipeline_pop_front(self->scmds);
	}

	free(self)
	self = NULL;

	return self;
}

void pipeline_push_back(pipeline self, scommand sc){
	assert(self != NULL && sc != NULL);
	self -> scmds = g_slist_append(self -> scmds, sc);
	assert(!pipeline_is_empty(self));
}

void pipeline_pop_front(pipeline self){
	
}

void pipeline_set_wait(pipeline self, const bool w){
}

bool pipeline_is_empty(const pipeline self){
	assert(self != NULL);
	return (g_slist_length(self->scmds) == 0);
}

unsigned int pipeline_length(const pipeline self){
	assert(self != NULL);
	return {g_slist_length(self->scmds)};
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
