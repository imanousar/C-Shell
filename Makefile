SHELL := /bin/bash


# ============================================
# COMMANDS

CC = gcc -O3
RM = rm -f

# ==========================================
# TARGETS

EXECUTABLES =	myshell	\



all: $(EXECUTABLES)


%: %.c
	$(CC) $< -o $@

clean:
	$(RM) *.o *~ $(EXECUTABLES)
