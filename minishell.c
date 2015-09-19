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

/* Tamaño máximo que puede tener la entrada */
#define MAX_INPUT 1024

/* fichero de entrada salida y error estandar */
#define std_in 0
#define std_out 1
#define std_err 2

typedef void (*sighandler_t)(int);

char *NombreSenal(int sen)  /*devuelve el nombre senal a partir de la senal*/ 
{/* para sitios donde no hay sig2str*/

    if (sen == SIGHUP) return "SIGHUP";
    if (sen == SIGINT) return "SIGINT";
    if (sen == SIGQUIT) return "SIGQUIT";
    if (sen == SIGILL) return "SIGILL"; 
    if (sen == SIGTRAP) return "SIGTRAP";
    if (sen == SIGABRT) return "SIGABRT";
    if (sen == SIGIOT) return "SIGIOT";
    if (sen == SIGBUS) return "SIGBUS"; 
    if (sen == SIGFPE) return "SIGFPE";
    if (sen == SIGKILL) return "SIGKILL";
    if (sen == SIGUSR1) return "SIGUSR1";
    if (sen == SIGSEGV) return "SIGSEGV";
    if (sen == SIGUSR2) return "SIGUSR2"; 
    if (sen == SIGPIPE) return "SIGPIPE";
    if (sen == SIGALRM) return "SIGALRM";
    if (sen == SIGTERM) return "SIGTERM";
    if (sen == SIGCHLD) return "SIGCHLD";
    if (sen == SIGCONT) return "SIGCONT";
    if (sen == SIGSTOP) return "SIGSTOP";
    if (sen == SIGTSTP) return "SIGTSTP"; 
    if (sen == SIGTTIN) return "SIGTTIN";
    if (sen == SIGTTOU) return "SIGTTOU";
    if (sen == SIGURG) return "SIGURG";
    if (sen == SIGXCPU) return "SIGXCPU"; 
    if (sen == SIGXFSZ) return "SIGXFSZ";
    if (sen == SIGVTALRM) return "SIGVTALRM";
    if (sen == SIGPROF) return "SIGPROF";
    if (sen == SIGWINCH) return "SIGWINCH"; 
    if (sen == SIGSYS) return "SIGSYS";
    if (sen == SIGIO) return "SIGIO";
/*estas senales no las hay en todos partes*/
#ifdef SIGPOLL
    if (sen == SIGPOLL) return "SIGPOLL";
#endif
#ifdef SIGPWR
    if (sen == SIGPWR) return "SIGPWR";
#endif
#ifdef SIGINFO
    if (sen == SIGINFO) return "SIGINFO";
#endif
#ifdef SIGEMT
    if (sen == SIGEMT) return "SIGEMT";
#endif
#ifdef SIGSTKFLT
 if (sen == SIGSTKFLT) return "SIGSTKFLT";
#endif
#ifdef SIGCLD
 if (sen == SIGCLD) return "SIGCLD";
#endif
#ifdef SIGLOST
 if (sen == SIGLOST) return "SIGLOST";
#endif
#ifdef SIGCANCEL
 if (sen == SIGCANCEL) return "SIGCANCEL";
#endif
#ifdef SIGTHAW
 if (sen == SIGTHAW) return "SIGTHAW";
#endif
#ifdef SIGFREEZE
 if (sen == SIGFREEZE) return "SIGFREEZE";
#endif
#ifdef SIGLWP
 if (sen == SIGLWP) return "SIGLWP";
#endif
#ifdef SIGWAITING
 if (sen == SIGWAITING) return "SIGWAITING";
#endif
 return ("SIGUNKNOWN");
}

void interactive_jobControl_signals(sighandler_t handler)
{
	signal(SIGINT, handler);
	signal(SIGQUIT, handler);
	signal(SIGTSTP, handler);
	signal(SIGTTIN, handler);
	signal(SIGTTOU, handler);
}

/* Almacenamos el directorio actual y el anterior accedido */
char *current_directory = NULL;
char *old_directory = NULL;



enum status {//Estados que puede tener un proceso
	running,
	stopped
};

char *status2string(enum status status)
{
	switch (status) {
		case running: return "running";
		case stopped: return "stopped";
		default:      return "???????";
	
	}
}

struct node {//Elements of tail
	int id;
	pid_t pid;/*pid del proceso*/
	char *command;/* nombre del comando */
	enum status status;/* estado en el que se encuentra nuestro proceso */
	int process; /* Indica el número de procesos que aún quedan por rematar */
	struct node *previous;
	struct node *next;
};

struct tail{
	struct node *first;
	struct node *end;
} tail = {NULL, NULL};


bool tail_is_empty(const struct tail tail)
{//return true if tail is empty
	return ((tail.first == NULL) && (tail.end == NULL))? true: false;
}

void node_init(struct node *ptr, const int id, const pid_t pid, char *command, enum status status, const int process)
{//Init node
	ptr->id = id;
	ptr->pid = pid;
	ptr->command = command;
	ptr->status = status;
	ptr->next = ptr->previous = NULL;
	ptr->process = process;
}

int tail_add(struct tail *tail, const pid_t pid, char *command, enum status status, int process)
{//add a element into tail
	struct node *new = malloc(sizeof(struct node));
	if (new == NULL) {
		perror("cola_add: malloc");	
		return -1;//NO MEMORY
	}
	
	int id = 1;
	if (tail->first != NULL) {//Tail isn't empty
		id = (tail->end)->id + 1;
		node_init(new, id, pid, command, status, process);//Init node
		new->previous = tail->end;
		(tail->end)->next = new;
	}
	else{//Tail is empty
		node_init(new, id, pid, command, status, process);//Init node
		tail->first = new;
	}
	
	tail->end = new;
	return 0;
}

struct node *tail_delete(struct tail *tail, const int id)
{//Delete a element of tail
	if (tail->first != NULL) {
		struct node *auxNext, *aux = tail->first;
		while ((aux->next != NULL) && (aux->id != id))/* la segunda es la comparacion */
			aux = aux->next;
		if (aux->id == id) {//It's the element to delete
			if (aux != tail->end)
				(aux->next)->previous = aux->previous;
			else
				tail->end = aux->previous;
			if (aux != tail->first)
				(aux->previous)->next = aux->next;
			else
				tail->first = aux->next;
			if (aux->command != NULL)
				free(aux->command);
			auxNext = aux->next;		
			free(aux);
			return auxNext;
		}
		return 0;
	}

	return NULL;//The element doesn't exist
}

struct node *tail_find(const struct tail tail, const int id)
{
	if (tail.first != NULL) {//Tail isn't empty
		struct node *aux = tail.first;
		while ((aux != NULL) && (id != aux->id))
			aux = aux->next;
		if ((aux != NULL) && (aux->id == id))
			return aux;
	}
	
	/* The element doesn't exist */
	return NULL;
}

void tail_print(const struct tail tail)
{//Print whole tail
	if (tail.first != NULL) {//Tail isn't empty
		struct node *aux = tail.first;
		while (aux != NULL) {
			printf("[%d] (%d) %s %s\n", aux->id, aux->pid, status2string(aux->status), aux->command);	
			aux = aux->next;	
		}
		return;
	}
	
	/* Tail is empty */
}

void tail_delete_whole(struct tail *tail)
{
	if (tail->first == NULL)
		return;

	struct node *aux2, *aux1 = tail->first;
	
	while (aux1 != NULL) {
		aux2 = aux1;
		aux1 = aux1->next;
		free(aux2->command);
		free(aux2);
	}
}

int tail_update(struct tail *tail)
{/* Modificarla para que se adapte a la multilista */
	struct node *auxNext, *aux = tail->first;

	int status;
	int pid;
	while (aux != NULL) {
		auxNext = aux->next;
		pid = waitpid(-(aux->pid), &status, WNOHANG|WUNTRACED|WCONTINUED );
		if (pid > 0){

		   if (WIFCONTINUED(status)) {
		   	printf("[%d] recieved SIGCONT\n", aux->id);
				aux->status = running;
	      	}
			if (WIFEXITED(status)){/*tengo que borrarlo de la cola*/
				/* decrementamos en uno el numero de procesos */
				aux->process = aux->process - 1;
				if (aux->process == 0) {/* acabaron todos los procesos */
					printf("[%d] Done\n", aux->id);
					auxNext = tail_delete(tail, aux->id);
				} else {/* miramos el mismo para saber si acabaron más procesos */
					auxNext = aux;
				}
			}				
			if (WIFSIGNALED(status)){/* tengo que borralo de la cola */
				printf("[%d] killed %s (%d)\n", aux->id, NombreSenal(WTERMSIG(status)), WTERMSIG(status));
				aux->process = aux->process - 1;
				if (aux->process == 0) 
					auxNext = tail_delete(tail, aux->id);
				else/* miramos el mismo para saber si acabaron más procesos */
					auxNext = aux;
			}
			if (WIFSTOPPED(status)){
				printf("[%d] recived %s (%d)\n", aux->id, NombreSenal(WSTOPSIG(status)), WSTOPSIG(status));
				aux->status = stopped;
			}
		}
		else if(pid ==-1) {
			printf("job [%d]: cannot make waitpid(pid = %d)\n", aux->id, aux->pid);
			auxNext = tail_delete(tail, aux->id); /* si te dio fallo pues lo eliminas */
		}
		aux = auxNext;
	}
	return 0;
}

/*fin de la implementación de la cola*/


static struct option long_options[] = {
	{ .name = "help",
	  .has_arg = no_argument,
	  .flag = NULL,
	  .val = 0},
	{0, 0, 0, 0}
};

static void usage(int i)
{
	printf(
		"Usage:  minishell\n"
		"Pequeño shell\n"
		"Opciones:\n"
		"  -h, --help: muestra esta ayuda\n\n"
	);
	exit(i);
}

static void handle_long_options(struct option option, char *arg)
{
	if (!strcmp(option.name, "help"))
		usage(0);
}

static int handle_options(int argc, char **argv)
{
	while (true) {
		int c;
		int option_index = 0;

		c = getopt_long (argc, argv, "lhRsairS",
				 long_options, &option_index);
		if (c == -1)
			break;

		switch (c) {
		case 0:
			handle_long_options(long_options[option_index],
				optarg);
			break;

		case '?':
		case 'h':
			usage(0);
			break;

		default:
			printf ("?? getopt returned character code 0%o ??\n", c);
			usage(-1);
		}
	}
	return 0;
}

int echo_handler(char *args[])
{/* display a line of text */
   int i = 0;
   while (args[i] != NULL) {
      printf("%s ", args[i]);
      i++;
   }
   printf("\n");
   
	return 0;
}

int exit_handler(char *args[])
{/* cause normal process termination */
   if (current_directory != NULL)
      free(current_directory);
   if (old_directory != NULL)
      free(old_directory);
	tail_delete_whole(&tail);

	exit(EXIT_SUCCESS);
	return 0;
}

int pwd_handler(char *args[])
{/* print name of current/working directory */
   printf("%s\n", current_directory);
   
   return 0;
}

/* tipo de datos de los comandos builtin */
struct builtin_command {
	char *name;
	int (*handler)(char **);
};

void update_directory(char **old, char **current, char *new)
{/* update directory's variables */
   char *aux = strdup(new);
   if (*old != NULL)
      free(*old);
   
   *old = *current;
   *current = aux;
}

int cd_home()
{/* change current directory to home directory */
   uid_t uid = getuid();
   struct passwd *passwd = getpwuid(uid);
   if (passwd == NULL) {
      perror("getpwuid");
      return -1;
   }

   if (chdir(passwd->pw_dir) != 0){
      perror("cd_home");
      return -1;
   }
   
   update_directory(&old_directory, &current_directory, passwd->pw_dir);
   
   return 0;
}

int cd_previous()
{/* change current directory to previous directory */
   if (old_directory == NULL) {
      printf("bash: cd: OLDPWD not set\n");
      return -1;
   }
   
   if (chdir(old_directory)!=0) {
      perror("cd_previous");
      return -1;
   }
   
   update_directory(&old_directory, &current_directory, old_directory);
   return 0;
}

int cd_commun(char *directory)
{/* change current directory to 'directory' */
   if (chdir(directory) != 0) {
      perror("cd_commun");
      return -1;
   }
   
   char *tmp = realpath(directory, NULL);
   update_directory(&old_directory, &current_directory, tmp);
   free(tmp);
   
   return 0;
}

int cd_handler(char *args[])
{/* handler change directory */
   if (args[0] == NULL) {//cd
      return cd_home();
   }   

   if (!strcmp("-", args[0])) {//cd -
      return cd_previous();
   }
   
   return cd_commun(args[0]);//cd directory
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
{/*try to find path of file */
/* FREE MEMORY IS NEEDED */
   char *path = strdup(getenv("PATH"));/* creamos una copia del path para no modificar el original */

   char *dir = strtok(path, ":");/* cortamos la copia del path para coger un directorio uno a uno */
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

char *catch_name(char *args[], char *s)
{/* search 's' in 'args'. return the next elemment */
   int i;
   for (i = 0; args[i] != NULL; i++) {
      if (!strcmp(s, args[i]))
         return (args[i+1]);
   }
   
   return NULL;
}

void quit_args(char *args[])
{/* unnecessary arguments is quitted */
   int i;
   
   for (i = 0; args[i] != NULL; i++) {
      if ((!strcmp(args[i], ">")) || (!strcmp(args[i], "<")) || (!strcmp(args[i], "#"))) {
         args[i] = NULL;
         return;
      }
   }
}

int execute(char **args, int input, int output)
{
	char *path = search_path(args[0]);
   if (path == NULL) {
   	printf("%s: command not found\n", args[0]);
      return -1;
   }
      
   if (input!=-1) {
   	if (close(0)==-1)
   		perror("close(0)");
   	else if (dup(input) == -1)
   		perror("dup(input)");
	}
   
   if (output!=-1) {
   	if (close(1) == -1)
   		perror("close(1)");
		else if(dup(output)==-1)
			perror("dup(ouput)");
	}
			
	if (execv(path, args)==-1)
		perror("execv");
	return -1;
}


int kill_job(int sig, int n)
{
	struct node *aux = tail_find(tail, n);
	if (aux == NULL) {
		printf("%d: No such job or process\n",n);
		return -1;
	}

	if (kill(-(aux->pid), sig) == -1) {
		perror("kill");
		return -1;
	}
	return 0;

}

int kill_pid(int sig, pid_t pid)
{
	if (kill(pid, sig) == -1) {
		perror("kill");
		return -1;
	}
	return 0;
}

int kill_handler(char *args[])
{
	if ((args[0] == NULL) || (args[1] == NULL)){
		printf("kill -signal pid_t | %%n\n");
		return -1;
	}
	
	/* primero cogemos la señal */
	if (strncmp(args[0],"-",1)!=0) {
		printf("kill -signal pid_t | %%n\n");
		return -1;
	}
	int sig = atoi(args[0]+1);
	
	int i;
	for (i = 1; args[i] != NULL; i++) {
		if (!strncmp(args[i],"%",1)) {/* primero miro si es un grupo de procesos */
			kill_job(sig, atoi(args[i]+1));
			continue;
		}
		kill_pid(sig, atoi(args[i]));
	}
	
	return 0;
}

int jobs_handler(char *args[])
{
	tail_print(tail);
	
	return 0;
}

int foreground(int n)
{
	struct node *aux = tail_find(tail, n);
	if (aux == NULL) {
		printf("%d: No such process or jobs\n", n);
		return -1;
	}
	
	if (tcsetpgrp(std_in, aux->pid)==-1)
		perror("tcsetpgrp");
	
	/* envio señal al grupo */
	if (kill(-(aux->pid), SIGCONT)==-1){
		perror("kill");
		return -1;
	}
	aux->status = running;

	int status;
	pid_t pid = waitpid(-(aux->pid), &status, WUNTRACED);
	
	if (tcsetpgrp(std_in, getpid())==-1)
		perror("tcsetpgrp");
	
	if (pid == -1) {
		perror("waitpid");/* si da fallo lo quito de la cola???? */
		return -1;
	}
	
	if (pid == 0) {
		printf("No changes\n");
	}
	
	if (pid > 0) {/* Hubo cambio */
		if (WIFEXITED(status)){/*tengo que borrarlo de la cola*/
			printf("[%d] Done\n", aux->id);
			tail_delete(&tail, aux->id);
		}				
		if (WIFSIGNALED(status)){/* tengo que borralo de la cola */
			printf("[%d] killed (-%d)\n", aux->id, WTERMSIG(status));
			tail_delete(&tail, aux->id);
		}
		if (WIFSTOPPED(status)){
			printf("[%d] recieved SIGSTOP\n", aux->id);
			aux->status = stopped;
		}
	}
	return 0;
}

int fg_handler(char *args[])
{
	if (args[0] == NULL) {
		printf("fg %%n\n");
		return -1;
	}
	
	int i;
	for (i = 0; args[i] != 0; i++) {
		if (!strncmp(args[i],"%",1))
			foreground(atoi(args[i]+1));
		else
			printf("%s: bad format\n", args[i]);
	}
	return 0;
}

int background(int n)
{
	/* diferenciar entre un proceso simple y un grupo */
	struct node *aux = tail_find(tail, n);
	if (aux == NULL) {
		printf("%d: No such process or jobs\n", n);
		return -1;
	}
	
	/* señal al grupo */
	if (kill(-(aux->pid), SIGCONT)==-1){
		perror("kill");
		return -1;
	}
	aux->status = running;
	
	int status;
	pid_t pid = waitpid(-(aux->pid), &status, WNOHANG | WUNTRACED);
	
	if (pid == -1) {
		perror("waitpid");/* si da fallo lo quito de la cola???? */
		return -1;
	}
	if (pid > 0) {/* Hubo cambio */
		if (WIFEXITED(status)){/*tengo que borrarlo de la cola*/
			printf("[%d] Done\n", aux->id);
			tail_delete(&tail, aux->id);
		}				
		if (WIFSIGNALED(status)){/* tengo que borralo de la cola */
			printf("[%d] killed (-%d)\n", aux->id, WTERMSIG(status));
			tail_delete(&tail, aux->id);
		}
		if (WIFSTOPPED(status)){
			printf("[%d] recieved SIGSTOP\n", aux->id);
			aux->status = stopped;
		}
	}
	return 0;
}

int bg_handler(char *args[])
{
	if (args[0] == NULL) {
		printf("bg %%n\n");
		return -1;
	}
	int i;
	for (i = 0; args[i] != 0; i++) {
		if (!strncmp(args[i],"%",1))
			background(atoi(args[i]+1));
		else
			printf("%s: bad format\n", args[i]);
	}
	return 0;
}

/* builtin_command */
struct builtin_command builtin[] = {
	{"echo",echo_handler,},
	{"exit",exit_handler,},
	{"pwd", pwd_handler,},
	{"cd", cd_handler,},
	
	{"kill", kill_handler},
	{"jobs", jobs_handler},
	{"fg", fg_handler},
	{"bg", bg_handler},
	{NULL, NULL}
};

bool is_background(char *args[])
{/* 'args' contains "&" */
   int i;
   
   for (i = 0; args[i] != NULL; i++)
   {
      if (strcmp("&", args[i])==0) {
         args[i] = NULL;
         return true;
      }
   }
   
   return false;
}

bool is_pipe(char *args[])
{/* 'args' contains "|" */

	int i;
	for (i = 0; args[i] != NULL; i++)
		if (!strcmp(args[i], "|"))
			return true;
	
	return false;

}

int exec_to_pipe(char **args, int input, int output)
{/* ejecuta un comando reemplazando el espacio de direcciones */
	char *path = search_path(args[0]);
	if (path == NULL) {
		printf("%s: command not found\n", args[0]);
		return -1;
	}

	if (input != -1){
		if (close(0)==-1)
			perror("close(0)");
		else if (dup(input)==-1)
			perror("dup(input)");
	}
	if (output != -1){
		if (close(1)==-1)
			perror("close(1)");
		else if (dup(output)==-1)
			perror("dup(output)");
			
	}
	
	if (execv(path, args)==-1)
		perror("execv");
		
	return -1;
}

int close_fd_to_like_want_son(int not_close, int max_fds, int array[][2])
{/* cierra descriptores de fichero para que se adecuen al hijo*/
	int i;
	for (i = 0; i < max_fds; i++) {
		if (i == not_close)
			continue;
		if (array[i][0] != -1)
			close(array[i][0]);
		if (array[i+1][1] != -1)
			close(array[i+1][1]);
	}
	return 0;
}

int close_fd_all_pipelines(int max_fds, int array[][2])
{/* cierra todos los descriptores de ficheros de los pipes */
	int i;
	for (i = 0; i < max_fds; i++){
		if (array[i][0] != -1)
			close(array[i][0]);
		if (array[i][1] != -1)
			close(array[i][1]);	
	}
	return 0;
}

int init_args_to_pipe(char **args, char **arg_pipe[MAX_INPUT/4])
{/* rellena el array donde en cada posicion hay un comando con sus respectivos argumentos */
	int i, j;
	arg_pipe[0] = args;/* creo el array de argumentos */
	for (j = i = 1; args[i] != NULL; i++)
		if (!strcmp("|",args[i])){
			arg_pipe[j] = args + i + 1;
			args[i] = NULL;
			j++;
		}
	arg_pipe[j] = NULL; // en j tengo el número de procesos a realizar
	return j;
}

int init_pipelines_to_pipe(int fds[][2], int max, int input, int output)
{/* Se crean todos los pipes que se necesitarán para lanzar los comandos */
/* input y output son los fds si se usa redirección de entrada y/o salida */
	int i;

	fds[0][0] = input;
	fds[0][1] = -1;/* pongo a -1 xq el primero no lo necesita */
	for (i = 1; i < max; i++)
		if (pipe(fds[i])==-1) {
			perror("pipe");
			return -1;
		}
	fds[i][0] = -1;
	fds[i][1] = output;/*en el último tmb los pongo a menos uno*/
	
	return 0;
}

char *group_process_name(char ***arg_pipe)
{/* de todos los procesos que hay para el pipe, solo cojemos el nombre */
	size_t size= 0;
	int i;	
	for (i = 0; arg_pipe[i] != NULL; i++)
		size += strlen(arg_pipe[i][0]);
		
	size += i;
	
	char *aux = malloc(sizeof(char)*size);
	if (aux == NULL) {
		perror("malloc");
		return NULL;	
	}
	strcpy(aux, arg_pipe[0][0]);
	for (i = 1; arg_pipe[i]!= NULL; i++) {
		strcat(aux,"|");
		strcat(aux, arg_pipe[i][0]);	
	}
	
	return aux;
}


int do_pipe(char **args, bool bg, int input, int output)
{
	char **arg_pipe[MAX_INPUT/4];
	pid_t pids[MAX_INPUT/4], aux;
	int fds[MAX_INPUT/4-1][2],j,i,status;
	
	j = init_args_to_pipe(args, arg_pipe);

	init_pipelines_to_pipe(fds, j, input, output);/* creo todos los pipes que necesitaré */
	

	for (i = 0; arg_pipe[i]!=NULL; i++) {
		pids[i] = fork();
		if (pids[i] == -1) {
			perror("fork");
			return -1;	
		}
		if (pids[i] == 0) {
			interactive_jobControl_signals(SIG_DFL);
			aux = (pids[0] == 0)?getpid():pids[0];/* What's the first pid */

			if (bg)
				while (getpgrp() != aux)
					;
			else
				while (tcgetpgrp(std_err) != aux)
					;
			close_fd_to_like_want_son(i,j,fds);
			exec_to_pipe(arg_pipe[i], fds[i][0], fds[i+1][1]);
			exit_handler(NULL);/**/
		} else {
			if (setpgid(pids[i], pids[0])==-1)
				perror("setpgid");
			if (!bg) 
				if (tcsetpgrp(std_err, pids[0])==-1)
					perror("tcsetpgrp");
		}
	}
	pids[i] = 0;
	close_fd_all_pipelines(j, fds);
	if (!bg) {
		for (i = 0; i <j; i++)
			aux = waitpid(-pids[0], &status, WUNTRACED);
			if (aux == -1)
				perror("waitpid");
			if (aux>0){ /* it changes */
				if (WIFSTOPPED(status)) {
					printf("(%d) recieved %s (%d)\n",aux, NombreSenal(WSTOPSIG(status)), WSTOPSIG(status));
					tail_add(&tail, pids[0], (j == 1)? strdup(args[0]): group_process_name(arg_pipe),stopped, j);
				}
				if (WIFSIGNALED(status))
					printf("%d killed %s (%d)\n", aux, NombreSenal(WTERMSIG(status)), WTERMSIG(status));	
				
			}
		if (tcsetpgrp(std_err, getpid()) == -1)
			perror("tcsetpgrp");
	} else {/* in background... saving into the tail*/
		tail_add(&tail, pids[0], (j == 1)? strdup(args[0]): group_process_name(arg_pipe),running, j);
	}
	

	return 0;
}

int handle_command(char *commandline)
{
	int i, ret;
	char *args[MAX_INPUT/4];
	const char separate[] = {" \t\n"};
	
	/* Separamos los argumentos */
	args[0] = strtok(commandline, separate);
	for (i = 1; ((args[i] = strtok(NULL, separate)) != NULL); i++)
		if (i == MAX_INPUT/4) {
			printf("too many arguments\n");
			return -1;
		}
   
	if (args[0] == NULL)
		return 0;
	
	for (i = 0; builtin[i].name != NULL; i++) {
		if (!strcmp(args[0], builtin[i].name))
			return builtin[i].handler(args+1);
	}
	
	
	/* No es ningun 'builtin' que tiene nuestro shell, pues ejecutarlo del path */
	bool bg = is_background(args);
	char *finput = catch_name(args,"<"), *foutput = catch_name(args, ">");
	int input = -1, output = -1;
	quit_args(args);
	if (finput != NULL)
		if ((input = open(finput, O_RDONLY)) ==-1)
			perror("open(input)");
	if (foutput != NULL)
		if ((output = open(foutput, O_WRONLY | O_CREAT, 0644)) ==-1)
			perror("open(output)");
	ret = do_pipe(args, bg, input, output);
	/*input y output are closed by do_pipe*/
	return ret;
}

char *pwd()
{/* return current directory */
/* FREE MEMORY IS NEEDED */
   return realpath(".", NULL);
}

int main(int argc, char *argv[])
{
	int result = handle_options(argc, argv);
	pid_t shell_pgid;

	if (result != 0)
		exit(result);

	/* Loop until we are in the foreground.  */
	while (tcgetpgrp(STDIN_FILENO) != (shell_pgid = getpgrp()))
		kill(-shell_pgid, SIGTTIN);	

	/* Ignore interactive and job-control signals.  */
	interactive_jobControl_signals(SIG_IGN);

	/* Put ourselves in own our process group */
	printf("pid %d pgid %d\n", getpid(), getpgrp());

	printf("minishell v0.1\n");

   current_directory = pwd(); /* save current directory */

	while (true) {
		char string[MAX_INPUT];
		char *p;

		printf("prompt $ ");
		p = fgets(string, MAX_INPUT, stdin);
		if (p == NULL) {
			if (!feof(stdin)) {
				perror("error reading command");
			}
			perror("fgets");
			exit(1);
		}
		tail_update(&tail);
		handle_command(p);
	}

	exit(0);
}
