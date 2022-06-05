bison -d syntax.y
flex compilerLex.l
gcc main.c syntax.tab.c -lfl -ly -o parser