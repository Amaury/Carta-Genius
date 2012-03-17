.PHONY: default cygwin macosx clean all cygall

default:
	if [ -f libpdf.a.linux ]; then \
		cp libpdf.a.linux libpdf.a; \
	else \
		if [ -f libpdf.a.linux.bz2 ]; then \
			bunzip2 --keep libpdf.a.linux.bz2; \
			mv libpdf.a.linux libpdf.a; \
		else \
			@echo "Unable to find PDFLib."; \
		fi; \
	fi
	if [ -f liby.a.linux ]; then \
		cp liby.a.linux liby.a; \
	else \
		if [ -f liby.a.linux.bz2 ]; then \
			bunzip2 --keep liby.a.linux.bz2; \
			mv liby.a.linux liby.a; \
			cd y; \
			make headers; \
		else \
			cd y; \
			make; \
		fi; \
	fi

cygwin:
	if [ -f cygwin1.dll ]; then \
		echo "cygwin1.dll file found."; \
	else \
		if [ -f cygwin1.dll.bz2 ]; then \
			bunzip2 --keep cygwin1.dll.bz2; \
		else \
			echo "Unable to find cygwin1.dll file."; \
		fi; \
	fi
	if [ -f libpdf.a.windows ]; then \
		cp libpdf.a.windows libpdf.a; \
	else \
		if [ -f libpdf.a.windows.bz2 ]; then \
			bunzip2 --keep libpdf.a.windows.bz2; \
			mv libpdf.a.windows libpdf.a; \
		else \
			echo "Unable to find PDFLib."; \
		fi; \
	fi
	if [ -f liby.a.windows ]; then \
		cp liby.a.windows liby.a; \
	else \
		if [ -f liby.a.windows.bz2 ]; then \
			bunzip2 --keep liby.a.windows.bz2; \
			mv liby.a.windows liby.a; \
			cd y; \
			make headers; \
		else \
			cd y; \
			make cygwin; \
		fi; \
	fi

macosx:
	if [ -f libpdf.a.macosx ]; then \
		cp libpdf.a.macosx libpdf.a; \
	else \
		if [ -f libpdf.a.macosx.bz2 ]; then \
			bunzip2 --keep libpdf.a.macosx.bz2; \
			mv libpdf.a.macosx libpdf.a; \
		else
			@echo "Unable to find PDFLib."; \
		fi; \
	fi
	if [ -f liby.a.macosx ]; then
		cp liby.a.macosx liby.a; \
	else \
		if [ -f liby.a.macosx.bz2 ]; then \
			bunzip2 --keep liby.a.macosx.bz2; \
			mv liby.a.macosx liby.a; \
			cd y; \
			make headers; \
		else \
			cd y; \
			make; \
		fi; \
	fi

clean:
	rm -f *~
	cd y; make clean

all: clean default

cygall: clean cygwin
