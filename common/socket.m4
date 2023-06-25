AC_DEFUN([CN_LIB_SOCKET],
[
   if test "$CN_SYSTEM" = "MINGW"; then
      SOCKET_LIBS="-lws2_32 -liphlpapi"
   elif test "$CN_SYSTEM" = "SunOS"; then
      SOCKET_LIBS="-lnsl -lsocket"
   fi
   AC_SUBST(SOCKET_LIBS)
])
