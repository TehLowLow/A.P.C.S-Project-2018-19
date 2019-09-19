### DEBUG
#CFLAGS = -O0 -g -fsanitize=address -fno-omit-frame-pointer -Wall -Wextra -pedantic
#CC     = clang

### VALGRIND
#CFLAGS = -O0 -g
#CC     = gcc

### RELEASE
CFLAGS = -O3 -DNDEBUG -std=gnu11 -pedantic
CC     = gcc

all: api19 api19rb

api19: api19.o relset.o relation.o slist.o
	$(CC) $(LDFLAGS) $(CFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@

api19rb: CFLAGS += -DRBTREE
api19rb: api19.o relsetrb.o relationrb.o slist.o rb.o
	$(CC) $(LDFLAGS) $(CFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@

relsetrb.o: relset.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $^ -o $@

relationrb.o: relation.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $^ -o $@

all_sa: api19_sa api19rb_sa

api19_sa: main.o
	$(CC) $(LDFLAGS) $(CFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@

api19rb_sa: mainrb.o
	$(CC) $(LDFLAGS) $(CFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@

main.o: main.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $^ -o $@

mainrb.o: CFLAGS += -DRBTREE
mainrb.o: main.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $^ -o $@

main.c: rb.h slist.h relation.h relset.h rb.c slist.c relation.c relset.c api19.c
	cat $^ | sed '/#include ".*"/d' > $@

clean:
	rm -f *.o api19 api19rb main.c
