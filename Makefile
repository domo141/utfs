
.DEFAULT:
	 $(MAKE) -f Makefile.build $(GOAL) OD=$(OD) DEBUG=$(DEBUG) DEP=$(DEP)

ohje:
ohje:
	 @sed -n 's/^#+#//p' Makefile

#+#
#+# Enter
#+#   'make client'   to compile utfs client
#+#   'make server'   to compile utfs server
#+#   'make all'      to compile client and server (unix versions)
#+#   'make debug'    to compile debug versions of the above
#+#
#+#   'make utfs-server.exe' to compile windows server
#+#
#+#  Client requires FUSE headers and libraries and C99 compiler. 
#+#  Server does not (see devel/genxtypes.sh for missing types, if any).
#+#

# No implicit rules (at least these)
.SUFFIXES:
% : %.o
% : %.c
%.o : %.c

OD=_obj_ux
DEBUG=0
DEP=_depend_ux
GOAL=$@

debug: GOAL= __debug
debug: DEBUG=1
debug: OD=_obj_dbg
debug: DEP=_depend_dbg

server.exe utfs-server.exe: OD=_obj_w32
server.exe utfs-server.exe: DEP=_depend_w32

snap=-xxx

snapshot: snap=-`date +%Y%m%d`
snapshot: _dist

release: snap=
release: _dist

_dist:
	eval `sh version.h`; \
	version=utfs-$$version$(snap); { echo 755 root root . $$version /; \
	grep '^#,#' Makefile | while read _ f x; do p=755; d=$$f; \
		test -d $$f && d=/ || case $$x in '') p=644;; esac; \
		echo $$p root root . $$version/$$f $$d; done; } \
	| tarlisted -Vz -o $$version.tar.gz

##,#  ChangeLog
#,#  README
#,#  Makefile
#,#  Makefile.build
#,#  output_server_errmap.pl
#,#  amiga_list.c
#,#  amiga_list.h
#,#  defs.h
#,#  pretocol.c
#,#  pretocol.h
#,#  simplexdr.c
#,#  simplexdr.h
#,#  sockfd.h
#,#  utfs-buffer.c
#,#  utfs-buffer.h
#,#  utfs.c
#,#  utfs-client.c
#,#  utfs-client-errmap.c
#,#  utfs-client-fuse.c
#,#  utfs-errmaps.h
#,#  utfs.h
#,#  utfs-server.c
#,#  utfs-server-fs.c
#,#  util.c
#,#  util.h
#,#  version.h
#,#  w32x.c
#,#  w32x.h
#,#  w32x_ext.h
#,#  w32x_ext.hc

#,#  devel
#,#  devel/dbgsetup.sh  x
#,#  devel/genxtypes.sh  x
#,#  devel/socketpairpipeline.pl x
#,#  devel/speedtest-ia32.c
#,#  devel/utfs-server-dbgwrapper.sh  x
#,#  devel/testoperations.sh  x
#,#  devel/testreadwrite.sh  x
#,#  devel/travis2utfs.c


clean distclean:
	touch _depend
	$(MAKE) -f Makefile.build $@
