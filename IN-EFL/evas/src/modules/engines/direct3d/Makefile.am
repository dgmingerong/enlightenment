
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-I$(top_srcdir)/src/modules/engines \
@EVAS_GENERAL_CFLAGS@ \
@FREETYPE_CFLAGS@ \
@PIXMAN_CFLAGS@ \
@FRIBIDI_CFLAGS@ \
@evas_engine_direct3d_cflags@

if BUILD_ENGINE_DIRECT3D

DIRECT3D_SOURCES = \
evas_engine.c \
evas_direct3d_context.cpp \
evas_direct3d_device.cpp \
evas_direct3d_image_cache.cpp \
evas_direct3d_main.cpp \
evas_direct3d_object.cpp \
evas_direct3d_object_font.cpp \
evas_direct3d_object_image.cpp \
evas_direct3d_object_line.cpp \
evas_direct3d_object_rect.cpp \
evas_direct3d_scene.cpp \
evas_direct3d_shader_pack.cpp \
evas_direct3d_vertex_buffer_cache.cpp

DIRECT3D_LIBADD = @evas_engine_direct3d_libs@


includes_HEADERS = Evas_Engine_Direct3D.h
includesdir = $(includedir)/evas-@VMAJ@

if !EVAS_STATIC_BUILD_DIRECT3D

pkgdir = $(libdir)/evas/modules/engines/direct3d/$(MODULE_ARCH)
pkg_LTLIBRARIES = module.la
module_la_SOURCES = $(DIRECT3D_SOURCES)
module_la_CXXFLAGS = -fno-exceptions
module_la_LIBADD = $(top_builddir)/src/lib/libevas.la $(DIRECT3D_LIBADD) @EVAS_GENERAL_LIBS@
module_la_LDFLAGS = @lt_enable_auto_import@ -no-undefined -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

else

noinst_LTLIBRARIES = libevas_engine_direct3d.la

libevas_engine_direct3d_la_SOURCES = $(DIRECT3D_SOURCES)
libevas_engine_direct3d_la_LIBADD = $(DIRECT3D_LIBADD)

endif
endif

EXTRA_DIST = \
array.h \
evas_direct3d_context.h \
evas_direct3d_device.h \
evas_direct3d_image_cache.h \
evas_direct3d_object.h \
evas_direct3d_object_font.h \
evas_direct3d_object_image.h \
evas_direct3d_object_line.h \
evas_direct3d_object_rect.h \
evas_direct3d_scene.h \
evas_direct3d_shader_pack.h \
evas_direct3d_vertex_buffer_cache.h \
evas_engine.h \
ref.h
