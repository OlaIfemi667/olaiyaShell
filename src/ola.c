#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#define OLA_RL_BUFSIZE 1024 // taille du tampon pour la ligne de commande
#define OLA_TOK_BUFSIZE 64 // taille du tampon pour les arguments de la commande
#define OLA_TOK_DELIM " \t\r\n\a" // délimiteurs pour diviser la ligne de commande


// les commandes buildin
// Elles doivent etre coder en interne
// Si on prend par exemple est commandes cd ou exit (ou toute commande qui est destiner a modifier le shell)
// nous devons la coder en interne et pas utiliser une commande externe car utiliser une commande externe creera un nouveau process enfant et la
//commande s'exécutera dans l'enfant sans modifier le parent (ici notre shell) ce qui est inutile dans notre cas


int ola_cd(char **args); 
int ola_help(char **args);
int ola_exit(char **args);
int ola_pwd(char ** args);


char *builtin_str[] = {
	"cd",
	"help",
	"exit"
};



int  (*builtin_func[]) (char **) = {
	// ceci est un tableau de pointeur vers les fonctions builtin et chaque fonctions prends un "char **"
	&ola_cd,
	&ola_help,
	&ola_exit
};


int ola_num_builtins(){
	// cette function calcule de nmbre de fonction buildIn dans le 
	return sizeof(builtin_str) / sizeof(char *);
}


/*Implémentation des fonctions builtin*/

int ola_cd(char **args)
{
	if (args[1] == NULL)
	{	 
                fprintf(stderr, "ola: allocation d'espace mémoire échouée\n");
	}
	else
	{
		if (chdir(args[1]) != 0)
		{
			// si le cd échoue 
			perror("ola");
		}
	}
	return 1;
}

int ola_help(char **args)
{
	int i;
	printf("Ola's Shell\n");
	printf("Conçu a partir du tutoriel de Stephen Brennan's LSH \n");
	printf("Voiçi les fontions builtin:\n");

	for (i = 0; i < ola_num_builtins(); i++)
	{
		printf("	%s\n", builtin_str[i]);
	}

	printf("Utilisez man pour avoir des informations sur les autres programmes\n");
	return 1;
}

int ola_exit(char ** args)
{
	return 0;
}


int ola_launch(char **args)
{
	pid_t pid, wpid;
	int status;

	pid = fork();

	if (pid == 0)
	{
		// ce qu\'il faut comprendre c\'est que fork créer deux process un pere et son fils
		// le fils retourne 0  et le pere le pid de son fils
		// pour donc executer une commande on l'exécute dans le fils.
		// Pour faire claire quand on exécuter ce program a la ligne fork un process fils sera créé et on l'utilisera pour exécuter
		// la nouvelle tache et comme pid == 0 l'enfant est bien créé
		// Nous sommes "dans" le process enfant actuellement

		if (execvp(args[0], args) == -1){
			perror("Ola");
		}
		exit(EXIT_FAILURE);
	}

	else if (pid < 0){
		perror("Ola");
	}


	else{
		//Process Père
		//Si on est ici c'est que tout s'est bien passé le process enfant est entrain  d'exécuter la command et le pere doit attendre
		//

		do{
			wpid = waitpid(pid, &status, WUNTRACED);
		}
		while (!WIFEXITED(status) &&!WIFSIGNALED(status));
	}
	return 1;
}



char **ola_split_line(char * ligne){
    int bufsize = OLA_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if (!tokens)
    {
        fprintf(stderr, "ola: allocation d'espace mémoire échouée\n");
        exit(EXIT_FAILURE);
    }
    token = strtok(ligne, OLA_TOK_DELIM); //En gros on utilise strtok pour creer des tokens  a partir de la variable ligne et des délimiteurs ***OLA_TOK_DELIM " \t\r\n\a"
    

    while (token != NULL)
    {
        tokens[position] = token;
        position++;

        if (position >= bufsize)
        {
            bufsize += OLA_TOK_BUFSIZE;
            tokens =  realloc(tokens, bufsize * sizeof(char*));

            if (!tokens)
            {
                exit(EXIT_FAILURE);
            }
            
        }
        token = strtok(NULL, OLA_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
    
}


void *ola_read_line(void)
{
    int bufsize = OLA_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;
    
    if (!buffer){
        fprintf(stderr, "ola: allocation d'espace mémoire échouée\n");
        exit(EXIT_FAILURE);
    }


    while (1)
    {
        c = getchar();

        if (c == EOF || c == '\n') //si End of FIle ou nouvelle ligne do this
        {
            buffer[position] = '\0';
            return buffer;
        }

        else{
            buffer[position] = c;
        }

        position++;


        // si le tampon est plein, réallouer plus d'espace

        if (position >= bufsize) {
            bufsize += OLA_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if(!buffer)
            {
                fprintf(stderr, "ola: allocation d'espace mémoire échouée\n");
                exit(EXIT_FAILURE);
            }
        }
        
    }
    
}



//ola_execute elle permet de choisir si on exécute un built in (si largument entrer correspond a un built) ou un process (si non)

int ola_execute(char **args)
{
	int i;
	
	if( args[0] == NULL)
	{
		return 1;
	}

	for (i = 0; i < ola_num_builtins(); i++)
	{
		if (strcmp(args[0], builtin_str[i]) == 0) //strcmp compare les strings si == 0  alors ils sont ego sinon l'un est plus grand que l'autre et on peut déterminer lequel en fonction du signe
		{
			return (*builtin_func[i])(args);
		}
	}

	return ola_launch(args);
}


// c'est la boucle, classique pour tout les shells
void ola_loop(){
    char *ligne;
    char **args; // c'est in split de ligne
    int status = 1;

    do{
        printf("ola> $~ ");

        ligne = ola_read_line(); // lire la ligne de commande
        args = ola_split_line(ligne); // diviser la ligne en arguments
        status = ola_execute(args); // exécuter la commande

        free(ligne); // libérer la mémoire allouée pour la ligne
        free(args); // libérer la mémoire allouée pour les arguments
    } while (status);

}
// the main function
int main(int argc, char **argv)
{
    ola_loop();
    
    return EXIT_SUCCESS; // retourner le code de succès
}
