CXXFLAGS:=-O2 -g -Wall -Werror -std=c++11 -Iinclude
LDFLAGS:=-g

all: check examples/huge

check: examples/main
	for t in tests/test*; do \
	    echo "==== Running $$t ====" ;\
	    $$t ;\
	    ret=$$? ;\
	    if [ $$ret -ne 0 ]; then \
	    	echo "**** FAILED with code $$ret ****" ;\
		exit $$ret ;\
	    fi ;\
	    echo ==== Done ==== ;\
	done

examples/main: examples/main.cpp include/*.hpp

examples/huge: examples/huge.cpp include/*.hpp

clean:
	$(RM) examples/main examples/huge

.PHONY: all check clean
