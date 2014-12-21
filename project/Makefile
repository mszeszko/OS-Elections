CC = gcc
CFLAGS = -Wall --pedantic -c
LDFLAGS = -Wall --pedantic -lpthread

OBJECTS = err.o committee_service.o report_service.o server_service.o process_results_service.o send_report_service.o update_results_service.o
ALL = committee report server

all: $(ALL)

%.o : %.c
	$(CC) $(CFLAGS) $<

$(ALL) : % : %.o $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	rm -f *.o $(ALL) *~
