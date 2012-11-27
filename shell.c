/*****

AUTHOR: PRATEEK SACHDEV
ROLL NO: 201101015
OBJECTIVE: TO CREATE AN INTERACTIVE BASH LIKE SHELL

 *****/



/********** STANDARD INCLUDES*******/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<pwd.h>

/*********************************************************************************************
  					DATA STRUCTURE TO STORE HISTORY
 ********************************************************************************************/
typedef struct node
{
	char process[50];
	long pid;
	int background;

}node;
void historycommand();
void histn(int);
/*******************************************************GLOBAL VARIABLES*******************************8***/
char host[20];
char *user;
char *cwd; 
int count=0;
int piping=0;
node *history[1000];
/**************************************************************************************************
  							SIGNAL HANDLERS
 *************************************************************************************************/
void handle_signal(int signo)
{
	printf("\n");
	printf("%s@%s:%s>",user,host,cwd);
	fflush(stdout);
}
void child_handler(int signum)
{
	int p;
	int i;
	p = waitpid(WAIT_ANY, NULL, WNOHANG);
	int index=0;
	for(i=0;i<1000;i++)
	{
		if(history[i]->pid!=0)
		{	if(history[i]->pid==p)
			{
				index=i;
				history[i]->background=0;
			}
		}
	}
	if(index<20 && p>0 && (piping ==0))
	{printf("\nprocess %s with pid: %d exited successfully\n",history[index]->process,p);
		printf("%s@%s:%s>",user,host,cwd);
		fflush(stdout);
	}
	signal(SIGCHLD, child_handler);
	return;

}

/********************************************MAIN FUNCTION******************************************/
int main()
{
	int i, fd;
	for(i=0;i<1000;i++)
	{
		history[i]=(node*)malloc(sizeof(node));
		history[count]->background=0;
		history[i]->pid=0;
	}
	/****
	  HANDLING child exit SIGNALS
	 *****/
	signal(SIGCHLD,SIG_IGN); 
	signal(SIGCHLD, child_handler);

	/****
	  HANDLING CTRL+C and SIGINT SIGNALS
	 *****/

	signal(SIGTSTP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGINT, handle_signal);

	signal(SIGPIPE,SIG_IGN); 


	int outpid=getpid();

	char c;


	/******* getting host name *********/
	gethostname(host,20);


	/*******getting username *********/
	register struct passwd *pw;
	register uid_t uid;
	uid = geteuid ();
	pw = getpwuid (uid);
	if (pw)
	{
		user=pw->pw_name;
	}
	char *tilda=getenv("PWD");
	chdir(tilda);

	/**************get current working directory***********/
	cwd = (char *)malloc(100);
	strcpy(cwd,"~");

	char *arg[2]={"clear",NULL};
	if(fork() == 0) {
		execvp("clear", arg);
		_exit(1);
	} else {
		wait(NULL);
	}
	printf("%s@%s:%s>",user,host,cwd);
	fflush(stdout);
	int input=1;
	int hist_exec=0;
	while(1)
	{
		char s[100];
		int pipecount=0;
		int flag=0;
		char *x;
		int ind=0;
		int index[50];
		char *arguments[100];
		int redirectionin=0;
		int redirectionout=0;
		if(hist_exec==0)
		{
			strcpy(s,"\0");

			if(input==1)
				scanf("%[^\n]%*c",s);
		}
		strcpy(history[count]->process,s);
/**********************************************************************************************************
  						HANDLING ZOMBIE PROCESS
 **********************************************************************************************************/
		if(strcmp(s,"quit")==0)
		{	for(i=0;i<100;i++)
			{
				if(history[i]->pid!=0 && history[i]->background==1)
				{
					kill(history[i]->pid,SIGQUIT);


				}
			}
			break;
		}
		hist_exec=0;
		if(strcmp(s,"\0")==0)
		{
			printf("%s@%s:%s>",user,host,cwd);
			scanf("%c",&c);
			continue;
		}
		for(i=0;i<strlen(s)-1;i++)
		{
			if(s[i]=='|' &&( (i-1)>=0) && s[i-1]==' '  && s[i+1]==' ')
				pipecount++;
			if(s[i]=='<' &&( (i-1)>=0) && s[i-1]==' '  && s[i+1]==' ')
				redirectionin++;
			if(s[i]=='>' &&( (i-1)>=0) && s[i-1]==' '  && s[i+1]==' ')
			{
				redirectionout++;
			}
		}
		arguments[0]=strtok(s," ");
		char *command="cd";
		char *command_pid="pid";
		char *command_all="all";
		char *command_current="current";
		char *command_back="&";
		char *command_hist="hist";
		char *command_histn="histn";
		ind++;
		while(x=strtok(NULL," "))
		{
			if(strcmp(x,"|")==0 || strcmp(x,"<")==0 || strcmp(x,">")==0) 
			{
				break;
			}
			arguments[ind]=x;
			ind++;
		}	
		arguments[ind]=NULL;
/*****************************************************************************************************************************
		  					 cd commands
******************************************************************************************************************************/
		if(strcmp(arguments[0],command)==0)
		{
			history[count]->pid=4;//for cd commands
			count+=1;
			if(ind==1 || strcmp(arguments[1],"~")==0 || strcmp(arguments[1],"~/")==0)
			{
				strcpy(cwd,"~");
				chdir(tilda);
			}
			else
			{
				if(strcmp(arguments[1],"..")!=0 && strcmp(arguments[1],".")!=0)
				{
					if(chdir(arguments[1])!=-1)
					{
						strcat(cwd,"/");
						strcat(cwd,arguments[1]);
					}
					else
						printf("No such file or directory\n");
				}
				else if (strcmp(arguments[1],"..")==0)
				{
					chdir(arguments[1]);
					for(i=strlen(cwd)-1;i>0;i--)
					{
						if(cwd[i]=='/')
						{
							cwd[i]='\0';
							break;
						}
						else
							cwd[i]='\0';
					}
				}

			}
			printf("%s@%s:%s>",user,host,cwd);
			fflush(stdout);
			input=1;
		}
/*****************************************************************************************************************************
		  					 user commands
******************************************************************************************************************************/
		else if(strcmp(arguments[0],command_pid)==0)
		{

			if(ind==1)
			{
				printf("command name: ./a.out process id: %d\n",outpid);
			}
			else if(ind==2 )
			{
				if(strcmp(arguments[1],command_all)==0)
				{
					printf("List of all processes spawned from this shell:\n");
					for(i=0;i<1000;i++)
					{
						if(history[i]->pid!=0)
						{

							printf("command name: %s ",history[i]->process);
							printf("process id :%ld\n",history[i]->pid);
						}
					}
				}
				if(strcmp(arguments[1],command_current)==0)
				{
					printf("List of currently executing processes spawned from this shell:\n");
					for(i=0;i<100;i++)
					{
						if(history[i]->pid!=0 && history[i]->background==1)
						{

							printf("command name: %s ",history[i]->process);
							printf("process id :%ld\n",history[i]->pid);
						}
					}
				}

			}
			printf("%s@%s:%s>",user,host,cwd);
			fflush(stdout);
			input=1;
		}
/*****************************************************************************************************************************
		  					 history commands
******************************************************************************************************************************/
		else if(strcmp(arguments[0],command_hist)==0)
		{	
			input=1;
			historycommand();
			history[count]->pid=5;//5 for hostory commands
			strcpy(history[count]->process,command_hist);
			count+=1;


		}
		else if(arguments[0][0]=='h' && arguments[0][1]=='i' && arguments[0][2]=='s' && arguments[0][3]=='t' && arguments[0][4]!='\0')
		{
			int value=0;
			int i=4;
			while(arguments[0][i]!='\0')
			{
				value=value*10 + (int)arguments[0][i]-(int)'0';
				i++;
			}
			histn(value);	
			input=1;
			//printf("%d",value);

		}
		else if(arguments[0][0]=='!' && arguments[0][1]=='h' && arguments[0][2]=='i' && arguments[0][3]=='s' && arguments[0][4]=='t'&& arguments[0][5]!='\0')
		{
			hist_exec=1;
			int value=0;
			int i=5;
			while(arguments[0][i]!='\0')
			{
				value=value*10 + (int)arguments[0][i]-(int)'0';
				i++;
			}
	int counter=1;
			for(i=0;i<1000;i++)
			{
				if(history[i]->pid!=0 )
				{
					if(counter==value)
					{
						printf("%s\n",history[i]->process);
						input=0;
						strcpy(s,history[i]->process);
						break;
					}
					counter+=1;
				}

			}


		}

		else
		{
			int in, out;
/*****************************************************************************************************************************
		  					 only '>' operation
******************************************************************************************************************************/
			if(redirectionin ==0 && redirectionout!=0 && pipecount ==0)
			{
				char* arguments2[100];
				ind=0;
				while(x=strtok(NULL," "))
				{
					if(strcmp(x,"|")==0 || strcmp(x,">")==0)
					{
						break;
					}
					arguments2[ind]=x;
					ind++;
				}
				arguments2[ind]=NULL;

				pid_t pid;
				pid=fork();
				if (pid == -1)
				{
					perror("Error forking...");
				}
				else if (pid==0)  //ls
				{
					out = open(arguments2[0], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
					dup2(out,1);
					close(out);
					if(execvp(arguments[0],arguments)!=0)
						printf("command not found");
					_exit(0);

				}
				else   /*parent process*/
				{
					wait(NULL);//waits till the child send output to pipe
					history[count]->pid=pid;
					count=count+1;
					printf("%s@%s:%s>",user,host,cwd);
					fflush(stdout);
				}


			}
/*****************************************************************************************************************************
		  					 only '<' operation
******************************************************************************************************************************/
			/******************only '<' operation*********************/
			else if(redirectionin !=0 && redirectionout==0 && pipecount==0)
			{
				//	printf("< operation\n");
				char* arguments2[100];
				ind=0;
				while(x=strtok(NULL," "))
				{
					if(strcmp(x,"|")==0 || strcmp(x,">")==0)
					{
						break;
					}
					arguments2[ind]=x;
					ind++;
				}
				arguments2[ind]=NULL;
				if((in = open(arguments2[0], O_RDONLY)<0))
				{
					printf("invlid input file\n");
					printf("%s@%s:%s>",user,host,cwd);
					fflush(stdout);
				}
				else
				{

					pid_t pid;
					pid=fork();
					if (pid == -1)
					{
						perror("Error forking...");
					}
					else if (pid==0)  //ls
					{
						in = open(arguments2[0], O_RDONLY);
						dup2(in,0);
						close(in);
						if(execvp(arguments[0],arguments)!=0)
							printf("command not found");
						_exit(0);

					}
					else   /*parent process*/
					{
						history[count]->pid=pid;
						count=count+1;
						wait(NULL);//waits till the child send output to pipe
						printf("%s@%s:%s>",user,host,cwd);
						fflush(stdout);
					}
				}
			}
/*****************************************************************************************************************************
		  					'<' and '>' operations without piping 
 ******************************************************************************************************************************/
			else if (redirectionin !=0 && redirectionout!=0 && pipecount==0)
			{
				char* arguments2[100];
				char* arguments3[100];
				piping=1;
				ind=0;
				int fds[2];
				pipe(fds);
				while(x=strtok(NULL," "))
				{
					if(strcmp(x,"|")==0 || strcmp(x,">")==0)
					{
						break;
					}
					arguments2[ind]=x;
					ind++;
				}
				arguments2[ind]=NULL;
				ind=0;
				while(x=strtok(NULL," "))
				{
					if(strcmp(x,"|")==0 || strcmp(x,">")==0)
					{
						break;
					}
					arguments3[ind]=x;
					ind++;
				}
				arguments3[ind]=NULL;
				pid_t pid;
				pid=fork();
				if (pid == -1)
				{
					perror("Error forking...");
				}
				else if (pid==0)  //ls
				{
					in = open(arguments2[0], O_RDONLY);
					dup2(in,0);
					dup2(fds[1],1);
					close(fds[1]);
					if(execvp(arguments[0],arguments)!=0)
						printf("command not found");

				}
				else   /*parent process*/
				{
					piping=1;

					wait(NULL);//waits till the child send output to pipe
					history[count]->pid=pid;
					count=count+1;
					pid_t newpid;
					newpid=fork();
					if(newpid==0)
					{
						dup2(fds[0],0);	
						close(fds[0]);
						close(fds[1]);
						out = open(arguments3[0], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
						dup2(out,1);
						close(out);
						if(execvp(arguments[0],arguments)!=0)
							printf("command not found");
						_exit(0);

					}
					else
					{
						close(fds[1]);
						close(fds[0]);
						printf("%s@%s:%s>",user,host,cwd);
						fflush(stdout);
					}
				}
			}
/*****************************************************************************************************************************
 					all others including '<' and '>' operations with piping
 ******************************************************************************************************************************/
			else// for ever other operation (jst piping or piping + redirection)
			{
				char* arguments2[100];
				for(i=0;i<ind;i++)
				{
					if(strcmp(arguments[i],command_back)==0)
					{
						arguments[i]=NULL;
						flag=1;

					}
				}
				if(redirectionin!=0)
				{
					ind=0;
					while(x=strtok(NULL," "))
					{
						if(strcmp(x,"|")==0 || strcmp(x,">")==0)
						{
							break;
						}
						arguments2[ind]=x;
						ind++;
					}
					arguments2[ind]=NULL;
					in = open(arguments2[0], O_RDONLY);

				}

				int executed=0;
				int fd[pipecount][2];
				for(i=0;i<pipecount;i++)
					pipe(fd[i]);
				pid_t pp;
				pp=fork();
				if (pp == -1)
				{
					perror("Error forking...");
				}
				else if (pp==0)  
				{
					if(redirectionin!=0)
					{
						dup2(in,0);
						close(in);
					}

					close(fd[executed][0]);   /*Closes read side of pipe*/
					if(pipecount>=1)
					{dup2(fd[executed][1],1);
						close(fd[executed][1]);}
					if(execvp(arguments[0],arguments)!=0)
						printf("command not found\n");
					_exit(0);

				}
				else   /*parent process*/
				{

					if(redirectionin!=0)
					{
						close(in);
						redirectionin--;
					}
					history[count]->pid=pp;
					count=count+1;
					if(flag==0)
						wait(&pp);        //waits till the child send output to pipe
					if(flag==1)
					{
						piping=0;
						history[count]->background=1;
					}
					if(pipecount!=0)
					{
						while(pipecount)
						{
							ind=0;
							while(x=strtok(NULL," "))
							{
								if(strcmp(x,"|")==0 || strcmp(x,">")==0)
								{
									break;
								}
								arguments2[ind]=x;
								ind++;
							}
							arguments2[ind]=NULL;

							if(pipecount==1 && redirectionout !=0)
							{
								char* arguments3[100];
								ind=0;
								while(x=strtok(NULL," "))
								{
									arguments3[ind]=x;
									ind++;
								}
								arguments3[ind]=NULL;
								out = open(arguments3[0], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
								pid_t pid;
								close(fd[executed][1]);
								pid=fork();
								if (pid == -1)
								{
									perror("Error forking...");
								}
								else if (pid==0)  //ls
								{
									close(fd[executed][1]);
									dup2(fd[executed][0],0);
									close(fd[executed][0]);
									dup2(out,1);
									close(out);
									if(execvp(arguments2[0],arguments2)!=0)
										printf("command not found");
									_exit(0);

								}
								else   /*parent process*/
								{


									wait(NULL);//waits till the child send output to pipe
									close(fd[executed][0]);
									close(out);
									redirectionout--;
									pipecount--;
									fflush(stdout);
								}


							}
							else
							{

								pid_t pid;
								close(fd[executed][1]);
								pid=fork();
								if (pid == -1)
								{
									perror("Error forking...");
								}
								else if (pid==0)  
								{
									close(0);       //stdin closed
									dup2(fd[executed][0],0);
									close(fd[executed][1]);
									if(pipecount>=1 )
										dup2(fd[executed+1][1],1);
									close(fd[executed+1][1]);
									if(execvp(arguments2[0],arguments2)!=0)
										printf("command not found");
									_exit(0);

								}
								else   /*parent process*/
								{
									wait(NULL);//waits till the child send output to pipe
									close(fd[executed][0]);
									if(pipecount!=1)
										close(fd[executed+1][1]);
									executed++;
									pipecount--;
									continue;
								}
							}
						}
					}
					printf("%s@%s:%s>",user,host,cwd);
					fflush(stdout);

				}

			}

			input=1;
		}


	}
	return 0;
}
void historycommand()
{
	int i;
	int counter=1;
			for(i=0;i<1000;i++)
			{
				if(history[i]->pid!=0)
				{
					printf("%d. %s\n",counter,history[i]->process);
					counter+=1;
				}

			}
			printf("%s@%s:%s>",user,host,cwd);//printing the prompt
}
void histn(int value)
{
	int i;	
	int counter=1;
	for(i=999;i>=0;i--)
	{
		if(history[i]->pid!=0 && counter<=value)
		{
			printf("%d. %s\n",counter,history[i]->process);
			counter+=1;
		}

	}
	history[count]->pid=5;
	count+=1;
	printf("%s@%s:%s>",user,host,cwd);//printing the prompt
}
