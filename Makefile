EXT=$(shell r2 -H R2_LIBEXT)
CFLAGS+=$(shell pkg-config --cflags r_core)
LDFLAGS+=$(shell pkg-config --libs r_core)
LDFLAGS+=-lgcov
R2SYSPLUGDIR=$(shell r2 -H R2_LIBR_PLUGINS)
R2USRPLUGDIR=$(shell r2 -H R2_USER_PLUGINS)
CFLAGS+=-I. -fPIC
CFLAGS+=-Wall
CFLAGS+=-g
CFLAGS+=-fprofile-arcs -ftest-coverage

USE_R2_CURL=1
USE_LIBCURL=0

HAVE_LIBCURL=$(shell pkg-config --exists libcurl && echo 1 || echo 0)

CFLAGS+=-DUSE_R2_CURL=$(USE_R2_CURL)
CFLAGS+=-DUSE_LIBCURL=$(USE_LIBCURL)

ifeq ($(USE_LIBCURL),1)
  ifeq ($(HAVE_LIBCURL),1)
    CFLAGS+=$(shell pkg-config --cflags libcurl)
    LDFLAGS+=$(shell pkg-config --libs libcurl)
    CFLAGS+=-DHAVE_LIBCURL=1
  else
    CFLAGS+=-DHAVE_LIBCURL=0
  endif
endif

OBJS=r2ai.o
OBJS+=auto.o
OBJS+=vdb.o
OBJS+=tools.o
OBJS+=messages.o
OBJS+=anthropic.o
OBJS+=r2ai_http.o
OBJS+=openai.o
OBJS+=markdown.o

all: $(OBJS) r2check
	$(CC) -fPIC -shared -o r2ai.$(EXT) $(OBJS) $(CFLAGS) $(LDFLAGS)

try: all
	$(MAKE) user-install
	$(MAKE) doc-install

doc-install:
	mkdir -p /tmp/embeds
	cp ../doc/data/quotes.txt /tmp/embeds

vdb.o: vdb_embed.inc.c

indent:
	for a in *.c ; do python indent.py $$a ; done

user-install: user-uninstall
	mkdir -p $(R2USRPLUGDIR)
	cp -f r2ai.$(EXT) $(R2USRPLUGDIR)

user-uninstall:
	rm -f $(R2USRPLUGDIR)/r2ai.$(EXT)

install:
	cp -f r2ai.$(EXT) $(R2SYSPLUGDIR)

uninstall:
	rm -f $(R2SYSPLUGDIR)/r2ai.$(EXT)

r2check:
	@r2 -qcq --

# Unit tests
TESTS=tests/messages_test tests/tools_test

tests/messages_test: tests/messages_test.c messages.o $(OBJS)
	$(CC) $(CFLAGS) -o $@ tests/messages_test.c messages.o $(LDFLAGS)

tests/tools_test: tests/tools_test.c tools.o markdown.o messages.o $(OBJS)
	$(CC) $(CFLAGS) -o $@ tests/tools_test.c tools.o markdown.o messages.o $(LDFLAGS)

.PHONY: test coverage clean-coverage

test: $(TESTS)
	./tests/messages_test
	./tests/tools_test
	@echo "Coverage Summary:"
	@gcovr -r . --print-summary

coverage: test
	mkdir -p coverage
	gcovr -r . --html --html-details -o coverage/coverage.html
	@echo "Coverage report generated at coverage/coverage.html"

clean-coverage:
	find . -type f \( -name '*.gcno' -o -name '*.gcda' \) -delete
	rm -rf coverage

clean:
	rm -f *.o *.d $(TESTS)
