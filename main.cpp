#include <cstdio>
#include <fstream>
#include <iostream>
#include <pty.h>
#include <sched.h>
#include <unistd.h>
#include <sys/wait.h>
#include <termios.h>
#include <cstdlib>

int main()
{
    int master_fd;
    pid_t procid = forkpty(&master_fd, nullptr, nullptr, nullptr);  //initializing pty

    if (procid < 0) {
        perror("forkpty failed");   //process error
    }

    if (procid == 0) {
        const char* shell_path = std::getenv("SHELL");  //get user shell name
        if (!shell_path){
            shell_path="/bin/sh";
        }
        execlp(shell_path, shell_path, nullptr);    //run shell
    }
    return 0;
}
