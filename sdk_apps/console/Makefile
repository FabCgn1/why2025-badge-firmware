CFLAGS += `pkg-config --cflags sdl3`
LDFLAGS += `pkg-config --libs sdl3`
OBJS = main.o cp437.o

all: console

run: console
	./console

clean:
	rm -f console *.o

console: ${OBJS}
	${CC} -o $@ ${OBJS} ${CFLAGS} ${LDFLAGS}
