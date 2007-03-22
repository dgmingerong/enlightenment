#! /bin/sh

echo "Running aclocal..."; aclocal $ACLOCAL_FLAGS || exit 1
echo "Running libtoolize..."; (libtoolize --copy --automake || glibtoolize --automake) || exit 1
echo "Running automake..."; automake --gnu --add-missing --copy || exit 1
echo "Running autoconf..."; autoconf || exit 1

if [ -z "$NOCONFIGURE" ] ; then
  ./configure "$@"
fi
