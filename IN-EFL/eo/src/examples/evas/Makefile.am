
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_builddir)/src/lib \
@EFL_EO_BUILD@ \
@EO_CFLAGS@ \
@ELM_CFLAGS@

EXTRA_PROGRAMS =

if EO_BUILD_EXAMPLE_EVAS

EXTRA_PROGRAMS += evas

examples: evas

evas_SOURCES = \
elw_box.c \
elw_box.h \
elw_boxedbutton.c \
elw_boxedbutton.h  \
elw_button.c \
elw_button.h \
elw_win.h \
elw_win.c \
evas_obj.c \
evas_obj.h \
test.c

evas_LDADD = $(top_builddir)/src/lib/libeo.la @ELM_LIBS@ @EO_LIBS@

else

examples:

endif
