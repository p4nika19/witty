#include <codecvt>
#include <csignal>
#include <cstddef>
#include <iostream>
#include <cstdio>
#include <iterator>
#include <pty.h>
#include <sched.h>
#include <stdexcept>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <termios.h>
#include <cstdlib>
#include <fcntl.h>
#include <vector>

void runshell(){
    const char* shell = std::getenv("SHELL");  //get user shell name
    if (!shell){
        shell="/bin/sh";
    }
    execlp(shell, shell, nullptr);    //run shell
    perror("execlp failed");
    _exit(1);
}

class RawModeHandler {
    struct termios original_termios;
    bool active = false;

    public:
    void enable(){
        if (tcgetattr(STDIN_FILENO, &original_termios)==-1){
            throw std::runtime_error("tcgetattr error");
            return;
        }
        struct termios raw = original_termios;
        cfmakeraw(&original_termios);
        if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw)==-1){
            throw std::runtime_error("tcsetattr error");
        }
        active = true;
    }

    void disable(){
        if(active){
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
        }
    }
    ~RawModeHandler(){disable();}
};

class Pty {
    int master_fd = -1;
    pid_t procid = -1;
    public:
    Pty(){
        struct termios pty_termios;
        tcgetattr(STDIN_FILENO, &pty_termios);
        cfmakeraw(&pty_termios);
        procid = forkpty(&master_fd, nullptr, &pty_termios, nullptr);  //initializing pty
        if (procid < 0) {
            perror("forkpty failed");   //process error
            exit(1);
        }
        if (procid == 0) {
            runshell();
        }
        int flags = fcntl(master_fd, F_GETFL, 0);
        fcntl(master_fd, F_SETFL, flags | O_NONBLOCK);
    }
    int getfd() const { return master_fd; }

    ssize_t readfd(char *buffer, size_t size){
        return read(master_fd, buffer, size);
    }
    ssize_t writefd(const char *buffer, size_t size){
        return write(master_fd, buffer, size);
    }

    ~Pty(){
        if (master_fd != -1) { close(master_fd); }
        if (procid > 0) {
            kill(procid, SIGHUP);
            waitpid(procid, nullptr, 0);
        }
    }
};

class witty{
    RawModeHandler raw;
    Pty pty;
    bool running = true;
    std::vector<char> buffer;

    private:
    void pty_output(){
        ssize_t count = pty.readfd(buffer.data(), buffer.size());
        if (count <= 0) {
            running = false;
            return;
        }
        write(STDOUT_FILENO, buffer.data(), count);
    }
    void user_input(){
        ssize_t count = read(STDIN_FILENO, buffer.data(), buffer.size());
        if (count <= 0) {
            running = false;
            return;
        }
        pty.writefd(buffer.data(), count);
    }

    public:
    witty() : buffer(4096){
        raw.enable();
        std::cout << "welcome to witty <3" << std::endl;    //you're finally there!
    }

    void run(){
        fd_set read_fds;    //initializing fdset

        while(true){    //main cycle
            FD_ZERO(&read_fds); //clear existing fdset
            FD_SET(STDIN_FILENO, &read_fds);    //put standart input method to fdset
            FD_SET(pty.getfd(), &read_fds);   //put pty to fdset

            int max_fd = (pty.getfd() > STDIN_FILENO) ? pty.getfd() : STDIN_FILENO;
            int activity = select(max_fd + 1, &read_fds, nullptr, nullptr, nullptr);    //initializing select function with API required max pty number + 1 argument
            if(activity <= 0){
                perror("select error");
                break;
            }
            if (FD_ISSET(pty.getfd(), &read_fds)) {
                pty_output();
            }
            if (FD_ISSET(STDIN_FILENO, &read_fds)) {
                user_input();
            }
        }
    }
};

int main()
{
    witty app;
    app.run();
    return 0;
}
