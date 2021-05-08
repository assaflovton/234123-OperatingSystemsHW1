#include <iostream>
#include <cstdlib>
#include <sys/wait.h>
#include <signal.h>
#include "Commands.h"
#include "signals.h"
#include <string.h>
int main(int argc, char* argv[]) {
    if(signal(SIGTSTP , ctrlZHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-Z handler");
    }
    if(signal(SIGINT , ctrlCHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-C handler");
    }
    SmallShell& smash = SmallShell::getInstance();
    struct sigaction new_action;
    new_action.sa_handler = alarmHandler;
    new_action.sa_flags =SA_RESTART;
    if(sigaction(SIGALRM, &new_action, NULL)<0){
        perror("smash error: sigaction failed");
    }
    while(true) {
        std::cout <<smash.sprompt<<"> ";
        std::string cmd_line;
        (std::getline(std::cin, cmd_line));
        if(cmd_line.empty()){
            continue;
        }
        smash.jobs_list->removeFinishedJobs();
        smash.executeCommand(cmd_line.c_str());
        
    }
    return 0;
}