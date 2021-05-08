#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"
#include <unistd.h>

using namespace std;
void ctrlZHandler(int sig_num) {
    std::cout<<"smash: got ctrl-Z\n";
    JobsList::JobEntry* job = (SmallShell::getInstance().cur_job);
    if(job==nullptr){
      return;
    }
    job->job_status=STOPPED;
    pid_t pid =job->pid;
    if(kill(pid,SIGSTOP)<0){
      perror("smash error: kill failed");
    }
    (SmallShell::getInstance().jobs_list)->job_vec.push_back(*job);
    std::cout<<"smash: process "<<pid<< " was stopped\n";
}
void ctrlCHandler(int sig_num) {
  std::cout<<"smash: got ctrl-C\n";
  JobsList::JobEntry* job = (SmallShell::getInstance().cur_job);
  if(job==nullptr){
    return;
  }
  job->job_status=FINISHED;
  if(kill(job->pid,SIGKILL)<0){
      perror("smash error: kill failed");
  }
  std::cout<<"smash: process "<<job->pid<< " was killed\n";
}
void alarmHandler(int sig_num) {
    SmallShell* smash_inst =&(SmallShell::getInstance());
    TimeEntry* time_entry = smash_inst->getTimeoutsList()->findByTime(time(nullptr));
    if(time_entry==nullptr){
        return;
    }
    cout << "smash: got an alarm"<<endl;
    cout << "smash: timeout " << time_entry->curr_dur << " "<<time_entry->_cmd<<" timed out!"<< endl;
    if(time_entry->pid!=getpid()){
        if(kill(time_entry->pid , SIGKILL) <0){
            perror("smash error: kill failed");
            return;
        }
    }
    smash_inst->getTimeoutsList()->removeTimeoutByPid(time_entry->pid);
    if(smash_inst->getTimeoutsList()->getSize()!=0)
    {
        unsigned int new_timeout = smash_inst->getTimeoutsList()->getClosestTimeout(nullptr);
        alarm(new_timeout);
    }
}