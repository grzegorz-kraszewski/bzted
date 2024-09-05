CC = g++
LD = g++
CFLAGS = -m68000 -nostdlib -fbaserel -O2 -fomit-frame-pointer -fno-exceptions -fno-rtti -mregparm -D__NOLIBBASE__
LDFLAGS = -m68000 -nostdlib -fbaserel -nostartfiles -fno-exceptions -fno-rtti
LIBS = 
OBJS = start.o main.o compiler.o token.o keyedarray.o builtins.o inter.o function.o
EXE = Bzted

all: $(OBJS)
	@echo "Linking $(EXE)..."
	@$(LD) $(LDFLAGS) -o $(EXE).db $^ $(LIBS)
	@strip $(EXE).db -o $(EXE) --strip-unneeded
	@Protect $(EXE) +E
	List $(EXE)

clean:
	rm -vf *.o $(EXE) $(EXE).db

start.o: start.cpp
	@echo "Compiling $@..."
	@$(CC) $(CFLAGS) -fwritable-strings -c -o $@ $<

%.o: %.cpp
	@echo "Compiling $@..."
	@$(CC) $(CFLAGS) -c -o $@ $<

# dependencies

builtins.o: builtins.cpp builtins.h operator.h function.h syslist.h inter.h keyedarray.h
compiler.o: compiler.cpp compiler.h main.h token.h operator.h function.h syslist.h inter.h builtins.h keyedarray.h
function.o: function.cpp main.h compiler.h token.h operator.h function.h syslist.h inter.h builtins.h keyedarray.h
inter.o: inter.cpp main.h compiler.h token.h operator.h function.h syslist.h inter.h builtins.h keyedarray.h
keyedarray.o: keyedarray.cpp keyedarray.h
main.o: main.cpp compiler.h main.h token.h operator.h function.h syslist.h inter.h builtins.h keyedarray.h
start.o: start.cpp
syscalls.o: syscalls.cpp syscalls.h
token.o: token.cpp main.h compiler.h token.h operator.h function.h syslist.h inter.h builtins.h keyedarray.h

