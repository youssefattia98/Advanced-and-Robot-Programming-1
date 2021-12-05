#include <stdio.h> 
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <ctype.h>

//Global varibales
int Xpos=0;
float Xesti_pos = 0;
float err=0;
char rec[80]="0";
char sen[80]= "0"; //output sting
char act[80]; //global variable stores which is token from the command
float pos=0;
int sign =0;




char senstr[100];
int motorzPID,motorxPID,commandPID,inspectionPID; 

void createfileZ(){
  FILE *fp;
  
   // Open file in write mode
   fp = fopen("./pidMotorZ","w+");

   // If file opened successfully, then write the string to file
   if ( fp )
   {
	   motorzPID=getpid();
       //printf("my pid is:%d\n",motorzPID);
       sprintf(senstr, "%d", motorzPID);
	   fputs(senstr,fp);
    }
   else
      {
         printf("Failed to open the file\n");
        }
//Close the file
   fclose(fp);
}

float generror(){

    /*
    This function returns a random number between 0 and 1 as float
    */
    float error;
    srand ( time(NULL) );
    return error = (double)rand() / (double)RAND_MAX ;
}

// keys 
char inc[] = "Inc";
char dec[] = "Dec";
char still[] = "Sti";
char reset[]="res";

void handle_sigusr2(int sig){
    strcpy(rec, reset);
}
int subadd(){
     srand ( time(NULL) );
    sign=rand()%2;
    return sign;
}

float motion(){
if(!strcmp(rec, inc))
{
            //should keep increasing until we reach Zpos=10
            if(Xpos<10)
            {
                
                    Xpos++;
                    err=generror();
                    sign = subadd();
                    err = err/2;
                    if(sign==0){
                        Xesti_pos=Xpos-err;
                    }
                    else{
                        Xesti_pos=Xpos+err;
                    }
                    
                    return Xesti_pos;
               
            }

            else
            {
                err=generror();
                Xesti_pos=Xpos;
                return Xesti_pos;
            }

}
if(!strcmp(rec, dec))
{
            //should keep decreasing until we reach Zpos=10
            if(Xpos>0)
            {
               
                    Xpos--;
                    err=generror();
                    sign = subadd();
                    err = err/2;
                    if(sign==0){
                        Xesti_pos=Xpos-err;
                    }
                    else{
                        Xesti_pos=Xpos+err;
                    }
                    return Xesti_pos;
                
                
            }

            else
            {
                err=generror();
                Xesti_pos=Xpos;
                return Xesti_pos;
            }
}
if(!strcmp(rec, still))
{
            err=generror();
            sign = subadd();
                    err = err/2;
                    if(sign==0){
                        Xesti_pos=Xpos-err;
                    }
                    else{
                        Xesti_pos=Xpos+err;
                    }
            return Xesti_pos;
}
if(!strcmp(rec, reset))
{
            err=0;
            Xpos=0;
            Xesti_pos=Xpos+err;
            return Xesti_pos;

}
}


int main(int argc, char * argv[]) 
{ 
    int fd2,fd3,watchdogPID; 

    char * commandZ = "/tmp/commandZ"; 
    char * inspectionz = "/tmp/inspectionz";
    mkfifo(inspectionz, 0666); 
    
    struct sigaction sig;
    memset(&sig, 0, sizeof(sig));
    sig.sa_handler= &handle_sigusr2;
    sigaction(SIGUSR2, &sig, NULL);


    
    
    while (1) 
    {
        createfileZ();
        fd2 = open(commandZ,O_RDONLY);
        fd_set rfds;
        struct timeval tv;
        int retval;
        FD_ZERO(&rfds);
        FD_SET(fd2, &rfds);
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        retval = select(fd2+1, &rfds, NULL, NULL, &tv);

        if (retval == -1){
            perror("error!");
        }
        else if (retval){
            /*
            data is now available
            */
           read(fd2, rec, 80);
           fd3 = open(inspectionz,O_WRONLY);
           if (rec[0] == 'q')
           {
               sen[0]='q';// should be chnaged
               write(fd3, sen, strlen(sen)+1);
               exit(EXIT_SUCCESS) ;
           }
           pos=motion();
           sprintf(sen, "%f", pos); //from string to float
    
           write(fd3, sen, strlen(sen)+1);
        }
        else{
            /*
            no data update
            */
           pos=motion();
           sprintf(sen, "%f", pos);
           fd3 = open(inspectionz,O_WRONLY);
           write(fd3, sen, strlen(sen)+1);
        }
        watchdogPID = atoi(argv[1]);
        kill(watchdogPID, SIGUSR1); //send a signal to the watchdog
    } 
    return 0; 
} 
