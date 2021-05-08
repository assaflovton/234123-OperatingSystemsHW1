#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_
#include <unistd.h>
#include <vector>
#include <ostream>
#include <time.h>
#include <string.h>
#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (30)
#define PATH_MAX 1000
#define NO_JOBS 0
#define PID_NOT_SET -2
#define JOB_ID_NOT_SET -2
#define DURATION_NOT_SET (0)
using std::vector;
enum status
{
  RUNNING,
  STOPPED,
  FINISHED
};

class Command
{
  // TODO: Add your data members
public:
  const char *cmd_line;
  char **argv;
  int argc;
  Command(const char *cmd_line);
  virtual ~Command()
  {
    delete[] argv;
  }
  virtual void execute() = 0;
  //virtual void prepare();
  //virtual void cleanup();
  //TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command
{
public:
  BuiltInCommand(const char *cmd_line) : Command(cmd_line) {}
  virtual ~BuiltInCommand() {}
};
class JobsList;

class TimeEntry{
public:
    pid_t pid;
    time_t _start_time;
    unsigned int curr_dur;
    char* _cmd;
    TimeEntry(pid_t pid, time_t st, unsigned int dur, const char* cmd)
            :pid(pid), _start_time(st),curr_dur(dur),_cmd(nullptr){
        _cmd = new char[COMMAND_ARGS_MAX_LENGTH];
        strcpy(_cmd, cmd);
    }
};
class TimeList{
private:
    std::vector<TimeEntry> timeouts_vec;
public:
    TimeList();
    ~TimeList() = default;
    void addTimeEntry(const char* cmd, int pid,time_t st, unsigned int dur);
    void killAllTimeouts();
    void removeFinishedTimeouts();
    void removeTimeoutByPid(pid_t pid);
    unsigned int getSize();
    pid_t getPidinPlace(unsigned int p);
    unsigned int getClosestTimeout(pid_t* pid);
    TimeEntry* findByTime(time_t curr);
    unsigned int exist(pid_t pid);
};

class ExternalCommand : public Command
{
public:
  JobsList *jobs_list;
  TimeList* _tl;
    unsigned int curr_dur;
  pid_t pid;
  ExternalCommand(const char *cmd_line,TimeList* time_list = nullptr, unsigned int dur = DURATION_NOT_SET);
  virtual ~ExternalCommand() {}
  void execute() override;
};

class PipeCommand : public Command {
    // TODO: Add your data members
public:
    PipeCommand(const char *cmd_line):Command(cmd_line){}

    virtual ~PipeCommand() {}

    void execute() override;
};

class RedirectionCommand : public Command {
    // TODO: Add your data members
public:
    explicit RedirectionCommand(const char *cmd_line):Command(cmd_line){}

    virtual ~RedirectionCommand() {}

    void execute() override;
    //void prepare() override;
    //void cleanup() override;
};


class ChangeDirCommand : public BuiltInCommand
{
public:
  ChangeDirCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}
  virtual ~ChangeDirCommand() {}
  void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand
{
public:
  GetCurrDirCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}
  virtual ~GetCurrDirCommand() {}
  void execute() override;
};

class ShowPidCommand : public BuiltInCommand
{
public:
  ShowPidCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}
  virtual ~ShowPidCommand() {}
  void execute() override;
};

class JobsList;

class QuitCommand : public BuiltInCommand
{
  public:
  JobsList *jobs;
  // TODO: Add your data members public:
  QuitCommand(const char *cmd_line, JobsList *jobs):BuiltInCommand(cmd_line) {
    this->jobs=jobs;
  }
  virtual ~QuitCommand() {}
  void execute() override;
};

class JobsCommand : public BuiltInCommand
{
  // TODO: Add your data members
public:
  JobsCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}
  virtual ~JobsCommand() {}
  void execute() override;
};

class KillCommand : public BuiltInCommand
{
  JobsList* job_list;
public:
  KillCommand(const char *cmd_line, JobsList *jobs);
  virtual ~KillCommand() {}
  void execute() override;
  bool ValidArgs();
};

class CatCommand : public BuiltInCommand
{
public:
  CatCommand(const char *cmd_line);
  virtual ~CatCommand() {}
  void execute() override;
};

class JobsList
{
public:
  class JobEntry
  {
    // TODO: Add your data members
  public:
    JobEntry(const char *cmd_line, pid_t pid, int job_id, status stat)
    {
      this->cmd_line = cmd_line;
      this->pid = pid;
      this->job_id = job_id;
      this->job_status = stat;
      this->start_time = time(NULL);
    }
    JobEntry() {}
    const char *cmd_line;
    int pid;
    int job_id;
    status job_status;
    time_t start_time;
    bool operator<(JobEntry &j)
    {
      return (this->job_id) < j.job_id;
    }
    void printJob();
    void printJobStopped();
  };
  // TODO: Add your data members
  vector<JobEntry> job_vec;
  int max_job; //current max job id
  JobsList()
  {
    job_vec = vector<JobEntry>();
    max_job = NO_JOBS;
  }
  void addJob(Command *cmd, bool isStopped = false);
  void printJobsList();
  void killAllJobs();
  void removeFinishedJobs();
  JobEntry *getJobById(int jobId);
  void removeJobById(int jobId);
  JobEntry *getLastJob(int *lastJobId);
  JobEntry *getLastStoppedJob(int *jobId);
  void removeJobByPid(pid_t pid);
  // TODO: Add extra methods or modify exisitng ones as needed
};

class SmallShell
{
private:
  SmallShell(const char *prompt = "smash")
  {
    sprompt = std::string(prompt);
    last_path = new char *[PATH_MAX];
    *last_path = nullptr;
    start_time = time(NULL);
    if (!start_time)
    {
      perror("smash error: time failed\n");
    }
    jobs_list = new JobsList();
    cur_job=nullptr;
    my_pid=getpid();
    added=false;
  }

public:
bool added;
  TimeList _tl;
  JobsList* jobs_list;
  std::string sprompt;
  char **last_path;
  time_t start_time;
  JobsList::JobEntry* cur_job;
  pid_t my_pid;
  Command *CreateCommand(const char *cmd_line);
  SmallShell(SmallShell const &) = delete;     // disable copy ctor
  void operator=(SmallShell const &) = delete; // disable = operator
  static SmallShell &getInstance()             // make SmallShell singleton
  {
    static SmallShell instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
  ~SmallShell()
  {
    delete[] last_path;
  }
  TimeList* getTimeoutsList(){ return &(_tl); }
  void executeCommand(const char *cmd_line);
  // TODO: add extra methods as needed
};

class ChangePrompt : public BuiltInCommand
{
public:
  ChangePrompt(const char *cmd_line) : BuiltInCommand(cmd_line) {}
  virtual ~ChangePrompt() {}
  void execute() override;
};

class ForegroundCommand : public BuiltInCommand
{
  JobsList *jobs_list;
  pid_t pid;
public:
  ForegroundCommand(const char *cmd_line, JobsList *jobs, pid_t pid):BuiltInCommand(cmd_line) 
  {
    this->jobs_list = jobs;
    this->pid = pid;
  }
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class BackgroundCommand : public BuiltInCommand
{
  JobsList *jobs_list;
  pid_t pid;    
public:
  BackgroundCommand(const char *cmd_line, JobsList *jobs, pid_t pid):BuiltInCommand(cmd_line) 
  {
  this->jobs_list = jobs;
  this->pid = pid;
  }
  virtual ~BackgroundCommand() {}
  void execute() override;
};

#endif //SMASH_COMMAND_H_
