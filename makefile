
WINEPATH=/usr/local
INSTALLPATH=$(WINEPATH)

VERSION_MAJOR=0
VERSION_MINOR=3
VERSION_MINOR_MINOR=1

VERSION=-DVERSION_MAJOR=$(VERSION_MAJOR) -DVERSION_MINOR=$(VERSION_MINOR) -DVERSION_MINOR_MINOR=$(VERSION_MINOR_MINOR)

CFLAGS_BASE=-Wall -Iinclude -Ik_various/include -I/site/include -I/usr/local/include -g -fPIC -D_REENTRANT
CFLAGS=$(CFLAGS_BASE) 
CFLAGS_NOWERROR=$(CFLAGS_BASE)
LDFLAGS=-lm

SD=servant/
LD=library/
KD=k_various/
CD=common/

COMMONOBJS_NOSTAT=k_communicate.o k_locks.o
COMMONOBJS=$(COMMONOBJS_NOSTAT) cache.o
SERVEROBJS=mainhandler.o clienthandler.o controlthread.o processthread.o shmhandler.o s_process.o s_processreplace.o s_setparameter.o s_getparameter.o s_dispatcher.o s_audioMaster.o s_effProcessEvents.o $(COMMONOBJS)
LIBOBJS_NOSTAT=newdelete.o setparameter.o getparameter.o dispatcher.o process.o processreplacing.o processing.o l_effProcessEvents.o $(COMMONOBJS_NOSTAT)
LIBOBJS=$(LIBOBJS_NOSTAT) cache.o newdeletecache.o


all:  include/vst/aeffectx.h vstserver vstservant.so libvst.a libvst_nostat.a clienttest
	@echo ""
	@echo "Compilation Ok."


install: all
	cp -f vstservant.so ${VST_PATH}/vstservant.so
	cp -f vstserver ${VST_PATH}/vstserver

	cp -af include/vst $(INSTALLPATH)/include/
	cp -f include/vstlib.h $(INSTALLPATH)/include/
	cp -f libvst.a $(INSTALLPATH)/lib/
	cp -f vstserver.sh $(INSTALLPATH)/bin/vstserver


uninstall:
	rm -f ${VST_PATH}/vstservant.so
	rm -f ${VST_PATH}/vstserver

	rm -fr $(INSTALLPATH)/include/vst
	rm -f $(INSTALLPATH)/include/vstlib.h
	rm -f $(INSTALLPATH)/lib/libvst.a
	rm -f $(INSTALLPATH)/bin/vstserver


doc: doc/html/index.html



clean:
	rm -f *.o */*.o */*/*.o *~ */*~ */*/*~ makefile~ */\#* */*/\#
	rm -f vstserver clienttest *.so *.a include/vst/aeffectx.h
	rm -fr doc/html doc/latex
	echo "\
	cd servant/win;make clean;\
	rm -f Make.rules Make.rules.in Makefile Makefile.in wineapploader.in;\
	rm -f configure config.log config.cache config.status configure.ac\
	" > makevstserver.sh
	sh makevstserver.sh
	rm makevstserver.sh


vstservant.so: $(SD)win/win.exe.so
	cp $(SD)win/win.exe.so vstservant.so

$(SD)win/win.exe.so: $(SD)win/Makefile $(SD)win/windowsstuff.c $(SD)win/main.c $(SD)win/winwin.c libvstservant.a
	echo "cd servant/win;make" >makevstserver.sh
	touch $(SD)win/dummy.c
	sh makevstserver.sh
	rm makevstserver.sh


#sed s/-z,defs,// <Makefile >m2;\
#mv -f m2 Makefile\

$(SD)win/Makefile:
	echo "\
	cd servant/win;\
	${WINEPATH}/bin/winemaker --lower-none --nolower-include --nosource-fix --nobanner --nobackup -I../../include -I${WINEPATH}/include/wine/windows -L../.. -lvstservant -L/usr/local/lib -L/site/lib .;\
	./configure  --with-wine=${WINEPATH}\
	" >makevstserver.sh
	sh makevstserver.sh
	rm makevstserver.sh


libvstservant.a: $(SERVEROBJS)
	ar rus libvstservant.a $(SERVEROBJS)

vstserver: vstserver.o libvst.a
	gcc -o vstserver vstserver.o -L. -lvst

clienttest: clienttest.o libvst.a
	gcc -o clienttest clienttest.o $(LDFLAGS) -L. -lvst

#libvst.so: $(LIBOBJS)
#	gcc -shared -o libvst.so $(LIBOBJS)

libvst.a: $(LIBOBJS)
	ar rus libvst.a $(LIBOBJS)

libvst_nostat.a: $(LIBOBJS_NOSTAT) 
	ar rus libvst_nostat.a $(LIBOBJS_NOSTAT)


include/vst/aeffectx.h: include/aeffectx.diff
	echo "\
	cd include/vst;\
	cp ../aeffectx_org.h aeffectx.h;\
	patch -p0 <../aeffectx.diff\
	" >makevstserver.sh
	sh makevstserver.sh
	rm makevstserver.sh

doc/html/index.html: doc/reference.doxygen include/vstlib.h
	echo "\
	cd doc;\
	make\
	" >makevstserver.sh
	sh makevstserver.sh
	rm makevstserver.sh

##############



k_communicate.o: $(KD)k_communicate.c
	gcc -c $(KD)k_communicate.c $(CFLAGS)
cache.o: $(CD)cache.c
	gcc -c $(CD)cache.c $(CFLAGS)
k_locks.o: $(KD)k_locks.c
	gcc -c $(KD)k_locks.c $(CFLAGS)

clienthandler.o: $(SD)clienthandler.c
	gcc -c $(SD)clienthandler.c $(CFLAGS)

vstserver.o: server/vstserver.c
	gcc -c server/vstserver.c $(CFLAGS) $(VERSION)

clienttest.o: tests/clienttest.c
	gcc -c tests/clienttest.c $(CFLAGS)

controlthread.o: $(SD)controlthread.c
	gcc -c $(SD)controlthread.c $(CFLAGS)
processthread.o: $(SD)processthread.c
	gcc -c $(SD)processthread.c $(CFLAGS)

#windowsstuff.o: $(SD)win/windowsstuff.c
#	gcc -c $(SD)win/windowsstuff.c $(CFLAGS)
shmhandler.o: $(SD)shmhandler.c
	gcc -c $(SD)shmhandler.c $(CFLAGS)
s_process.o: $(SD)s_process.c
	gcc -c $(SD)s_process.c $(CFLAGS)
s_processreplace.o: $(SD)s_processreplace.c
	gcc -c $(SD)s_processreplace.c $(CFLAGS)
s_setparameter.o: $(SD)s_setparameter.c
	gcc -c $(SD)s_setparameter.c $(CFLAGS)
s_getparameter.o: $(SD)s_getparameter.c
	gcc -c $(SD)s_getparameter.c $(CFLAGS)
s_dispatcher.o: $(SD)s_dispatcher.c
	gcc -c $(SD)s_dispatcher.c $(CFLAGS)
s_audioMaster.o: $(SD)s_audioMaster.c
	gcc -c $(SD)s_audioMaster.c $(CFLAGS_NOWERROR)
mainhandler.o: $(SD)mainhandler.c
	gcc -c $(SD)mainhandler.c $(CFLAGS)
s_effProcessEvents.o: $(SD)s_effProcessEvents.c
	gcc -c $(SD)s_effProcessEvents.c $(CFLAGS)

#.o: $(SD).c

#.o: $(SD).c
#	gcc -c $(SD).c $(CFLAGS)
#.o: $(SD).c

#.o: $(SD).c
#	gcc -c $(SD).c $(CFLAGS)
#.o: $(SD).c
#	gcc -c $(SD).c $(CFLAGS)
#.o: $(SD).c
#	gcc -c $(SD).c $(CFLAGS)


newdelete.o: $(LD)newdelete.c
	gcc -c $(LD)newdelete.c $(CFLAGS) $(VERSION)


setparameter.o: $(LD)setparameter.c
	gcc -c $(LD)setparameter.c $(CFLAGS)
dispatcher.o: $(LD)dispatcher.c
	gcc -c $(LD)dispatcher.c $(CFLAGS)
getparameter.o: $(LD)getparameter.c
	gcc -c $(LD)getparameter.c $(CFLAGS)
process.o: $(LD)process.c
	gcc -c $(LD)process.c $(CFLAGS)
processreplacing.o: $(LD)processreplacing.c
	gcc -c $(LD)processreplacing.c $(CFLAGS)
processing.o: $(LD)processing.c
	gcc -c $(LD)processing.c $(CFLAGS)
l_effProcessEvents.o: $(LD)l_effProcessEvents.c
	gcc -c $(LD)l_effProcessEvents.c $(CFLAGS)

newdeletecache.o: $(LD)newdeletecache.c
	gcc -c $(LD)newdeletecache.c $(CFLAGS)

#.o: $(LD).c
#	gcc -c $(LD).c $(CFLAGS)

#.o: $(LD).c
#	gcc -c $(LD).c $(CFLAGS)
#.o: $(LD).c
#	gcc -c $(LD).c $(CFLAGS)

#.o: $(LD).c
#	gcc -c $(LD).c $(CFLAGS)
#.o: $(LD).c
#	gcc -c $(LD).c $(CFLAGS)

#.o: $(LD).c
#	gcc -c $(LD).c $(CFLAGS)
#.o: $(LD).c
#	gcc -c $(LD).c $(CFLAGS)

