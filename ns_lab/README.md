# Usage Doc

## Cases

### UTC
> This case will start a shell in new utc and change the hostname.

#### Steps
1. Run program
```bash
>>> ./ns_lab -main=utc
```

2. Exit program
```bash
>>> exit
```

### IPC UTC
> This case will show how to start a new ipc and utc env

#### Steps
1. Create a ipc queue in host
```bash
>>> ipcmk -Q
Message queue id: 0
```

2. Show ipc queues in host
```bash
>>> ipcs -q
------ Message Queues --------
key        msqid      owner      perms      used-bytes   messages
0xa4cbf13a 0          xupeng     644
```

3. Run program
```bash
>>> ./ns_lab -main=ipc_utc
```

4. Show ipc queues in new utc. No queue is expected.
```bash
>>> ipcs -q
------ Message Queues --------
key        msqid      owner      perms      used-bytes   messages
```

5. Exit program

6. Remove the queue
```bash
>>> ipcrm -q 0
```
