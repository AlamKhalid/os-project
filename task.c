#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct proc
{
    int pid;
    char user[50], cmd[100];
    float cpu, mem;
};

struct proc process;
int wrote = 0;

void fileOutput()
{
    FILE *fp;
    if (wrote) {
        fp = fopen("proc.txt", "a");
    } else {
        fp = fopen("proc.txt", "w");
    }
    fprintf(fp, "%d %s %.1f %.1f %s\n", process.pid, process.user, process.cpu, process.mem, process.cmd);
    fclose(fp);
}

int getAllProc()
{
    FILE *fp;
    int i = 0, tok_num = 0, run = 1;
    char output[500], *token;
    fp = popen("ps au", "r");
    while (fgets(output, sizeof(output), fp))
    {
        if (!i)
        {
            i++;
            continue;
        }
        tok_num = 0;
        run = 1;
        token = strtok(output, " ");
        while (token && run)
        {
            switch (tok_num)
            {
            case 0:
                strcpy(process.user, token);
                break;
            case 1:
                process.pid = atoi(token);
                break;
            case 2:
                process.cpu = atof(token);
                break;
            case 3:
                process.mem = atof(token);
                break;
            case 10:
                if (token[strlen(token) - 1] == '\n')
                {
                    token[strlen(token) - 1] = '\0';
                }
                strcpy(process.cmd, token);
                break;
            case 11:
                run = 0;
                break;
            }
            ++tok_num;
            token = strtok(NULL, " ");
        }
        fileOutput();
        wrote = 1;
    }
    pclose(fp);
}

void checkFile(char *p) {
    FILE *fptr;
    fptr = fopen("out.txt", "r");
    fscanf(fptr, "%[^\n]", p);
    fclose(fptr);
}

void clearFile() {
    FILE *fptr;
    fptr = fopen("out.txt", "w");
    fprintf(fptr, "0");
    fclose(fptr);
}

int main(void)
{
    char kill_cmd[15], pid[10];
    while(1) {
        getAllProc();
        wrote = 0;
        sleep(2);
        checkFile(pid);
        if(pid[0] != '0') {
            strcpy(kill_cmd, "kill -9 ");
            strcat(kill_cmd, pid);
            system(kill_cmd);
            clearFile();
        }
    }
    return 0;
}