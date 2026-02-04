#include <codecvt>
#include <iostream>
#include <cstdio>
#include <iterator>
#include <pty.h>
#include <sched.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <termios.h>
#include <cstdlib>

int main()
{
    int master_fd;
    char buffer[4096];  //creating buffer

    pid_t procid = forkpty(&master_fd, nullptr, nullptr, nullptr);  //initializing pty

    if (procid < 0) {
        perror("forkpty failed");   //process error
        return 1;
    }

    if (procid == 0) {
        const char* shell = std::getenv("SHELL");  //get user shell name
        if (!shell){
            shell="/bin/sh";
        }
        execlp(shell, shell, nullptr);    //run shell
        return 1;
    }

    std::cout << "welcome to witty <3" << std::endl;    //you're finally there!
    fd_set read_fds;    //initializing fdset

    while(true){    //main cycle
        FD_ZERO(&read_fds); //clear existing fdset
        FD_SET(STDIN_FILENO, &read_fds);    //put standart input method to fdset
        FD_SET(master_fd, &read_fds);   //put pty to fdset

        int max_fd = (master_fd > STDIN_FILENO) ? master_fd : STDIN_FILENO;
        int activity = select(max_fd + 1, &read_fds, nullptr, nullptr, nullptr);    //initializing select function with API required max pty number + 1 argument
        if(activity <= 0){
            perror("select error");
            break;
        }

        if(FD_ISSET(master_fd, &read_fds)){ //catching bash output
            ssize_t bytes_read = read(master_fd, buffer, sizeof(buffer));
            if (bytes_read < 0) break;
            write(STDOUT_FILENO, buffer, bytes_read);
        }

        if(FD_ISSET(master_fd, &read_fds))  {   //catching pty input
            ssize_t bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer));
            if (bytes_read <= 0) break;
            write(master_fd, buffer, bytes_read);
        }
    }

    close(master_fd);
    wait(nullptr);
    return 0;
}
