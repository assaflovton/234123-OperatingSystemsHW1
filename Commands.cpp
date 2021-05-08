#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <sys/file.h>
#include <iomanip>
#include <stdio.h>
#include "Commands.h"
#include <algorithm> // std::sort
using namespace std;
#if 0
#define FUNC_ENTRY() \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT() \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
const char *CD_FLAG = "-";
const char *PIPE = "|";
const char *PIPEA = "|&";
const char *DOUBLEARROW = ">>";
const char *ARROW = ">";
const std::string WHITESPACE = "\n\r\t\f\v";

#endif

string _ltrim(const std::string &s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string &s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string &s)
{
  return _rtrim(_ltrim(s));
}

int _parsecommand_line(const char *cmd_line, char **args)
{
  FUNC_ENTRY()
  int i = 0;
  std::istringstream iss(_trim(string(cmd_line)).c_str());
  for (std::string s; iss >> s;)
  {
    args[i] = (char *)malloc(s.length() + 1);
    memset(args[i], 0, s.length() + 1);
    strcpy(args[i], s.c_str());
    args[++i] = NULL;
  }
  return i;

  FUNC_EXIT()
}

bool _isBackgroundComamnd(const char *cmd_line)
{
  const string str(cmd_line);
  return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char *cmd_line)
{
  const string str(cmd_line);
  // find last character other than spaces
  unsigned int idx = str.find_last_not_of(WHITESPACE);
  // if all characters are spaces then return
  if (idx == string::npos)
  {
    return;
  }
  // if the command line does not end with & then return
  if (cmd_line[idx] != '&')
  {
    return;
  }
  // replace the & (background sign) with space and then remove all tailing spaces.
  cmd_line[idx] = ' ';
  // truncate the command line string up to the last non-space character
  cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

// TODO: Add your implementation for classes in Commands.h

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
int string_to_int(char* sm, int strt){
    if(sm==NULL){
      return 0;
    }
    int num = 0,i=strt;
    while(sm[i]!='\0'){
        num = 10*num+(sm[i++]-'0');
    }
    return num;
}
int findnumword(int num, const char* cmd){
    int ind =0, curr_word =0;
    bool in_word =false;
    while(num!=curr_word){
        if(cmd[ind]!=' '&&!in_word){
            in_word =true;
            curr_word++;
        }
        if(cmd[ind]== ' '&& in_word){
            in_word =false;
        }
        if(cmd[ind]=='\0')
            return -1;
        ind++;
    }
    return ind-1;
}
Command *SmallShell::CreateCommand(const char *cmd_line)
{
  string cmd_s = _trim(string(cmd_line));
  string first_word = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
  string command_line = string(cmd_line);
  (SmallShell::getInstance().added)=false;//gets the current process id
  if (command_line.find('|') < command_line.size())
  {
    return new PipeCommand(cmd_line);
  }
  if (command_line.find('>') < command_line.size())
  {
    return new RedirectionCommand(cmd_line);
  }
  if(cmd_s == std::string("timeout") || cmd_s.find("timeout ") == 0){
        char** params = new char*[COMMAND_MAX_ARGS+1];
        for(int i=0; i<=COMMAND_MAX_ARGS; i++){
            params[i] = nullptr;
        }
        int argc=_parsecommand_line(cmd_line, params);
        if(argc<=2){
          cerr << "smash error: timeout: invalid arguments" << endl;
          delete[] params;
          return NULL;
        }
        unsigned int dur = string_to_int(params[1],0);
        int len_to_cmd = findnumword(3,cmd_line);
        ExternalCommand* cmd = new ExternalCommand(cmd_line+len_to_cmd,&_tl, dur);
        cmd->execute();
        delete[] params;//TODO clear inner params
        // if((SmallShell::getInstance().added)==true)
        //   return cmd;//gets the current process id)
  }
  if (first_word.compare("pwd") == 0)
  {
    return new GetCurrDirCommand(cmd_line);
  }
  else if (first_word.compare("showpid") == 0)
  {
    return new ShowPidCommand(cmd_line);
  }
  else if (first_word.compare("chprompt") == 0)
  {
    return new ChangePrompt(cmd_line);
  }
  else if (first_word.compare("cd") == 0)
  {
    return new ChangeDirCommand(cmd_line);
  }
  else if (first_word.compare("jobs") == 0)
  {
    return new JobsCommand(cmd_line);
  }
  else if (first_word.compare("kill") == 0)
  {
    return new KillCommand(cmd_line, (SmallShell::getInstance().jobs_list));
  }
  else if (first_word.compare("fg") == 0)
  {
    return new ForegroundCommand(cmd_line, (SmallShell::getInstance().jobs_list), PID_NOT_SET);
  }
  else if (first_word.compare("bg") == 0)
  {
    return new BackgroundCommand(cmd_line, (SmallShell::getInstance().jobs_list), PID_NOT_SET);
  }
  else if (first_word.compare("quit") == 0)
  {
    return new QuitCommand(cmd_line, (SmallShell::getInstance().jobs_list));
  }
  else if (first_word.compare("cat") == 0)
  {
    return new CatCommand(cmd_line);
  }
  else
  {
    return new ExternalCommand(cmd_line);
  }
  return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line)
{
  Command *cmd = CreateCommand(cmd_line);
  if (cmd != NULL)
    cmd->execute();
}

Command::Command(const char *cmd_line) : cmd_line(cmd_line)
{
  argv = new char *[COMMAND_MAX_ARGS + 1];
  argc = _parsecommand_line(cmd_line, argv);//returns the number of args in the argv vector
}

void GetCurrDirCommand::execute()
{
  char *buf = new char[PATH_MAX];
  if (!getcwd(buf, PATH_MAX))//get the currernt directory and writes it into buf
  {
    perror("smash error: getcwd failed");
  }
  std::cout << (buf) << "\n";
}

void ShowPidCommand::execute()
{
  int pid = (SmallShell::getInstance().my_pid);//gets the current process id
  std::cout << "smash pid is " << pid << "\n";
}

void ChangePrompt::execute()
{
  if (argc <= 1) //got no args change prompt name to smash on defualt
  {
    SmallShell::getInstance().sprompt = std::string("smash");
  }
  else
  {
    SmallShell::getInstance().sprompt = std::string(argv[1]);//change the name of the prompt to the specified arg  
  }
}

void ChangeDirCommand::execute()
{
  if (argc > 2)
  { //too many args
    std::cerr << "smash error: cd: too many arguments\n";
    return;
  }
  char *buf = new char[PATH_MAX];                           //alloc buf to save the last dir path
  char *last_path = *(SmallShell::getInstance().last_path); //get last dir path
  if (!strcmp(argv[1], CD_FLAG))
  { //go back to the last path
    if (last_path == nullptr)//there is no path to go back to
    {
      std::cerr << "smash error: cd: OLDPWD not set\n";
      return;
    }
    if (!getcwd(buf, PATH_MAX)) //get cur dir
    {
      perror("smash error: getcwd failed");
      return;
    }
    if (chdir(last_path) < 0)
    { //chdir faild
      //fprintf(stderr, "smash error: chdir failed: %s\n", strerror(errno));
      perror("smash error: chdir failed");
      return;
    }
  }
  else
  {                             //change dir to the specified dir
    if (!getcwd(buf, PATH_MAX)) //get the curr dir
    {
      perror("smash error: chdir failed");
      return;
    }
    if (chdir(argv[1]) < 0) //change to the given args
    {
      //fprintf(stderr, "smash error: chdir failed: %s\n", strerror(errno));
      perror("smash error: chdir failed");
      return;
    }
  }
  delete[] SmallShell::getInstance().last_path;               //deletes the last last dir
  SmallShell::getInstance().last_path = new char *[PATH_MAX]; //alloc for lst d
  *(SmallShell::getInstance().last_path) = buf;               //updated last dir to be last dir
}
void QuitCommand::execute()
{
  char *k = (char *)"kill";
  if (argc == 1) //got no paremeters only exit
  {
    kill(SmallShell::getInstance().my_pid,SIGKILL);
    exit(0);
  }
  else if (strcmp(argv[1], k) == 0) //got parameter kill, killing all jobs before exit
  {
    JobsList *jobs_list = (SmallShell::getInstance().jobs_list);
    std::cout << "smash: sending SIGKILL signal to " << jobs_list->job_vec.size() << " jobs:"<<endl;
    for (auto &job : jobs_list->job_vec)
  {
    if (job.job_status != FINISHED)
    {
       std::cout<<job.pid<<": "<<job.cmd_line<<endl;
    } 
  }
    jobs_list->killAllJobs();
    kill(SmallShell::getInstance().my_pid,SIGKILL);
    exit(0);
  }
}

KillCommand::KillCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line)
{
  this->job_list = jobs;
}

bool KillCommand::ValidArgs()
{
  if (argv[1] == nullptr || argv[2] == nullptr || argv[1][0] != '-' ||
      argv[1][1] < '0' || argv[1][1] > '9' || argv[3] != nullptr)
  {
    cerr << "smash error: kill: invalid arguments" << endl;
    return false;
  }
  int i = 2;
  while (argv[1][i] != '\0') //make sure the sig is a number
  {
    if (argv[1][i] < '0' || argv[1][i] > '9')
    {
      cerr << "smash error: kill: invalid arguments" << endl;
      return false;
    }
    i++;
  }
  i = 0;
  if (argv[2][0] == '\0')
  {
    cerr << "smash error: kill: invalid arguments" << endl;
    return false;
  }
  if (argv[2][0] == '-')
  {
    i = 1;
  }
  while (argv[2][i] != '\0') //make sure the pid is a number
  {
    if (argv[2][i] < '0' || argv[2][i] > '9')
    {
      cerr << "smash error: kill: invalid arguments" << endl;
      return false;
    }
    i++;
  }
  if (argv[2][0] == '-')
  {
    cerr << "smash error: kill: job-id -" << string_to_int(argv[2], 1) << " does not exist" << endl;
    return false;
  }
  return true;
}
void KillCommand::execute()
{
  if (!ValidArgs())
    return;
  int signum = string_to_int(argv[1], 1);
  int jb_id = string_to_int(argv[2], 0);
  JobsList::JobEntry *jb = job_list->getJobById(jb_id);
  if (jb == NULL)
  {
    std::cerr << "smash error: kill: job-id " << jb_id << " does not exist" << endl;
    return;
  }
  if (kill(jb->pid, signum) == -1)
    perror("smash error: kill failed");
  else
    cout << "signal number " << signum << " was sent to pid " << jb->pid << endl;
}

void JobsList::JobEntry::printJob()
{
  time_t t = time(NULL);
  time_t time = difftime(t, start_time);
  std::cout << "[" << job_id << "] ";
  unsigned int dur = (SmallShell::getInstance()).getTimeoutsList()->exist(pid);
    if(dur != DURATION_NOT_SET){
            cout<<"timeout " << dur << " ";
      }
 std::cout << this->cmd_line << " : " << pid << " " << time << " secs"
            << "\n";
}

void JobsList::JobEntry::printJobStopped()
{
  time_t t = time(NULL);
  time_t time = difftime(t, start_time);
  std::cout << "[" << job_id << "] ";
  unsigned int dur = (SmallShell::getInstance()).getTimeoutsList()->exist(pid);
    if(dur != DURATION_NOT_SET){
            cout<<"timeout " << dur << " ";
      }
  std:cout << this->cmd_line << " : " << pid << " " << time << " secs"
            << " (stopped)"
            << "\n";
}
void JobsList::printJobsList()
{
  int i = 0;
  for (auto &job : this->job_vec)
  {
    if (job.job_status == FINISHED)
    {
      job_vec.erase(job_vec.begin() + i);
    }
    i++;
  }
  vector<JobEntry> v_sorted(job_vec.size());
  partial_sort_copy(begin(job_vec), end(job_vec), begin(v_sorted), end(v_sorted));
  int dur;
  for (auto &job : v_sorted)
  {
    if (job.job_status == RUNNING)
    {
      job.printJob();
    }
    if (job.job_status == STOPPED)
    {
      job.printJobStopped();
    }
  }
}

void JobsCommand::execute()
{
  (SmallShell::getInstance()).jobs_list->printJobsList();
}

void ForegroundCommand::execute()
{
  int job_id;
  if (argc > 2)
  {
    std::cerr << "smash error: fg: invalid arguments\n";
    return;
  }
  JobsList *jobs_list = (SmallShell::getInstance().jobs_list);
  if (argc == 1) // pid not supported we will bring the max stopped
  {
    job_id = jobs_list->max_job;
  }
  else if (argc == 2)
  {
    if (strcmp(argv[1], "0") == 0)
    {
      std::cerr << "smash error: fg: job-id " << 0 << " does not exist" << endl;
      return;
    }
    job_id = atoi(argv[1]);
    if (!job_id) //conversion failed
    {
      std::cerr << "smash error: fg: invalid arguments\n";
      return;
    }
  }
  if (job_id == NO_JOBS) //the job list is empty of stopped jobs
  {
    std::cerr << "smash error: fg: jobs list is empty\n";
    return;
  }
  JobsList::JobEntry *job = jobs_list->getJobById(job_id);
  if (job == nullptr)
  {
    std::cerr << "smash error: fg: job-id " << job_id << " does not exist" << endl;
    return;
  }
  unsigned int dur = (SmallShell::getInstance()).getTimeoutsList()->exist(job->pid);
    if(dur != DURATION_NOT_SET){
        cout << "timeout "<<dur<<" ";
    }
  std::cout<<job->cmd_line<<" : "<<job->pid<<endl;
  job->job_status = RUNNING;
  (SmallShell::getInstance().cur_job) = job;
  pid_t pid = job->pid;
  kill(pid, SIGCONT); //brings the max_id to foreground
  waitpid(pid, NULL, WUNTRACED);
  jobs_list->removeJobById(job_id);
  (SmallShell::getInstance().cur_job) = nullptr;
}

void BackgroundCommand::execute()
{
  int job_id;
  if (argc > 2)
  {
    std::cerr << "smash error: bg: invalid arguments";
    return;
  }
  JobsList *jobs_list = (SmallShell::getInstance().jobs_list);
  if (argc == 1) // pid not supported we will bring the max stopped
  {
    int max_id = NO_JOBS;
    for (auto it = begin(jobs_list->job_vec); it != end(jobs_list->job_vec); ++it)
    {
      if (it->job_status == STOPPED && max_id < it->job_id)
      {
        max_id = it->job_id;
      }
    }
    job_id = max_id;
  }
  else if (argc == 2)
  {
    if (strcmp(argv[1], "0") == 0)
    {
      std::cerr << "smash error: bg: job-id " << 0 << " does not exist" << endl;
      return;
    }
    job_id = atoi(argv[1]);
    if (!job_id) //conversion failed
    {
      std::cerr << "smash error: bg: invalid arguments\n";
      return;
    }
  }
  if (job_id == NO_JOBS) //the job list is empty of stopped jobs
  {
    std::cerr << "smash error: bg: there is no stopped jobs to resume\n";
    return;
  }
  JobsList::JobEntry *job = jobs_list->getJobById(job_id);
  if (job == nullptr)
  {
    std::cerr << "smash error: bg: job-id " << job_id << " does not exist" << endl;
    return;
  }
  if (job->job_status == RUNNING)
  {
    std::cerr << "smash error: bg: job-id "<<job_id<<" is already running in the background\n";
    return;
  }
  job->job_status = RUNNING;
  unsigned int dur = (SmallShell::getInstance()).getTimeoutsList()->exist(job->pid);
    if(dur != DURATION_NOT_SET){
        cout << "timeout "<<dur<<" ";
    }
  std::cout<<job->cmd_line<<" : "<<job->pid<<endl;
  kill(job->pid, SIGCONT); //brings the max_id to background
}

ExternalCommand::ExternalCommand(const char *cmd_line,TimeList* time_list, unsigned int dur) : 
Command(cmd_line),_tl(time_list),curr_dur(dur)
{
  this->jobs_list = (SmallShell::getInstance().jobs_list);
  this->pid = PID_NOT_SET;
}

void ExternalCommand::execute()
{
  for (int i = 0; i < 3000000; i++)
  {
      i++;
  }
  (SmallShell::getInstance()).jobs_list->removeFinishedJobs();
  bool isBack = _isBackgroundComamnd(cmd_line);
  char *command = new char[COMMAND_ARGS_MAX_LENGTH + 1];
  strcpy(command, cmd_line);
  _removeBackgroundSign(command);
  this->pid = fork();
  if(curr_dur!= DURATION_NOT_SET && _tl!=nullptr&& this->pid>0 ){
        _tl->addTimeEntry(cmd_line,this->pid,time(NULL),curr_dur);
        pid_t min_pid = 0;
        unsigned int new_dur = _tl->getClosestTimeout(&min_pid);
        if(_tl->getSize() == 0 || min_pid == this->pid)
        {
            alarm(new_dur);
        }
    }
  if (pid == 0)
  { //son
    setpgrp();
    execl("/bin/bash", "bash", "-c", command, NULL);
    perror("smash error: exec failed");
    delete[] command;
    return;
  }
  else if (pid < 0)
  { //error
    perror("smash error: fork failed");
    delete[] command;
    return;
  }
  else if (isBack) // external command in background
  {
    char *command3 = new char[COMMAND_ARGS_MAX_LENGTH + 1];
    strcpy(command3, cmd_line);
    int id = ++(jobs_list->max_job);
    if(!(SmallShell::getInstance().added)){
      JobsList::JobEntry *jb = new JobsList::JobEntry(command3, pid, id, RUNNING);
      jobs_list->job_vec.push_back(*jb);
    }
    (SmallShell::getInstance().added)=true;
    delete[] command;
    return;
  }
  else if (!isBack) //external command running in foreground
  {
    char *command2 = new char[COMMAND_ARGS_MAX_LENGTH + 1];
    strcpy(command2, cmd_line);
    int id = ++(jobs_list->max_job);
    if(!(SmallShell::getInstance().added)){
      JobsList::JobEntry *jb = new JobsList::JobEntry(command2, pid, id, RUNNING);
      (SmallShell::getInstance().cur_job) = jb;
    }
    (SmallShell::getInstance().added)=true;
    if (waitpid(pid, NULL, WUNTRACED) < 0)
    {
      perror("smash error: wait failed");
    }
    /////////////(SmallShell::getInstance().jobs_list)->removeJobById(jb->job_id);
    (SmallShell::getInstance().cur_job) = nullptr;
    delete[] command;
    return;
  }
}

void JobsList::removeJobByPid(pid_t pid)
{
  for (unsigned int i = 0; i < job_vec.size(); i++)
  {
    if (job_vec[i].pid == pid)
    {
      job_vec.erase(job_vec.begin() + i);
      return;
    }
  }
}

void JobsList::removeFinishedJobs()
{
  pid_t finproc;
  finproc = waitpid(-1, NULL, WNOHANG);
  while (finproc > 0)
  { //checks if there is a finished child
    removeJobByPid(finproc);
    finproc = waitpid(-1, NULL, WNOHANG);
  }
  int max_id = NO_JOBS;
  for (auto it = begin(this->job_vec); it != end(this->job_vec); ++it)
  {
    if (it->job_status == FINISHED)
    {
      this->job_vec.erase(it);
    }
    if (max_id < it->job_id)
    {
      max_id = it->job_id;
    }
  }
  this->max_job = max_id;
}

JobsList::JobEntry *JobsList::getJobById(int jobId)
{
  int cou = 0;
  for (auto it = begin(this->job_vec); it != end(this->job_vec); ++it)
  {
    if (it->job_id == jobId)
    {
      return &job_vec[cou];
    }
    cou++;
  }
  return NULL;
}

void JobsList::killAllJobs()
{
  for (auto it = begin(this->job_vec); it != end(this->job_vec); ++it)
  {
    kill(it->pid, SIGKILL);
  }
}

void JobsList::removeJobById(int jobId)
{
  for (auto it = begin(this->job_vec); it != end(this->job_vec); ++it)
  {
    if (it->job_id == jobId)
    {
      this->job_vec.erase(it);
      return;
    }
  }
}

void RedirectionCommand::execute()
{
  std::string string_cmd_line = std::string(cmd_line);
  int position = string_cmd_line.find('>');
  std::string after_pipe = string_cmd_line.substr(position + 1, string_cmd_line.size() - position - 1);
  std::string string_until_pos = string_cmd_line.substr(0, position);
  while(after_pipe.size() && isspace(after_pipe.front())) after_pipe.erase(after_pipe.begin() + (76 - 0x4C));
  pid_t pid = fork(); //the son process
  if (pid == 0)
  {
    if (!after_pipe.empty() && after_pipe.at(after_pipe.size() - 1) == '&') //ignore & running always in foreground
    {
      after_pipe.pop_back(); //remove &
    }
    close(1);                    //close the standart output, screen
    if (after_pipe.at(0) == '>') //if we had >> we want to append to the file insted of overwriting it
    {
      after_pipe.erase((after_pipe.begin()));                        //delete >>
      open(after_pipe.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644); //open with create and append
    }
    else
    {
      open(after_pipe.c_str(), O_WRONLY | O_CREAT, 0644); //will overwrite the file
    }
    SmallShell::getInstance().executeCommand(string_until_pos.c_str()); //executing the command that was supplied the pipe
    exit(0);                                                            //exit shell in son process
  }
  else if (pid > 0) //the parent process
  {
    if (!string_cmd_line.empty() && string_cmd_line.at(string_cmd_line.size() - 1) == '&')
    { //need to run in background
    }
    else
    {                                                                              //need to run in foreground
      JobsList::JobEntry *job = new JobsList::JobEntry(cmd_line, pid, 1, RUNNING); /////////fix
      (SmallShell::getInstance()).cur_job = job;
      waitpid(pid, nullptr, WUNTRACED);
      (SmallShell::getInstance()).cur_job = nullptr;
    }
  }
}

void PipeCommand::execute()
{
  int pipe_array[2];
  int pipe_out_channel = 1;
  if (pipe(pipe_array) < 0)
  {
    perror("smash error: fork failed");
  }
  std::string string_cmd_line = std::string(cmd_line);
  size_t pipe_sep = string_cmd_line.find('|');
  string first_command = string_cmd_line.substr(0, pipe_sep);
  string second_command = string_cmd_line.substr(pipe_sep + 1, string_cmd_line.size() - pipe_sep - 1);
  while(second_command.size() && isspace(second_command.front())) second_command.erase(second_command.begin() + (76 - 0x4C));//remove the spaces between the pipe and the next argumant
  //check if need to run in backgound
  if (second_command.at(0) == '&')
  {
    pipe_out_channel = 2;
    second_command.erase(second_command.begin());
  }
  if (!second_command.empty() && second_command.at(second_command.size() - 1) == '&')
  {
    second_command.pop_back();
  }
  //FIXME:BG
  int first_pid = fork();
  if (first_pid < 0)
  {
    perror("smash error: fork failed");
  }
  if (first_pid == 0)
  {                                        //the first son is the the writer
    dup2(pipe_array[1], pipe_out_channel); //stdout is where we write to
    close(pipe_array[0]);                  //close the reading channel
    close(pipe_array[1]);                  //close the writing channel (the stdout is connected to pipe now)
    SmallShell::getInstance().executeCommand(first_command.c_str());
    exit(0); //exit shell in son process
  }
  int second_pid = fork();
  if (second_pid < 0)
  {
    perror("smash error: fork failed");
  }
  if (second_pid == 0)
  {                         //son2-reader
    dup2(pipe_array[0], 0); //stdin is reading edge
    close(pipe_array[0]);   //close reading edge(the stdin is connected to pipe now)
    close(pipe_array[1]);   //close writing edge
    SmallShell::getInstance().executeCommand(second_command.c_str());
    exit(0); //exit shell in son process
  }
  //parent process
  close(pipe_array[0]);
  close(pipe_array[1]);
  if (!string_cmd_line.empty() && string_cmd_line.at(string_cmd_line.size() - 1) == '&')
  {
    JobsList::JobEntry *job = new JobsList::JobEntry(cmd_line, second_pid, 1, RUNNING); /////////fix
    (SmallShell::getInstance()).jobs_list->job_vec.push_back(*job);
  }
  else
  {
    JobsList::JobEntry *job = new JobsList::JobEntry(cmd_line, second_pid, 1, RUNNING); /////////fix
    (SmallShell::getInstance()).cur_job = job;
    waitpid(second_pid, nullptr, WUNTRACED); //running in foreground waiting for child to end or to be stopped
    (SmallShell::getInstance()).cur_job = nullptr;
  }
}

CatCommand::CatCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}
void CatCommand::execute()
{
  int fd, i, buff;
  if(argc==1)
  {
    perror("smash error: cat: not enough arguments");
    return;
  }
  for (i = 1; i < argc; i++)
  {
    FILE* f = fopen(argv[i],"r"); //use readonly mode in oreder to read the file 
    fd = fileno(f);
    if (fd<0)
    {
      perror("smash error: cat failed");
      return;
    }
    while (read(fd, &buff, 1)) 

      write(STDOUT_FILENO, &buff, 1); //writing one byte at the time to the std out

    close(fd); //
  }
}






TimeList::TimeList():timeouts_vec(vector<TimeEntry>()){}

void TimeList::addTimeEntry(const char* cmd, int pid,time_t st, unsigned int dur){
    TimeEntry t = TimeEntry(pid,st,dur,cmd);
    timeouts_vec.push_back(t);
    return;
}

unsigned int TimeList::exist(pid_t pid){
    for(unsigned int i=0; i<timeouts_vec.size();i++){
        if(timeouts_vec[i].pid == pid){
            return timeouts_vec[i].curr_dur;
        }
    }
    return DURATION_NOT_SET;
}

void TimeList::killAllTimeouts(){
    for(unsigned int i=0; i < timeouts_vec.size(); i++){
        if(kill(timeouts_vec[i].pid , SIGKILL)<0){
            perror("smash error: kill failed");
        }
    }
}

void TimeList::removeTimeoutByPid(pid_t pid){
    for(unsigned int i=0; i < timeouts_vec.size(); i++){
        if(timeouts_vec[i].pid ==pid){
            timeouts_vec.erase(timeouts_vec.begin()+i);
            return;
        }
    }
}
double myabs(double a){
    if(a>=0)
        return a;
    return (-a);
}

TimeEntry* TimeList:: findByTime(time_t curr){
    double real_duration, how_close, closest;
    if(timeouts_vec.size()==0)
        return nullptr;
    unsigned int closest_index=0;
    for(unsigned int i=0; i < timeouts_vec.size(); i++){
        real_duration = difftime(curr,timeouts_vec[i]._start_time);
        how_close = real_duration-timeouts_vec[i].curr_dur;
        if(i==0)
            closest = myabs(how_close);
        if(myabs(how_close)<closest){
            closest = myabs(how_close);
            closest_index = i;
        }
    }
    return &(timeouts_vec[closest_index]);
}
unsigned int TimeList::getSize(){
    return timeouts_vec.size();
}

pid_t TimeList::getPidinPlace(unsigned int p){
    if(p>=timeouts_vec.size())
        return 0;
    else
        return (timeouts_vec[p].pid);
}
unsigned int TimeList::getClosestTimeout(pid_t* pid){
    time_t curr_time = time(NULL);
    unsigned int min_dur =-1;
    unsigned int how_close;
    for(unsigned int i=0; i < timeouts_vec.size(); i++){
        double real_duration = difftime(curr_time,timeouts_vec[i]._start_time);
        how_close = (unsigned int)(myabs(timeouts_vec[i].curr_dur - real_duration));
        if(i==0) {
            min_dur = how_close;
        }
        if(how_close <= min_dur){
            min_dur = how_close;
            if(pid != nullptr){
                *pid = timeouts_vec[i].pid;
            }
        }

    }
    return min_dur;
}