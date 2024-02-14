CC=gcc
CFLAGS=-g -Wall
LIBS=-lpthread

all: compute tcompute manage report

compute: compute.c defs.h
	$(CC) $(CFLAGS) compute.c -o compute $(LIBS)

tcompute: tcompute.c defs.h
	$(CC) $(CFLAGS) tcompute.c -o tcompute $(LIBS)

manage: manage.c defs.h
	$(CC) $(CFLAGS) manage.c -o manage $(LIBS)

report: report.c defs.h
	$(CC) $(CFLAGS) report.c -o report $(LIBS)

clean: 
	rm compute tcompute manage report
