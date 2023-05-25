# Process-Scheduler

### To run, compile all the files in projects and your test programs, then execute the interpreter.


- Runs pseudo-programs through a scheduler. They will be either round robin or real time.
- The exec.txt file contains a specific format for the programs you want to run: 
    - "Run (program name)" - for Round Robin
    - "Run (program name) I=(start time) D=(execution duration)"
  
- Start time and duration are in seconds.
- If you want to run more than 10 programs, alter the constant MAXPROCESSES to your desired value.
- The pseudo-programs are programs that run indefinitely, printing every one second.
- You can alter the total run time of the Process-Scheduler by changing the TIMELIMIT (in seconds) constant.
