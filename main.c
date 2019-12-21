#include<stdio.h> 
#include<string.h>
#include<stdbool.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
#include<dirent.h>
#define SH_TOK_BUFSIZE 64

bool sh_cd(char **args);
bool sh_help(char **args);
bool sh_exit(char **args);
bool sh_ls(char **args);

char* builtin_str[] = {
    "cd",
    "help",
    "exit",
    "ls"
};

bool (*builtin_fun[])(char**) = {
    &sh_cd,
    &sh_help,
    &sh_exit,
    &sh_ls
};

int sh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

/*  
 * Build In function implementations
 * */

bool sh_cd(char **args) {
    if(args[1] == NULL) {
        fprintf(stderr, "sh: expected argument to \"cd\"\n");
    }
    else {
        if(chdir(args[1]) != 0) {
            perror("sh");
        }
    }
    return true;
}

bool sh_help(char **args) {
    printf("Anurag Shah's Shell\n");
    printf("The following built in functions are implemented by me and all other functions are of linux's.\n");
    for(int i = 0; i < sh_num_builtins(); i++) {
        printf("%d. %s\n", i+1, builtin_str[i]);
    }
    printf("Use man command for information for other functions.\n");
    return true;
}

bool sh_exit(char **args) {
    return false;
}

bool sh_ls(char **args) {
    if(args[2] != NULL) {
        fprintf(stderr, "sh: expected 1 argument to \"ls\" provided more than 1 arguments\n");
    }
    else {
        struct dirent **namelist;
        int n;
        if(args[1] != NULL) {
            n = scandir(args[1], &namelist, NULL, alphasort);
        }
        else {
            n = scandir(".", &namelist, NULL, alphasort);
        }
        if (n < 0)
            perror("scandir error");
        else {
            while (n--) {
                printf("%s\n", namelist[n]->d_name);
                free(namelist[n]);
            }
            free(namelist);
        }
    }
    return true;
}

char* sh_read_line(void) {
    int size = SH_TOK_BUFSIZE;
    char* line = malloc(size * sizeof(char));
    int characters = 1;
    char ch = getchar();
    while(ch != '\n' || ch == EOF) {
        while(size < characters) {
            //printf("allocating space : %d\n", size);
            size += SH_TOK_BUFSIZE;
            line = realloc(line, size * sizeof(char));
            //printf("charatersize = %d line size = %d\n", characters, size);
            if(!line) {
                //printf("Error in reallocating buffer!\n");
                exit(EXIT_FAILURE);
            }
        }
        line[characters-1] = ch;
        ch = getchar();
        characters++;
    }
    line[characters-1] = '\0';
    //printf("Found: %s\n", line);
    return line;
}

char** sh_tokenize(char* line) {
    int bufsize = SH_TOK_BUFSIZE;
    char* token = strtok(line, " ");
    char** tokens = malloc(bufsize * sizeof(char*));
    int position = 1;
    tokens[0] = token;
    
    while(token) {
        while(position > bufsize) {
            bufsize += SH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize*sizeof(char*));
        }
      //  printf("token found = %s\n", token);
        token = strtok(NULL, " ");
        tokens[position] = token;
        position++;
    }
    return tokens;
}

bool launch(char **args) {
    pid_t pid, wpid;
    int status;
    
    pid = fork();
    if(pid == 0) {
        if(execvp(args[0], args) == -1) {
            perror("sh: cannot execute");
        }
        exit(EXIT_FAILURE);
    }
    else if(pid < 0) {
        perror("sh: Error forking\n");
    }
    else {
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return true;
}

bool sh_execute(char** args) {
    if(args[0] == NULL) {
        return true;
    }
    //printf("args[0] = %s, length = %lu\n", args[0], strlen(args[0]));
    for(int i = 0; i < sh_num_builtins(); i++) {
        //printf("function name = %s, value = %d, length = %lu\n", builtin_str[i], strcmp(args[0], builtin_str[i]), strlen(builtin_str[i]));
        if(strcmp(args[0], builtin_str[i]) == 0) {
            //printf("Matched with %s", builtin_str[i]);
            return (*builtin_fun[i])(args);
        }
    }
    return launch(args);
}
/*
char* rewind() {
    
}*/

void main() {
    char *line;
    char **args;
    bool status = true;
    
    do {
        printf("> ");
        line = sh_read_line();
        args = sh_tokenize(line);
        status = sh_execute(args);
        
        free(line);
        free(args);
    }while(status);
}
