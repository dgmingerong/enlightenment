ACLOCAL_AMFLAGS = -I m4
CLEANFILES =
MAINTAINERCLEANFILES =
EXTRA_DIST =

SUBDIRS = doc

AM_MAKEFLAGS = --no-print-directory
AM_CFLAGS = \
	-include $(top_builddir)/config.h \
	-I$(top_srcdir)/src/lib \
	@ECORE_CFLAGS@ \
	@DBUS_CFLAGS@

AM_CPPFLAGS = -DEFL_EDBUS_BUILD=1

includedir = @includedir@/edbus-@VMAJ@/

SED_PROCESS = \
	$(AM_V_GEN)$(MKDIR_P) $(dir $@) && $(SED) \
	-e 's,@VERSION\@,$(VERSION),g' \
	-e 's,@prefix\@,$(prefix),g' \
	-e 's,@exec_prefix\@,$(exec_prefix),g' \
	-e 's,@libdir\@,$(libdir),g' \
	-e 's,@includedir\@,$(includedir),g' \
	< $< > $@ || rm $@

%.pc: %.pc.in Makefile
	$(SED_PROCESS)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = edbus.pc
CLEANFILES += edbus.pc
EXTRA_DIST += edbus.pc.in

EXTRA_DIST += \
	m4/efl_doxygen.m4 \
	m4/efl_compiler_flag.m4


MAINTAINERCLEANFILES += \
	aclocal.m4 \
	compile \
	config.guess \
	config.h.in \
	config.sub \
	configure \
	depcomp \
	install-sh \
	ltmain.sh \
	Makefile.in \
	missing \
	mkinstalldirs

lib_LTLIBRARIES = libedbus.la

include_HEADERS = \
	src/lib/EDBus.h \
	src/lib/edbus_connection.h \
	src/lib/edbus_freedesktop.h \
	src/lib/edbus_message.h \
	src/lib/edbus_object.h \
	src/lib/edbus_pending.h \
	src/lib/edbus_proxy.h \
	src/lib/edbus_service.h \
	src/lib/edbus_signal_handler.h

libedbus_la_LIBADD = @ECORE_LIBS@ @DBUS_LIBS@
libedbus_la_SOURCES = \
	src/lib/edbus_private.h \
	src/lib/edbus_private_types.h \
	src/lib/edbus_proxy.c \
	src/lib/edbus_core.c \
	src/lib/edbus_message.c \
	src/lib/edbus_object.c \
	src/lib/edbus_pending.c \
	src/lib/edbus_freedesktop.c \
	src/lib/edbus_service.c \
	src/lib/edbus_signal_handler.c

noinst_PROGRAMS = \
	src/examples/connman-list-services \
	src/examples/ofono-dial \
	src/examples/banshee \
	src/examples/complex_types \
	src/examples/complex_types_server \
	src/examples/server \
	src/examples/client

EXAMPLES_LIBS = libedbus.la @ECORE_LIBS@

src_examples_connman_list_services_SOURCES = \
	src/examples/connman-list-services.c
src_examples_connman_list_services_LDADD = $(EXAMPLES_LIBS)

src_examples_ofono_dial_SOURCES = src/examples/ofono-dial.c
src_examples_ofono_dial_LDADD = $(EXAMPLES_LIBS)

src_examples_banshee_SOURCES = src/examples/banshee.c
src_examples_banshee_LDADD = $(EXAMPLES_LIBS)

src_examples_complex_types_SOURCES = src/examples/complex_types.c
src_examples_complex_types_LDADD = $(EXAMPLES_LIBS)

src_examples_complex_types_server_SOURCES = src/examples/complex_types_server.c
src_examples_complex_types_server_LDADD = $(EXAMPLES_LIBS)

src_examples_server_SOURCES = src/examples/server.c
src_examples_server_LDADD = $(EXAMPLES_LIBS)

src_examples_client_SOURCES = src/examples/client.c
src_examples_client_LDADD = $(EXAMPLES_LIBS)

.PHONY: doc

# Documentation

doc:
	@echo "entering doc/"
	make -C doc doc