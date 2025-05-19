EXT=$(shell r2 -H R2_LIBEXT)
CFLAGS+=$(shell pkg-config --cflags r_core)
LDFLAGS+=$(shell pkg-config --libs r_core)
R2SYSPLUGDIR=$(shell r2 -H R2_LIBR_PLUGINS)
R2USRPLUGDIR=$(shell r2 -H R2_USER_PLUGINS)
CFLAGS+=-I. -fPIC
CFLAGS+=-Wall
CFLAGS+=-g
CFLAGS+=--coverage
LDFLAGS+=--coverage

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
TESTS=tests/messages_test
TOOLS_TEST_OBJ=tests/tools_test
VDB_TEST_OBJ=tests/vdb_test
MARKDOWN_TEST_OBJ=tests/markdown_test
R2AI_CORE_TEST_OBJ=tests/r2ai_core_test

$(TESTS): tests/messages_test.c messages.o
	$(CC) $(CFLAGS) -o $@ tests/messages_test.c messages.o $(LDFLAGS)

$(TOOLS_TEST_OBJ): tests/tools_test.c tools.o messages.o markdown.o
	$(CC) $(CFLAGS) -o $@ tests/tools_test.c tools.o messages.o markdown.o $(LDFLAGS)

$(VDB_TEST_OBJ): tests/vdb_test.c vdb.o messages.o
	$(CC) $(CFLAGS) -o $@ tests/vdb_test.c vdb.o messages.o $(LDFLAGS)

$(MARKDOWN_TEST_OBJ): tests/markdown_test.c markdown.o
	$(CC) $(CFLAGS) -o $@ tests/markdown_test.c markdown.o $(LDFLAGS)

$(R2AI_CORE_TEST_OBJ): tests/r2ai_core_test.c r2ai.o messages.o tools.o vdb.o auto.o openai.o anthropic.o r2ai_http.o markdown.o
	$(CC) $(CFLAGS) -o $@ tests/r2ai_core_test.c r2ai.o messages.o tools.o vdb.o auto.o openai.o anthropic.o r2ai_http.o markdown.o $(LDFLAGS)

.PHONY: test

test: $(TESTS) $(TOOLS_TEST_OBJ) $(VDB_TEST_OBJ) $(MARKDOWN_TEST_OBJ) $(R2AI_CORE_TEST_OBJ)
	./tests/messages_test
	./$(TOOLS_TEST_OBJ)
	./$(VDB_TEST_OBJ)
	./$(MARKDOWN_TEST_OBJ)
	./$(R2AI_CORE_TEST_OBJ)

clean:
	rm -f *.o *.d $(TESTS) $(TOOLS_TEST_OBJ) $(VDB_TEST_OBJ) $(MARKDOWN_TEST_OBJ) $(R2AI_CORE_TEST_OBJ)
	rm -f *.gcda *.gcno
	rm -f tests/*.gcda tests/*.gcno
