# ghttpsrv

Light HTTP/HTTPS Server with GUI

## Description

A light HTTP/HTTPS Server to expose local files for download, with file upload support.

## Build

Program can be compiled without SSL support with the `--disable-ssl` configure option.

Supported OS:
* Windows (using cygwin GCC cross compiler)
* Linux
* MacOS

### Dependencies

* wxWidgets version >= 3.0.0
* OpenSSL v1 or v3 (if compiled with SSL support)

## Other infos

When SSL is enabled, a certificate and private key can be specified, otherwise a default embedded certificate is used.
Authentication methods supported are basic (user and password) and with certificate (if SSL is enabled).

Special urls:
* `/user` to show current authenticated user
* `/send` to upload a file to server (a folder "uploads" will be created in the root direcetory)

## Version History

* 3.0
    * Initial Release
