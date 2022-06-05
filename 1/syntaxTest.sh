bison -d syntax.y
flex compilerLex.l
gcc syntax.tab.c -lfl -ly -o parser