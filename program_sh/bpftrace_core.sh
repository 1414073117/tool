BEGIN 
{ 
    printf("Tracing kill() signals... Hit Ctrl-C to end.\n"); 
    printf("%-9s %-6s %-16s %-4s %-6s %s\n", "TIME", "PID", "COMM", "SIG", 
        "TPID", "RESULT"); 
} 
 
tracepoint:syscalls:sys_enter_kill 
{ 
    @tpid[tid] = args->pid; 
    @tsig[tid] = args->sig; 
} 
 
tracepoint:syscalls:sys_exit_kill 
/@tpid[tid]/ 
{ 
    time("%H:%M:%S  "); 
    printf("%-6d %-16s %-4d %-6d %d\n", pid, comm, @tsig[tid], @tpid[tid], 
        args->ret); 
    delete(@tpid[tid]); 
    delete(@tsig[tid]); 
} 