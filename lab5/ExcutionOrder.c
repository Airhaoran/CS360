main()
     {
        int pid;
        pid=fork();
        if (pid){ // PARENT
           printf("PARENT %d DYING\n", getpid());
        }
        else{ // child
               printf("child %d sleeps for 2 seconds\n",getpid());
               sleep(2); // sleep for 2 seconds ==> PARENT DIES FIRST
               printf("child %d my parent=%d\n", getpid(), getppid());
        }
     }

