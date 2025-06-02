#include <stdio.h>
#include <stdlib.h>
#define OLA_RL_BUFSIZE 1024 // taille du tampon pour la ligne de commande


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