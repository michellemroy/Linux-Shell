#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<signal.h>
#include<time.h>
//#include<ncursers.h>
#include <termios.h>
#include <unistd.h>


#define KOL "\x1B[36m\033[1m"
#define NOR "\x1B[37m\033[0m"
#define RED "\x1B[31m\033[1m"

#define cursorup(x) printf("\033[%dA", (x))
#define cursordown(x) printf("\033[%dB", (x))
#define cursorforward(x) printf("\033[%dC", (x))
#define cursorbackward(x) printf("\033[%dD", (x))

#define KEY_ESCAPE  0x001b
#define KEY_ENTER   0x000a
#define KEY_UP      0x0105
#define KEY_DOWN    0x0106
#define KEY_LEFT    0x0107
#define KEY_RIGHT   0x0108
#define KEY_BACK    0x0008

static struct termios term, oterm;

static int getch(void);
static int kbhit(void);
static int kbesc(void);
static int kbget(void);

int keyboardhit(void)
{
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}


void catch(int signo)
{
	printf("\n\n%stimer done!%s\n\n",RED,NOR);
	printf("press enter to exit timer");
}

static int getch(void)
{
    int c = 0;

    tcgetattr(0, &oterm);
    memcpy(&term, &oterm, sizeof(term));
    term.c_lflag &= ~(ICANON | ECHO);
    term.c_cc[VMIN] = 1;
    term.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &term);
    c = getchar();
    tcsetattr(0, TCSANOW, &oterm);
    return c;
}

static int kbhit(void)
{
    int c = 0;

    tcgetattr(0, &oterm);
    memcpy(&term, &oterm, sizeof(term));
    term.c_lflag &= ~(ICANON | ECHO);
    term.c_cc[VMIN] = 0;
    term.c_cc[VTIME] = 1;
    tcsetattr(0, TCSANOW, &term);
    c = getchar();
    tcsetattr(0, TCSANOW, &oterm);
    if (c != -1) ungetc(c, stdin);
    return ((c != -1) ? 1 : 0);
}

static int kbesc(void)
{
    int c;

    if (!kbhit()) return KEY_ESCAPE;
    c = getch();
	
    if (c == '[') {
        switch (getch()) {
            case 'A':
                c = KEY_UP;
                break;
            case 'B':
                c = KEY_DOWN;
                break;
            case 'C':
                c = KEY_LEFT;
                break;
            case 'D':
                c = KEY_RIGHT;
                break;
            default:
                c = 0;
                break;
        }
    } else {
        c = 0;
    }
    if (c == 0) while (kbhit()) getch();
	//if(c == '?')
	//	return KEY_BACK;
    return c;
}

static int kbget(void)
{
    int c;

    c = getch();
	//printf("|%d|",c);
	if(c == 127)
		return KEY_BACK;
	if(c == 4)
		return -1;
    return (c == KEY_ESCAPE) ? kbesc() : c;
}
int main(int argc, char** argv)
{
	char *buf,command[1024],junk,cwd[1024]; pid_t pid;
	char *args[100];
	int fd,fd2;
	char *hist[25];
	int hp = 0;
	char *alias[] ={"la","ld","rsort","caps","cls","udate","day","dus","lz"};
	char *canon[] = {"ls -al","ls -d","sort -r","tr 'a-z' 'A-Z'","clear","date -u","date +%A","du -s","find -size 0"};

	
	
	
	int op = dup(1),ip = dup(0);
	int opredir = 0, ipredir = 0;

	while(1)
	{
		int pd[2];
		opredir=0;
		ipredir = 0;
		
		buf = malloc(sizeof(char)*1024);
		getcwd(cwd, sizeof(cwd));
		dup2(ip,STDIN_FILENO);
		op = dup(1);
		ip = dup(0);
		printf("\n");
		printf("%smash:~%s",KOL,cwd);
		printf("%s$ ",NOR);
		
		system("/bin/stty raw");
		int arrow  = 1;
		int cp = hp;
		while(arrow)
		{
		
		int a  = getchar();
		if(a == 13)
			{
			break;}
		if(a==27)
			{
			getchar();
			int d = getchar();
			if(d == 65)
			{
			cp--;
			if(cp<0)
				cp = 0;
			}
			else if(d == 66)
			{
			cp++;
			if(cp>hp)
				cp = hp;
			}
			//fflush(stdout);
			printf("\r\33[2K%smash:~%s",KOL,cwd);
			printf("%s$ ",NOR);
			printf("%s",hist[cp]);
			if (hist[cp] != NULL)
			buf = strdup(hist[cp]);
			
			}
		else
			{buf[0] = a;
			arrow = 0;}
		}
		system("/bin/stty cooked");
		if(arrow == 0){
			scanf("%[^\n]",buf+1);
			getchar();
		}
		printf("%s\n",NOR);
		hist[hp] = strdup(buf);
		hp++;
		if(hp == 25)
			hp = 0;
		int ct = 0;
		int j = 0;
		int len = strlen(buf);

		//printf("len = %d\n",len);
		if(len ==0)
			{dup2(ip,STDIN_FILENO);
			continue;}
		int flag = 1;
		int k = 0;
		while(flag)
		{
		opredir=0;
		ipredir = 0;
		pipe(pd);
		//dup2(pd[0],0);
		//dup2(pd[1],1);
		for (k;k<=len;k++){
			//printf("%d",strlen(buf));
			if(k == len)
				{
				flag = 0;
				//printf("end");
				//dup2(op,STDOUT_FILENO);
				//dup2(ip,STDIN_FILENO);
				}
			//printf("|%c|\n",buf[j]);  
			if(buf[j] == 32 || buf[j] == 9 || buf[j] == '\0' || buf[j] == 124 || buf[j] == 42 )
			{

				
				if(buf[j] == '\0')
					flag = 0;
				
				if(j != 0)
				{
				strncpy(command,buf,j);
				command[j] = '\0';
				//printf("\n%s\n",command);

				int fl = 0;
				for(int l=0;l<9;l++)
				{
					if(strcmp(command,alias[l])==0)
						{
						//printf("\ninside alias check\n");
						char *temp = malloc(sizeof(char)*1024);
						strcpy(temp,canon[l]);
						char *temp2 = malloc(sizeof(char)*1024);
						temp2 = strdup(buf);
						temp2 += j;
						strcat(temp, temp2);
						buf = strdup(temp);
						//printf("buf  = %s\n",buf);
						len = len + strlen(canon[l]);
						fl = 1; 
						}
						
				}
				
				if(buf[j] == 42)
				{
					
					buf+=j;
					char *tempz = malloc(sizeof(char)*1024);
					char wild[] = "| grep ";
					tempz = strcat(wild,command);
					//printf("%s\n",tempz);
					char *temp = malloc(sizeof(char)*1024);
					temp = strdup(buf);
					temp = strcat(temp,tempz);
					buf = strdup(temp);
					len = len + 6;
					//printf("buf  = %s, temp = %s, tempz = %s\n",buf,temp,tempz);
					fl = 1;
					
				}
				
				
				if(fl == 0)
				{
				args[ct] = strdup(command);
				//printf("\narg ct:%s\n",args[ct]);
				
				ct++;
				if(buf[j] == 124){
					//printf("pipe\n");
					break;

					}
				buf+=j+1;
				}
				}
			else
				buf++;
			j = 0;
			if(buf[j] == 124){
					//printf("pipe\n");
					break;
					}
				
			}


			else
				j++;
		}

		if(buf[0]!=32 && buf[0]!=NULL && buf[0]!=9 && buf[0]!=124 )
		{args[ct] = strdup(buf);
		ct++;
		//flag = 1;
		}
		args[ct] = NULL;

		//printf("ct = %d",ct);
		//ct = 0;
		
		
		
		
		
		for(int i=0;i<ct;i++)
		{
		
			if(strcmp(args[i],">")==0)
			{
			
				opredir = 1;
				//printf("Output redirect");
				op = dup(1);
				fd = open(args[i+1],O_CREAT|O_RDWR|O_TRUNC,S_IRWXU|S_IRWXG|S_IRWXO);
				fd = dup2(fd,1);
				ct-=2;
				args[i] = NULL;
				
				
			}
			
			else if(strcmp(args[i],"<")==0)
			{	
				ipredir = 1;
				//printf("input redirect");
				ip = dup(0);
				fd = open(args[i+1],O_CREAT|O_RDONLY,S_IRWXU|S_IRWXG|S_IRWXO);
				fd = dup2(fd,0);
				ct-=2;
				args[i] = NULL;
				
				
			}
	
			
		
		}
		
		ct = 0;

		//char*ar = args+1;
		if(strcmp(args[0],"timer")==0)
		{
			int tpid = fork();
			int s;
			
			if(args[2]!=NULL&&strcmp(args[2],"m")==0)
				s = (int)atof(args[1])*60;
			else
				s = atoi(args[1]);
			//printf("%d",atoi(args[1]));
			
			if(tpid==0)
			{
				signal(SIGALRM,catch);				
				alarm(s);
				sleep(s+1);exit(0);
			}
			//signal(SIGALRM,catch);
			continue;
			
		}

		if(strcmp(args[0],"stpw")==0)
		{
			int ms=0,h=0,s=0,m=0;
			char nut;
			//int spid = fork();
			//printf("stopwatch");
			
			//if(spid ==0)
			time_t initial, final;
			int sub;
			time(&initial);
			
				while(!keyboardhit())
				{
					time(&final);
					sub = (int)(final - initial);
					printf("\r\33[2K%d:%d:%d",sub/3600,(sub/60)%60,sub%60);
					fflush(stdout);
					usleep(600000);
				}
				nut = getchar();

			
			continue;
			
				
			
		}

		
		if(strcmp(args[0],"cd")==0)
			{
				chdir(args[1]);
				continue;
			}
			
		if(strcmp(args[0],"edit")==0)
			{
				//printf("%s\n",args[1]);
				
				//int c;
				FILE* fp;
				char c;
				char a[1000][1000];
				int i=0,j=-1;
				//char *args
				
				if(access(args[1],F_OK)!=-1)
				{
				fp = fopen(args[1],"r+");
				
				while((c = fgetc(fp)) != EOF)
				{
				j++;
				if(c == 10)
					{i++;
					j = -1;}
				printf("%c",c);
				}
				fclose(fp);
				}

				
				fp = fopen(args[1],"w+");


				    while (1) {
					
					//printf("[%d %d]",i,j);
					c = kbget();
					//printf("|%d|",c);
					if(c == -1)
					{j++;
					a[i][j] = EOF;
 
						break;
						}
					if (c == KEY_ENTER || c == KEY_ESCAPE ) {
					j++;
					a[i][j] = c;
					i++;
					j = -1;
					    putchar(c);
					
						
						//break;
					} else
					if (c == KEY_BACK) {
						a[i][j] = ' ';
						j--;
						if(j<0)
							{
							i--;
							while(a[i][j]!='\n')
								j--;
							}
					    printf("\b \b");
						//fflush(stdout);
					}else
					if (c == 8) {
						j--;
					    cursorbackward(1);
					} else
					if (c == 7) {
						j++;
					    cursorforward(1);
					} else if (c == 5) {
					//printf("UP");
						i--;
					    cursorup(1);
					}else if (c == 6) {
					i++;
					    cursordown(1);
					}else{
						j++;
						a[i][j] = c;
					    putchar(c);
					}
				    }
				    
				   
				for(int i=0;i<1000;i++)
				{int j;
					for(j=0;j<1000;j++)
					{
						if(a[i][j] == EOF)
							break;
						fprintf(fp,"%c",a[i][j]);
						if(a[i][j] == '\n')
							break;
						
					}
				if(a[i][j] == EOF)
					break;	
				}
						
				
				fclose(fp);
				
				continue;
			}
		
		//printf("reached here\n");
		//op = dup(1);
		//ip = dup(0);
		
		
				
		pid=fork();
		if(pid== -1)
		{
			printf("\nCouldn't execute fork :(\n");
			//continue;
		}
		else if(pid == 0)
		{
			
			//for(int i=0;i<ct;i++)
				//printf("args = %s ",args[i]);
			
			if(opredir == 0)
				dup2(pd[1],1);
			
			if(flag == 0 && opredir==0)
				dup2(op,STDOUT_FILENO);
				

			//printf("FLAG = %d\n",flag);
			

			if(execvp(args[0],args) < 0)
				printf("Cannot execute %s : Invalid command\n",args[0]);


			
			exit(0);
		}

		else if(pid>0)
		{
		wait(0);
		
		
		dup2(pd[0],0);
		close(pd[1]);

		
		//printf("FLAGOut = %d\n",flag);

		if(flag == 0)
		{
			if(ipredir == 0)
				dup2(ip,STDIN_FILENO);
			dup2(op,STDOUT_FILENO);
			//printf("ip = %d\n",ip);
		}
		
		//printf("executed command\n");
		}
	}		

	}
	
} 
