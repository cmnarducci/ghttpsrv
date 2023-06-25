AC_DEFUN([CN_DEFINES],
[
   CN_SYSTEM=$(echo $host_alias | grep mingw | sed 's/.*mingw.*/MINGW/')
   if test "$CN_SYSTEM" = ""; then
      CN_SYSTEM=$(echo $host_alias | grep mipsel | sed 's/.*mipsel.*/MIPSEL/')
   fi
   if test "$CN_SYSTEM" = ""; then
      CN_SYSTEM=$(echo $host_alias | grep powerpc | sed 's/.*powerpc.*/PPC/')
   fi
   if test "$CN_SYSTEM" = ""; then
      CN_SYSTEM=$(echo $host_alias | grep arm | sed 's/.*arm.*/ARM/')
   fi
   if test "$CN_SYSTEM" = ""; then
      CN_SYSTEM=$(uname -s 2> /dev/null | sed 's/_NT.*$//')
   fi
   case "$CN_SYSTEM" in
      "MIPSEL")
         CN_DEFS="$CN_DEFS -DMIPSEL"
         CXXFLAGS="$CXXFLAGS -pthread"
         LIBS="$LIBS -lpthread"
         ;;
      "PPC")
         CN_DEFS="$CN_DEFS -DPPC"
         CXXFLAGS="$CXXFLAGS -pthread"
         LIBS="$LIBS -lpthread"
         ;;
      "ARM")
         CN_DEFS="$CN_DEFS -DARM"
         CXXFLAGS="$CXXFLAGS -pthread"
         LIBS="$LIBS -lpthread"
         ;;
      "SunOS" )
         CN_DEFS="$CN_DEFS -DSOLARIS"
         CXXFLAGS="$CXXFLAGS -pthreads"
         LIBS="$LIBS -lpthread"
         ;;
      "HP-UX" )
         CN_DEFS="$CN_DEFS -DHPUX"
         CXXFLAGS="$CXXFLAGS -pthread"
         LIBS="$LIBS -lpthread"
         ;;
      "Darwin" )
         CN_DEFS="$CN_DEFS -DMACOS"
         ;;
      "CYGWIN" )
         CN_DEFS="$CN_DEFS -DCYGWIN"
         LIBS="$LIBS -lpthread"
         ;;
      "MINGW" )
         CN_DEFS="$CN_DEFS -DMINGW -D_WIN32_WINNT=0x0600"
         LIBS="$LIBS -lpthread"
         LDFLAGS="$LDFLAGS -static -Wl,--allow-multiple-definition"
         ;;
      * )
         CXXFLAGS="$CXXFLAGS -pthread"
         LIBS="$LIBS -lpthread"
         ;;
   esac
   CXXFLAGS="$CXXFLAGS -Wall"
   CFLAGS="$CFLAGS -Wall"
   AC_DEFINE(HAVE_PTHREAD,1,[Define if you have POSIX threads libraries and header files.])
   AC_SUBST(CN_SYSTEM)
   AC_SUBST(CN_DEFS)
])
