/*
  Libraries used
*/
#include <sys/wait.h> //waitpid()
#include <sys/types.h> //chdir(),fork(), exec(), pid_t
#include <unistd.h> // malloc(), realloc(), free(), exit(), execvp(), EXIT_SUCCESS, EXIT_FAILURE
#include <stdlib.h> //fprintf(), printf(), stderr, getchar(), perror()
#include <stdio.h> //fprintf(), printf(), stderr, getchar(), perror()
#include <string.h> //strcmp(), strtok()
#include <unistd.h> //getcwd
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#define clear() printf("\033[H\033[J")
#define MAX_BUF_SIZE 513
#define  TOK_BUFSIZE 64

void execute(char **args,int start, int end,int* status);

/*Function to print history*/
void print_history(void)
{
  char **cmd=(char**) malloc(MAX_BUF_SIZE*sizeof(char*));
  cmd[0]="cat";
  cmd[1]="history";
  if(fork()==0)
  {
    execvp(cmd[0],cmd);
  }wait(0);

  free(cmd);

}

/*Function to create history file*/
void history(char* line)
{
  FILE* fp;
  fp = fopen("history","a");
  if (fp==NULL) return;
  fprintf(fp, "%s", line);
  fclose(fp);
}

/*Function to execute pipes*/
void execute_pipe(char **args,int start_2,int end_2,int start_1,int end_1,int *status){
    int unused __attribute__((unused));
    int i;
    char **cmd1=(char**) malloc(MAX_BUF_SIZE*sizeof(char*));
    char **cmd2=(char**) malloc(MAX_BUF_SIZE*sizeof(char*));
    for(i=start_1; i<=end_1+1; i++)
    {
      cmd1[i-start_1]=args[i];
      if(i==end_1+1) cmd1[i-start_1]=NULL;
    }
    for(i=start_2; i<=end_2+1; i++)
    {
      cmd2[i-start_2]=args[i];
      if(i==end_2+1) cmd2[i-start_2]=NULL;

    }

    int fd[2];
  unused=pipe(fd);
  if ( fork() == 0 ) {
       /* Redirect output of process into pipe */
       close(fd[0]);
       dup2( fd[1], fileno(stdout));
       close(fd[1]);
       execvp(cmd1[0], cmd1);
       printf("Error Command \"%s|%s\" is invalid \n",cmd1[0],cmd2[0]);
       exit(-1);

  }
  if ( fork() == 0 ) {
       /* Redirect input of process out of pipe */
       close(fd[1]);
       dup2( fd[0], fileno(stdin));
       close(fd[0]);
       execvp(cmd2[0], cmd2);
       printf("Error Command \"%s|%s\" is invalid \n",cmd1[0],cmd2[0]);
       exit(-1);
  }
  /* Main process */
  close( fd[0] );
  close( fd[1] );

  wait(status);
  wait(status);

      free(cmd1);
      free(cmd2);
return;
}

/* Signal Handler for SIGINT */
void sigintHandler(int sig_num)
{
    /* Reset handler to catch SIGINT aka CTRL+C */
    signal(SIGINT, sigintHandler);
    clear();
    printf("Ctrl+C is used to clear the screen!\nManousaridis_8855>");
    fflush(stdout);
}
/* Signal Handler for SIGTSTP aka CTRL+Z*/
void sighandler(int sig_num)
{
    clear();
    // Reset handler to catch SIGTSTP next time
    signal(SIGTSTP, sighandler);
    printf("\n\nFunction of Ctrl+Z was not changed for simplicity reasons\n\n");
    exit(0);
}

/*Function to execute redirection <*/
void less(char **args,int start,int end,int* status,char* name)
{
  FILE* fp;
  fp=fopen(name,"r");
  if (fp==NULL) return;
  pid_t pid = fork();
  if(pid==0){
    dup2(fileno(fp),fileno(stdin));
    execute(args,start,end,status);
    exit(0);
  }
  fclose(fp);
  wait(0);
}

/*Function to execute redirections >, >>*/
void greater(char **args,int start,int end,int* status,char* name, char* job)
{
  FILE* fp;
  fp = fopen(name,job);
  if (fp==NULL) return;
  pid_t pid=fork();
  if(pid==0){
    dup2(fileno(fp),fileno(stdout));
    execute(args,start,end,status);
    exit(0);
  }
  fclose(fp);
  wait(0);
}

/*Function to execute commands*/
void execute(char **args,int start, int end,int* status){
  char** cmd=(char**)malloc(MAX_BUF_SIZE*sizeof(char*) );

  for(int i=start; i<end; i++)
  {
    cmd[i-start]=args[i];
  }

  if(strcmp(cmd[0],"cd")==0)
    {
      if (chdir(cmd[1]) != 0) {
           perror("Invalid directory entered");
    }
    return;
  }
  if(strcmp(cmd[0],"help")==0)
    {
      printf("\n");
      printf("This program simulates the working of a shell in a Unix-like \n");
      printf("environment. It was built in the scope of a project at AUTh.\n");
      printf("Built-in commands it supports are: cd, help, history, quit.\n");
      printf("Also, it supports redirections like >, >> , < and &&, ;.\n");
      printf("Signals like CTRL+Z and CTRL+C are changed. Type them for more");
      printf(" info.\n\nAuthor: Giannis Manousaridis\n\n");

    return;
  }
  if(strcmp(cmd[0],"history")==0)
    {
      print_history();
      return;
  }

  int pid=fork();
  if(pid==0)
  {
    if(execvp(cmd[0],cmd)<0){
      printf("Error: Command \"%s\" is invalid \n",cmd[0]);
    }
  }
  wait(status);
  if(status[0]!=0) printf("Error: Command \"%s\" is invalid \n",cmd[0]);
free(cmd);
}

/*Function to execute for creating symbol pointer*/
void parse_symbol(char** args, const char* parser, char* symbol){

  int i=0;
  while(args[i]!=NULL){
      if(strcmp(args[i],">")==0)
      {
        symbol[i]='>';
        if(i==0) symbol[i]='E';
      }else if(strcmp(args[i],">>")==0){
        symbol[i]='a';
        if(i==0) symbol[i]='E';
        }
      else if(strcmp(args[i],"<")==0){
        symbol[i]='<';
        if(i==0) symbol[i]='E';
      }else if(strcmp(args[i],"|")==0){
        symbol[i]='|';
        if(i==0) symbol[i]='E';
      }else if(strcmp(args[i],";")==0){
        symbol[i]=';';
        if(i==0) symbol[i]='E';
      }
      else if(strcmp(args[i],"&&")==0){
        symbol[i]='&';
        if(i==0) symbol[i]='E';
      }
      else if(strncmp(args[i],"&&&",3)==0){
        symbol[i]='E';
      }
      else if(strncmp(args[i],";;",2)==0){
        symbol[i]='E';
      }
      else if(strcmp(args[i],"&")==0){
        symbol[i]='E';
      }
      else if(strncmp(args[i],"||",2)==0){
        symbol[i]='E';
      }
      else
      {
        symbol[i]='N';
      }
      i++;
  }
}
/*Function to remove spaces*/
char** parse_line(char *line){
  int bufsize =  TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token, **tokens_backup;

  if (!tokens) {
    fprintf(stderr, "Allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, " \t\r\n\a");
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize +=  TOK_BUFSIZE;
      tokens_backup = tokens;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
		free(tokens_backup);
        fprintf(stderr, "Allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, " \t\r\n\a");
  }
  tokens[position] = NULL;
  return tokens;
}
char * read_line(void){
  int unused __attribute__((unused));
  char *line = NULL;
  ssize_t bufsize = 0; // have getline allocate a buffer for us
  unused=getline(&line, &bufsize, stdin);
  return line;
}
void  loop(char* arg){


  signal(SIGINT, sigintHandler);
  signal(SIGTSTP, sighandler);

  char* line=(char*)malloc(MAX_BUF_SIZE*sizeof(char));
  char** args=(char**)malloc(MAX_BUF_SIZE*sizeof(char*));
  char* symbol=(char*)malloc(MAX_BUF_SIZE*sizeof(char) );
  int* status=(int*)malloc(sizeof(int));
  FILE* fp;
  char* unused __attribute__((unused));

  if(arg!=NULL){
    fp = fopen(arg,"r");
    if(fp==NULL){
      printf("File %s not found\n", arg);
      exit(-1);
    }
  }

  do {

    if(arg!=NULL){
      unused=fgets(line, MAX_BUF_SIZE, fp);
      if(feof(fp)) sprintf(line,"quit ");
      if(strcmp(line,"")==0) {
        printf("Error File \"%s\" is invalid \n",arg);
        exit(-1);
      }
    }else{
      printf("Manousaridis_8855>");
      line =  read_line();
    }

    if(strlen(line)>MAX_BUF_SIZE)
    {
      printf("Buffer Overflow\n");
      continue;
    }

    history(strdup(line));

      char* temp=(char*)malloc(MAX_BUF_SIZE*sizeof(char));

      int k=0, i = 0, j=0 ,start=0,end=1;

      while(k<strlen(line))
      {
        if(line[i]==';' ||  line[i]=='&' || line[i]=='>' || line[i]=='<' ||  line[i]=='|')
        {
          temp[j]=' ';
          j++;
          while(line[i]==';' ||  line[i]=='&' || line[i]=='>' || line[i]=='<' ||  line[i]=='|')
          {
            temp[j]=line[i];
            i++;
            j++;
          }
          i--;
          temp[j]=' ';
        }
        else{
          temp[j]=line[i];
        }
        j++;
        i++;
        k++;
      }
      for(k=0; k<strlen(temp);k++)
      {
        line[k]=temp[k];
      }
      line[strlen(line)+1]=' ';

      free(temp);


      args =  parse_line(line);

      parse_symbol(args,"&><|;",symbol);
      i=0,j=0;
      int flag =0,SAVE_end,SAVE_start;
      while (args[i]!=NULL ){



        if(strcmp(args[i],"quit")==0)  exit(0);

        if(symbol[i]=='&' && flag==1){
          end=i-1;
          execute_pipe(args,start,end,SAVE_start,SAVE_end,status);
          flag=0;
          start = i+1;
          end = i+2;
          if(status[0]!=0) break;
        }
        else if(symbol[i]==';' && flag==1){
          end=i-1;
          execute_pipe(args,start,end,SAVE_start,SAVE_end,status);
          flag=0;
          start = i+1;
          end = i+2;
        }
        else if(symbol[i]=='&'){
          if(start<end-1) execute(args,start,end-1,status);
          start = i+1;
          end = i+2;
          if(status[0]!=0) break;
        }else if(symbol[i]==';'){
          if(start<end-1) execute(args,start,end-1,status);
          start = i+1;
          end = i+2;
        }else if(symbol[i]=='|'){
          //printf("Todo for BONUS\n");
          //break;
          flag=1;
          SAVE_start=start;
          SAVE_end=i-1;
          start=i+1;
          end=i+2;
        }else if(symbol[i]=='>'){
          greater(args,start,end-1,status,args[i+1],"w");
          start=i+2;
          end = i+3;
          i=i+1;
        }else if(symbol[i]=='a'){
          greater(args,start,end-1,status,args[i+1],"a");
          start=i+2;
          end = i+3;
          i=i+1;
        }else if(symbol[i]=='<'){
          less(args,start,end-1,status,args[i+1]);
          start=i+2;
          end = i+3;
          i=i+1;
        }else if (args[i+1]==NULL && args[i]!=NULL) {
          if(flag==0){
	        end++;
          execute(args,start,end,status);
        }else if(flag==1)
        {
          execute_pipe(args,start,i,SAVE_start,SAVE_end,status);
          flag=0;
          start = i+1;
          end = i+2;
          break;
        }
        }else if(symbol[i]=='E'){
          printf("Invalid syntax. Please check input again!\n");
          break;
        }
        else{
            end++;
        }
        i++;
    }


  } while (1);
  free(status);
  free(symbol);
  free(line);
  free(args);
  if(arg!=NULL) fclose(fp);
  return;
}

int main(int argc, char **argv)
{
  // Load config files, if any.

  // Run command loop
  if(argc==2)
    loop(argv[1]);
  else if(argc==1)
    loop(NULL);
  else{
    printf("%s\n", "Error with the given arguments");
    return -1;
  }
  // Perform any shutdown/cleanup.

  return 0;
}
