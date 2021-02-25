#	@(#) dump.mk: 2.5 12/10/82

#	dump.mk dump makefile

ROOT =
BINDIR = $(ROOT)/bin
#
# CFLAGS should also be passed down, but this won't be done until
# all the makefiles have been standardized
#

all dump:
	-if vax; then \
		cd vax; $(MAKE) -$(MAKEFLAGS) CC="$(CC)"; \
	elif u3b; then \
		cd u3b; $(MAKE) -$(MAKEFLAGS) CC="$(CC)"; \
	else \
		echo 'Cannot make dump command: unknown target procesor.'; \
	fi



clean:
	-if vax; then \
		cd vax; $(MAKE) -$(MAKEFLAGS) clean; \
	elif u3b; then \
		cd u3b; $(MAKE) -$(MAKEFLAGS) clean; \
	fi


install: 
	-if vax; then \
		cd vax; $(MAKE) -$(MAKEFLAGS) CC="$(CC)" BINDIR=$(BINDIR) install; \
	elif u3b; then \
		cd u3b; $(MAKE) -$(MAKEFLAGS) CC="$(CC)" BINDIR=$(BINDIR) install; \
	fi

clobber: 
	-if vax; then \
		cd vax; $(MAKE) -$(MAKEFLAGS) clobber; \
	elif u3b; then \
		cd u3b; $(MAKE) -$(MAKEFLAGS) clobber; \
	fi
