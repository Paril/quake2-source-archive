DOMC
The Document Object Model in C
http://users.erols.com/mballen/domc/

DOMC is a light weight C implementation of the DOM as specified in
the W3C Document Object Model Level 1, Level 2, and Level 2 Events
recommendations. The DOM is a popular API for manipulating XML and HTML
documents as a tree of nodes in memory. It is the more sophisticated but
more memory constraining alternative to the SAX API. This implementation
is not W3C compliant because it lacks support for entity references,
DOCTYPE nodes, DTD default values, and other peripheral functionality. The
DOM_Node type and it's associated operations should work well however
because what functionality is supported has been tested with the DOM
Conformace Test Suite. A simple serialization module that uses Expat
is provided. As of version 0.6.0, DOMC will use the locale dependent
multi-byte encoding such as UTF-8 for the DOM_String type.

INSTALLATION

The libmba library is required. Download and install the RPM or tar.gz from:

  http://users.erols.com/mballen/libmba/

Now either install the DOMC RPM or tar.gz. In the later case unpack
the tar.gz in an appropriate location (e.g. /usr/local/src/) and change
the prefix (default is /usr/local/) in the Makefile if necessary. Now
just run:

  # make
  # make install

For i18n support the encdec package is required but the encdec package
requires the __STDC_ISO_10646__ environment which is not supported on
many platforms. The only platform that I am sure of is GNU/Linux with
glibc 2.2. To enable i18n support, install encdec, add -DUSE_ENCDEC to
the CFLAGS in the Makefile, and rebuild the package. In theory it would
not be difficult to remove the encdec requirement from the expatls.c
module perhaps replacing it with ICU or plain iconv support.

USING DOMC

See the api documentation in doc/api. The definitive DOM references are
the W3C recommendations however.

COPYING

Currently all project files are freely distributable under the MIT
License. See each file for details.

Michael B. Allen <mballen@erols.com>

