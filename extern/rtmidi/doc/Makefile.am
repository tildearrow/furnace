
MAINTAINERCLEANFILES=Makefile.in 

CLEANFILES=doxygen-build.stamp

DOX=Doxyfile

EXTRA_DIST=html release.txt

INSTIMAGES=html/doxygen.png

DOC_STAMPS=doxygen-build.stamp

DOC_DIR=$(HTML_DIR)

all-local: doxygen-build.stamp

doxygen-build.stamp: doxygen/$(DOX) $(top_srcdir)/RtMidi.h $(top_srcdir)/rtmidi_c.h
	@echo '*** Running doxygen ***'
	cd doxygen; $(DOXYGEN) $(DOX)
	touch doxygen-build.stamp

clean-local:
	rm -f *~ *.bak $(DOC_STAMPS) || true
	if test -d html; then rm -fr html; fi
	if test -d latex; then rm -fr latex; fi
	if test -d man; then rm -fr man; fi

distclean-local: clean
	rm -f *.stamp || true
	if test -d html; then rm -rf html; fi

html-local: $(DOC_STAMPS)
