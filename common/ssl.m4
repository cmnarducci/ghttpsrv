AC_DEFUN([CN_LIB_SSL],
[
   _root="undefined"
   AC_ARG_ENABLE(ssl,[  --disable-ssl           Do not use the SSL library],,enable_ssl=yes)
   if [[ "$enable_ssl" = "yes" ]]; then
      AC_ARG_WITH(ssl, AS_HELP_STRING(--with-ssl=PATH, [alternate location of SSL headers and libs]),
      [
         if test "$withval" = "yes"; then
            places=search
         else
            places="$withval"
            _root=""
         fi
      ],
      [
         places=search
      ])

      if test "$places" = "search"; then
         places="std /usr/local /usr/local/ssl /opt/local /opt/local/ssl /usr/local/opt/openssl"
      fi

      found=not

      if test "$_root" = "undefined"; then
         if test "$CN_SYSTEM" = "MINGW" -o "$CN_SYSTEM" = "MIPSEL" -o "$CN_SYSTEM" = "ARM"; then
            CN_CPP="${host_alias}-cpp -print-sysroot"
            _root="$($CN_CPP)"
         elif test "$CN_SYSTEM" = "ARMEB"; then
            CN_CPP="${host_alias}-cpp -print-search-dirs"
            _root="$($CN_CPP | grep ^libraries | cut -d: -f2 | sed -s '/^.*=/s///' | sed -s '/\/usr\/.*/s///')"
         elif test "$CN_SYSTEM" = "Darwin"; then
            _root="/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk"
         else
            _root=""
         fi
      fi
      for place in $places; do
         case "$place" in
            "std")
               header=${_root}/usr/include
               lib=${_root}/usr/lib
               description="the standard places"
               ;;
            *":"*)
               header=${_root}$(echo $place | sed -e 's/:.*$//')
               lib=${_root}$(echo $place | sed -e 's/^.*://')
               description="$header and $lib"
               ;;
            *)
               header=${_root}$place/include
               lib=${_root}$place/lib
               description="${_root}$place"
               ;;
         esac

         AC_MSG_CHECKING([for SSL in $description])
         subdir=""
         if test -f $header/openssl/opensslv.h; then
            AC_MSG_RESULT(yes)
            found="$place"
            subdir=""
            break
         fi
         AC_MSG_RESULT(no)
         test "$found" != "not" && break

      done

      case "$found" in
         "not")
            AC_MSG_ERROR([Could not find SSL - Try using --with-ssl=PATH or --disable-ssl])
            ;;
         *)
            if test "$found" != "std"; then
               if test "$CN_SYSTEM" != "Darwin" -a "$CN_SYSTEM" != "HP-UX"; then
                  LDFLAGS="${LDFLAGS} -Wl,-rpath,$lib "
               fi
               SSL_LIBS="-L$lib -lssl -lcrypto"
            else
               SSL_LIBS="-lssl -lcrypto"
            fi
            if test "$found" != "std" -o "$subdir" != ""; then
               SSL_CFLAGS="-I${header}${subdir}"
            fi
            if test "$CN_SYSTEM" = "MINGW"; then
               SSL_LIBS="${SSL_LIBS} -lz -lws2_32"
            fi
            ;;
      esac
      AC_SUBST(SSL_CFLAGS)
      AC_SUBST(SSL_LIBS)
      AC_SUBST(LDFLAGS)
      AC_DEFINE([HAVE_SSL],[1],[Define to 1 if you have ssl Support])
   fi
])
