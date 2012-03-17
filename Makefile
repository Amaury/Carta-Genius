# Documentation files (in POD format)
DOC_FILES =	doc/english/3.command-line/carta-genius_en	\
		doc/francais/3.ligne-commande/carta-genius_fr
# Executable files
EXE_FILES =	configure bin/*

.PHONY: default dynamic static cygwin macosx doc rights distrib clean cygclean docclean all cygall

# ######################################################
# #                   Default rule                     #
# ######################################################
default:
	@echo "Bad parameter. Possible options are:"
	@echo "make dynamic     create dynamically linked Carta-Genius program"
	@echo "make static      create statically linked Carta-Genius program"
	@echo "make cygwin      create Carta-Genius program under Cygwin environment"
	@echo "make macosx      create Carta-Genius program under Mac OS X"
	@echo "make doc         create documentation for CLI"
	@echo "make rights      set basic file rights"
	@echo "make distrib     create a tar.bz2 file of the cleaned archive"
	@echo "make clean       clean archive from binary files"
	@echo "make cygclean    clean archive from binary files under Cygwin"
	@echo "make docclean    clean generated documentation files"
	@echo "make all         create a fresh new dynamically linked program"
	@echo "make staticall   create a fresh new statically linked program"
	@echo "make cygall      create a fresh new program under Cygwin"
	@echo "make docall      create a fresh new documentation for CLI"

dynamic:
	cd lib; make
	cd src; make

static:
	cd lib; make
	cd src; make static

cygwin:
	cd lib; make cygwin
	cd src; make cygwin

macosx:
	cd lib; make macosx
	cd src; make macosx

doc:
	for i in $(DOC_FILES); do \
		pod2man -c "Pandocreon" -r "4.0.0" $$i.pod $$i.man; \
		pod2text $$i.pod $$i.txt; \
		pod2html --title="Carta Genius" --noindex --infile=$$i.pod --outfile=$$i.html; \
	done
	rm -f pod2htm*

rights:
	chmod 644 `find . -type f`
	chmod 755 `find . -type d` $(EXE_FILES)

distrib: clean rights
	dir=`pwd`; dir=`basename $$dir`; \
	cd ..; tar cfps $$dir.tar $$dir; bzip2 $$dir.tar

clean:
	rm -f *~
	cd lib; make clean
	cd src; make clean

cygclean:
	rm -f *~
	cd lib; make clean
	cd src; make cygclean

docclean:
	for i in $(DOC_FILES); do \
		rm -f $$i.man $$i.txt $$i.html; \
	done

all: clean dynamic

staticall: clean static

cygall: cygclean cygwin

docall: docclean doc
