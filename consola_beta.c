#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
 
/* The array below will hold the arguments: args[0] is the comando. */
static char* args[512];
static int n = 0;      /* number of calls to 'comando' */
pid_t pid;
int comando_pipe[2];


static void esperar(int);
static int ejecutar(char *, int , int , int );
static int comando(int, int, int);
static void separar(char *);
static char * trim(char *);
static char * substring( char *, int ) ;
static int findchar( char *, char ); 
 
int main()
{
	char linea[100];
	char *cmd;
	char *pos;
	char *cmdfile, *archivotrim;
	char archivo[100];
	int posr, out, save_out, salto;

	printf("**********************************************************\n");
	printf("*             CONSOLA IMPLEMENTADA CON PIPES             *\n");
	printf("**********************************************************\n");

	while (1) {
		/* Imprimir cursor */
		printf("(>*.*)> ");
		fflush(NULL);
 
		/* Leer linea de comandos */
		if (!fgets(linea, 100, stdin)) 
			return 0;
 
		int input = 0;
		int first = 1;
 
		cmd = linea;
		posr = findchar(cmd, '>'); /* Buscar simbolo de redireccionamiento '>' */
		if( posr != -1 ) {
			save_out = dup(STDOUT_FILENO);  // salida estandar original

			cmdfile = substring( cmd, posr );	
			strcpy(archivo, cmd+posr+1);	
			salto = findchar(archivo, '\n');
			archivo[salto] = '\0';
			archivotrim = trim(archivo);

		    out = open( archivotrim, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR |S_IRGRP | S_IWGRP | S_IWUSR );	
			dup2( out, 1 );
			close(out);
			cmd = cmdfile;
		}

		//printf( "%s, archivo = %s\n", cmdfile, archivo );
		
		pos = strchr(cmd, '|'); /* Buscar el pipe '|' */
 
		if (pos != NULL) {

			*pos = '\0';
			input = ejecutar(cmd, input, first, 0);
 
			cmd = pos + 1;
			first = 0;
		}
		input = ejecutar(cmd, input, first, 1);
		if( posr != -1 ) {
			/* regresar a la salida estandar,
			   si hubo redireccionamiento
			*/
			dup2(save_out, STDOUT_FILENO);			
		}

		esperar(n);

		n = 0;
	}
	return 0;
}
 
/*  Funcion para esperar todos los procesos   */
static void esperar(int n)
{
	int i;
	for (i = 0; i < n; ++i) 
		wait(NULL); 
}
 
 
static int ejecutar(char* cmd, int input, int first, int last)
{
	separar(cmd);

	if (args[0] != NULL) {
		if (strcmp(args[0], "salir") == 0) 
			exit(0);
		n += 1;
		return comando(input, first, last);
	}
	return 0;
}
 
static int comando(int input, int first, int last)
{
	int pipettes[2];
 
	pipe( pipettes );	
	pid = fork(); 
 
	if (pid == 0) {
		if (first == 1 && last == 0 && input == 0) {
			dup2( pipettes[1], STDOUT_FILENO );
		} else if (first == 0 && last == 0 && input != 0) {
			dup2(input, STDIN_FILENO);
			dup2(pipettes[1], STDOUT_FILENO);
		} else {
			dup2( input, STDIN_FILENO );
		}
 
		if (execvp( args[0], args) == -1)
			_exit(EXIT_FAILURE); 
	}
 
	if (input != 0) 
		close(input);
 
	close(pipettes[1]);

	if (last == 1)
		close(pipettes[0]);
 
	return pipettes[0];
}

static char* trim(char* s)
{
	while (isspace(*s)) ++s;
	return s;
}
 
static void separar(char* cmd)
{
	cmd = trim(cmd);
	
	char* next = strchr(cmd, ' ');
	int i = 0;
 
	while(next != NULL) {
		next[0] = '\0';
		args[i] = cmd;
		++i;
		cmd = trim(next + 1);
		next = strchr(cmd, ' ');
	}
 
	if (cmd[0] != '\0') {
		args[i] = cmd;
		next = strchr(cmd, '\n');
		next[0] = '\0';
		++i; 
	}
 
	args[i] = NULL;
}

static char* substring( char *original, int n ) {
	char * nueva = malloc( sizeof(char)*n+1 );
	strncpy( nueva, original, n );
	nueva[n] = '\0';
	return nueva;
}

static int findchar( char *original, char c ) {
	char find;
	int i=0;
	int tam = strlen( original );

	for( i=0; i<tam; i++ ) {
		if( original[i]==c ) {
			return i;
		}
	}
	return -1;
}
