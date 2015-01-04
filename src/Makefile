

include Makefile.config

# build order.
DIRS = lib util collection thread pre nukes bouncer who

all: build size

build:
	@echo Building ..
	@for x in $(DIRS); do \
		(cd $$x; make) \
	done

size:
	@echo Build objects ..
	@find . -type f -name *.o | xargs size

clean:
	for x in $(DIRS); do \
		(cd $$x; make clean) \
	done
