#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

void main()
{
    for (int n1=10; n1<1000001; n1=n1*10)
    {
        for (int n2=10; n2<1000001; n2=n2*10)
        {
            for (int n3=10; n3<1000001; n3=n3*10)
            {
                char n1_s[10];
                char n2_s[10];
                char n3_s[10];
                char quantum[10];
                sprintf(n1_s, "%d", n1);
                sprintf(n2_s, "%d", n2);
                sprintf(n3_s, "%d", n3);
                sprintf(quantum, "%d", 10000000);
                for(int i=0; i<2; ++i)
                {
                    if (!fork())
                    {
                        //printf("In child. Executing execlp...\n");
                        if(i==0)
                        {
                            if(execlp("./m", "./m", n1_s, n2_s, n3_s, "RR", "1m.txt", "1m_s.txt", quantum, NULL) == -1)
                            {
                                printf("Error in execlp\n");
                                exit(0);
                            }
                        }
                        else
                        {
                            if(execlp("./m", "./m", n1_s, n2_s, n3_s, "FCFS", "1m.txt", "1m_s.txt", NULL) == -1)
                            {
                                printf("Error in execlp\n");
                                exit(0);
                            }
                        }
                    }
                    else
                    {
                        //printf("In parent. Waiting for execlp...\n");
                        wait(NULL);
                    }
                }
            }
        }
    }
}