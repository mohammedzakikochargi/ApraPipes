Note: 
libnvidia-encode.so.1 is a copy of libnvidia-encode.so instead of a symbolic link

this was done to support WSL build where a checkout done on windows never created a symbolic link.

a better approach is to follow this instead
https://stackoverflow.com/questions/5917249/git-symbolic-links-in-windows/59761201#59761201