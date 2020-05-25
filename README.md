# Boothshell

Boothshell is a simple implementation of a shell in the C language. It is a basic shell.
That is: read, parse, fork, exec, and wait.  Since its very small, it has many
limitations, including:

* Commands must be on a single line.
* Arguments must be separated by whitespace.
* No quoting arguments or escaping whitespace.
* No piping or redirection.
* Only builtins are: `cd`, `help`, `exit or quit`
