#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <pwd.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#define MAX_INPUT 1024

void print_args(char **a) {
	int i = 0;
	
	for (; a[i] != NULL; i++) {
		printf("%s ", a[i]);	
	}
	
	printf("\n");
}

char *concatenate(char *s1, char *s2, char *s3)
{/* return s1+s2+s3 */
/* FREE MEMORY IS NEEDED */
   size_t size = strlen(s1) + strlen(s2) + strlen(s3);
   char *aux = malloc(sizeof(char) * (size + 1));
   if (aux == NULL) {
      perror("concatenate");
      return NULL;
   }
   
   strcpy(aux, s1);
   strcat(aux, s2);
   return strcat(aux, s3);
}

char *search_path(char *file)
{/* busca el path */
   char *path = strdup(getenv("PATH"));

   char *dir = strtok(path, ":");
   char *aux;
   struct stat stat;
   while (dir != NULL) {
      aux = concatenate(dir, "/", file);
      if (aux == NULL)/* without memory */
         return NULL;

      if (lstat(aux, &stat) == 0){ /*i find it*/
         free(path);
         return aux;
      }
      
      free(aux);
      dir = strtok(NULL, ":");/* and... the next? */
   }
   
   return NULL;
}

int execute(char **args) {
	/* primero buscamos el path */
	char *path = search_path(args[0]);
	
	if (path == NULL) {
		printf("%s: command not found\n",args[0]);
		return -1;
	}
	
	return execv(path, args);
}

pid_t p = fork()
p == 0
	execute
else
	waitpid(p, NULL, 0)

int main(){

	char commandline[MAX_INPUT];
	strcpy(commandline, "ls -l | grep minishell");
	/* ya tengo creada mi linea de comandos ahora a cortarla */

	char *args[MAX_INPUT/4];
   int i;
   
   args[0] = strtok(commandline, " \t\n");
   for (i = 1; ((args[i] = strtok(NULL, " \t\n")) != NULL); i++)
      ;
//print_args(args);
//execute(args);
	
	/* a partir de aquí vamos a empezar a probar el pipe */
	char **args1, **args2; 
	
	
	exit(0);
}

