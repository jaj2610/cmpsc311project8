# Makefile for pr7 shell

# CMPSC 311, Spring 2013, Project 8
#
# Author:		Jacob Jones
# Email:		jaj5333@psu.edu
#
# Author:		Scott Cheloha
# Email:		ssc5145@psu.edu
#
 
INC = hake.c wrapper.c macro.c linked.c haketarget.c
LIB = hake.h wrapper.h macro.h linked.h haketarget.h

hake-gcc : $(INC) $(LIB)
	gcc -std=c99 -Wall -Wextra -pedantic -o hake $(INC) $(LIB)

# NOTE: Untested on Solaris and Linux
hake-sun-c99 : $(INC) $(LIB)
	c99 -v -D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=600 -o hake $(INC) $(LIB)

hake-sun-gcc : $(INC) $(LIB)
	gcc -std=c99 -Wall -Wextra -pedantic -D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=600 -o hake $(INC) $(LIB)

hake-linux : $(INC) $(LIB)
	gcc -std=c99 -Wall -Wextra -pedantic -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=700 -o hake $(INC) $(LIB)

clean :
	rm -f hake a.out *.o
