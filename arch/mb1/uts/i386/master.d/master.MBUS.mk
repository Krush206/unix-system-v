#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#	Copyright (c) 1988  Intel Corporation
#	All Rights Rreserved
#
#	INTEL CORPORATION PROPRIETARY INFORMATION
#
#	This software is supplied to AT & T under the terms of a license 
#	agreement with Intel Corporation and may not be copied nor 
#	disclosed except in accordance with the terms of that agreement.


#ident	"@(#)mb1:uts/i386/master.d/master.MBUS.mk	1.3.1.1"

BUS= 
ROOT=
CONF = $(ROOT)/etc/conf
LCPP = $(CPP) -P $(MORECPP) 

MB1MODS =	\
	i214	\
	i214tp	\
	i546	\
	console	\
	dma	\
	i8251   \
	iasy	\
	clock 	\
	lma	\
	clone	\
	bps	\
	ramd

all:
	-@ if [ -f master.$(ARCH).mk ]					;\
	then                                                             \
	echo "\nDoing MB1 modules"					;\
	for i in $(MB1MODS)				 		;\
	do							         \
	    [ -d $(CONF)/pack.d/$$i ] || mkdir $(CONF)/pack.d/$$i	;\
	    cd $$i							;\
	    echo "$$i \c"						;\
	    [ -f *.c ]   && cp *.c $(CONF)/pack.d/$$i			;\
    	    [ -f mdev ] && $(LCPP) mdev |				\
		grep -v "^[\*#]ident" >> $(CONF)/cf.d/mdevice		;\
	    [ -f sdev ] && $(LCPP) sdev |				\
		grep -v "^[\*#]ident" > $(CONF)/sdevice.d/$$i 		;\
	    [ -f mfsys ] && $(LCPP) mfsys |				\
		grep -v "^[\*#]ident" > $(CONF)/mfsys.d/$$i		;\
	    [ -f sfsys ] && $(LCPP) sfsys |				\
		grep -v "^[\*#]ident" > $(CONF)/sfsys.d/$$i 		;\
	    [ -f node ] && $(LCPP) node |				\
		grep -v "^[\*#]ident" > $(CONF)/node.d/$$i 		;\
	    [ -f init ] && $(LCPP) init |				\
		grep -v "^[\*#]ident" > $(CONF)/init.d/$$i 		;\
	    [ -f $(CONF)/pack.d/$$i/* ] && chmod +w $(CONF)/pack.d/$$i/*;\
	    cd .. 							;\
	done								 \
	fi

clean:

clobber:	clean
	for i in $(MB1MODS) 		;\
	do								 \
		rm -rf $(CONF)/pack.d/$$i				;\
	done

