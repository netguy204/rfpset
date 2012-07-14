require 'mkmf'

# $CFLAGS << ' -g -pg -ggdb '
# $LDFLAGS << ' -g -pg '

#have_library('zlib', 'zlibVersion')
$LDFLAGS << ' -lz '
create_makefile('rfpset/rfpset')
