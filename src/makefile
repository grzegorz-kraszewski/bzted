CC = g++
LD = g++
CFLAGS = -W -Wall -Wno-parentheses -m68000 -nostdlib -fbaserel -O2 -fomit-frame-pointer -fno-exceptions \
 -fno-rtti -mregparm -D__NOLIBBASE__
LDFLAGS = -m68000 -nostdlib -fbaserel -nostartfiles -fno-exceptions -fno-rtti
LIBS = 
OBJS = start.o main.o compiler.o scanner.o lexer.o optimizer.o token.o function.o inter.o builtins.o \
 syscalls.o logger.o strutils.o scheduler.o
EXE = Bzted

all: $(OBJS)
	@echo "Linking $(EXE)..."
	@$(LD) $(LDFLAGS) -o $(EXE).db $^ $(LIBS)
	@strip $(EXE).db -o $(EXE) --strip-unneeded
	@Protect $(EXE) +E
	List $(EXE)

clean:
	rm -vf *.o $(EXE) $(EXE).db

dep:
	g++ -MM $(OBJS:.o=.cpp)

start.o: start.cpp
	@echo "Compiling $@..."
	@$(CC) $(CFLAGS) -fwritable-strings -c -o $@ $<

%.o: %.cpp
	@echo "Compiling $@..."
	@$(CC) $(CFLAGS) -c -o $@ $<

# dependencies

start.o: start.cpp
main.o: main.cpp compiler.h main.h rplist.h strutils.h keyedarray.h \
 token.h function.h inter.h builtins.h operator.h syscalls.h logger.h
compiler.o: compiler.cpp compiler.h main.h rplist.h strutils.h \
 keyedarray.h token.h function.h inter.h builtins.h operator.h \
 syscalls.h logger.h scanner.h lexer.h optimizer.h
scanner.o: scanner.cpp scanner.h token.h function.h rplist.h \
 strutils.h inter.h logger.h main.h
lexer.o: lexer.cpp compiler.h main.h rplist.h strutils.h keyedarray.h \
 token.h function.h inter.h builtins.h operator.h syscalls.h lexer.h \
 logger.h
optimizer.o: optimizer.cpp optimizer.h inter.h rplist.h strutils.h \
 function.h main.h
token.o: token.cpp compiler.h main.h rplist.h strutils.h keyedarray.h \
 token.h function.h inter.h builtins.h operator.h syscalls.h logger.h
function.o: function.cpp main.h compiler.h rplist.h strutils.h \
 keyedarray.h token.h function.h inter.h builtins.h operator.h \
 syscalls.h logger.h
inter.o: inter.cpp main.h inter.h rplist.h strutils.h
builtins.o: builtins.cpp builtins.h operator.h keyedarray.h strutils.h \
 function.h rplist.h inter.h
syscalls.o: syscalls.cpp syscalls.h keyedarray.h strutils.h
logger.o: logger.cpp main.h logger.h strutils.h
strutils.o: strutils.cpp strutils.h
