AUTOMAKE_OPTIONS=-Wno-portability
GIT_COMMIT = $(shell git describe)
GIT_HASH = $(shell git log -n 1 --pretty=format:%H)

INCLUDES = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/widgets \
	@GTK_CFLAGS@ \
	@DEPRECATED_CFLAGS@ \
	@GTKGL_CFLAGS@ \
	@LIBGLADE_CFLAGS@ \
	@MEGATUNIX_DEFINES@ -DGIT_HASH=\"$(GIT_HASH)\" -DGIT_COMMIT=\"$(GIT_COMMIT)\"

bin_PROGRAMS = raspexi

raspexi_SOURCES = \
	Makefile \
	main.c \
	dashboard.c \
	powerfc.c \
	../src/listmgmt.c \
	../src/widgetmgmt.c \
	../src/stringmatch.c \
	../src/keyparser.c \
	../src/serialio.c \
	helpers.c

raspexi_DEPENDENCIES = @WIN32_RESOURCES@  ../widgets/libmtxgauge.la ../mtxcommon/libmtxcommon.la

../widgets/libmtxgauge.la: ../mtxcommon/libmtxcommon.la
	cd ../widgets && make

../mtxcommon/libmtxcommon.la:
	cd ../mtxcommon && make

raspexi_LDADD = @GTK_LIBS@ @GTKGL_LIBS@ @LIBGLADE_LIBS@ -lfl -lcurl ../mtxcommon/libmtxcommon.la ../widgets/libmtxgauge.la  @EXTRA_LIBS@ @WIN32_RESOURCES@ @PROF_FLAGS@

raspexi_LDFLAGS = $(LTLIBINTL)

COMMON_FLAGS = @PROF_FLAGS@ @DEBUG_FLAGS@ 

AM_CFLAGS += $(COMMON_FLAGS) 

CLEANFILES = *~ core gmon.out nohup.out 

EXTRA_DIST = README.md LICENSE AUTHORS raspexi.cfg run.sh package.sh Dashboards Gauges

