.SUFFIXES: .c .o
CC = gcc
CCFLAGS = -Wall -Werror
EXEC = cpu
OBJS = cpu.o

${EXEC}: ${OBJS}
	${CC} ${CCFLAGS} -o ${EXEC} ${OBJS}

.c.o:
	${CC} ${CCFLAGS} -c $<

run: ${EXEC}
	./${EXEC}

clean:
	rm -f ${EXEC} ${OBJS}

cpu.o: cpu.c