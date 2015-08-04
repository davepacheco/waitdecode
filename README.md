# waitdecode: decode process exit status

This program takes the status codes returned by `wait(3C)` or `bash(1)` and
decodes them into a human-readable summary:

    $ waitdecode 0x500
    status: 0x500 (decimal 1280), as wait(3C) status
        process exited normally with exit status 5

    $ waitdecode 6
    status: 0x6 (decimal 6), as wait(3C) status
        process terminated on SIGABRT

    $ waitdecode 0x86
    status: 0x86 (decimal 134), as wait(3C) status
        process terminated on SIGABRT
        core file created on termination

Note that bash(1) expresses these status codes slightly differently (details
below).  You can decode a bash exit code with the "-b" option:

    $ waitdecode -b 134
    status: 0x86 (decimal 134), as bash return code
        process terminated on SIGABRT


## Background

Traditionally, when Unix processes exit, they return a status code to the
process that waits for them (usually the parent process).  This status embeds a
few pieces of information:

* whether the process exited normally or as a result of a signal
* if normal exit: at least 7 bits of the code passed to `_exit(2)`
* if terminated by a signal: the signal number
* (on some systems) if terminated by a signal, whether a core dump was generated

The same status codes can be returned by wait(3C) when the child has not
actually exited, but rather has stopped or continued as part of job control.

POSIX systems define macros in
[sys/wait.h](http://pubs.opengroup.org/onlinepubs/009695399/basedefs/sys/wait.h.html)
for decoding the status code.  This program uses those to print a
human-readable summary.  On illumos systems,
[http://www.illumos.org/man/3c/wait](wait(3C)) documents the status code in
more detail.

One source of confusion is that bash(1) _also_ provides a status code that
includes whether the process exited normally or as a result of a signal, and
also indicates the value passed to `_exit(2)` or signal number, respectively.
However, bash encodes this information differently.  From the bash(1) manual
page:

    The exit status of an executed command is the value returned by the
    waitpid system call or equivalent function.

    ...

    For the shell's purposes, a command which exits with a zero exit status
    has succeeded.  An exit status of zero indicates success.  A non-zero
    exit status indicates failure.  When a command terminates on a fatal
    signal N, bash uses the value of 128+N as the exit status.

With the "-b" option, this program interprets the argument as a bash exit
status rather than one that came from wait(3C).
