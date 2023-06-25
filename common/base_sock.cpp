#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <cstring>
#include <ctime>
#include <csignal>
#include <sstream>
#ifdef SOLARIS
#include <sys/filio.h>
#endif
#include "tools.h"
#include "base_sock.h"

#if defined(HAVE_SSL) && OPENSSL_VERSION_NUMBER >= 0x10100000
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#if defined (HAVE_SSL) && defined(HAVE_PTHREAD)
pthread_rwlock_t *BaseSock::_locks = 0;
#endif

#ifdef HAVE_SSL
uint8_t BaseSock::_pk_der[] =
{
  0x30, 0x82, 0x04, 0xa3, 0x02, 0x01, 0x00, 0x02, 0x82, 0x01, 0x01, 0x00,
  0xb8, 0x90, 0x92, 0x62, 0xf6, 0x1f, 0xf0, 0x94, 0x87, 0xbb, 0xf3, 0xb4,
  0xd9, 0x0d, 0x74, 0x1f, 0xfd, 0xe9, 0xd4, 0x4f, 0xa6, 0xfa, 0xf9, 0xf4,
  0x5d, 0x9d, 0x9d, 0x92, 0xdb, 0x65, 0x12, 0x0a, 0x16, 0xd3, 0x2b, 0x10,
  0xe3, 0xff, 0x08, 0xa9, 0x68, 0x2a, 0x16, 0xdb, 0xe6, 0xa2, 0xa7, 0xab,
  0x51, 0xc2, 0xd5, 0xb8, 0x4b, 0x8b, 0xa4, 0x90, 0x5a, 0xea, 0x74, 0x5d,
  0x7f, 0xd6, 0x22, 0xf4, 0xad, 0xfe, 0x5c, 0xb5, 0x6b, 0xc4, 0x7f, 0xcc,
  0x65, 0xa9, 0xde, 0xc9, 0x27, 0x60, 0x9c, 0x7f, 0x9a, 0xbe, 0x07, 0x7f,
  0x96, 0xf2, 0x43, 0xaf, 0x39, 0xc0, 0xdb, 0x99, 0x1f, 0x7a, 0xce, 0x3a,
  0xd4, 0x6b, 0x30, 0xc2, 0xe3, 0xb9, 0xc8, 0x7f, 0xad, 0x72, 0x18, 0x6c,
  0xed, 0xb3, 0x36, 0x66, 0x54, 0x81, 0x20, 0x44, 0x25, 0xb4, 0xff, 0x7b,
  0x2a, 0x14, 0x99, 0x74, 0xc9, 0x36, 0x10, 0xbc, 0x8d, 0xb1, 0xd3, 0xf6,
  0x29, 0xb9, 0x92, 0x56, 0x7a, 0x52, 0x24, 0xf3, 0xa6, 0x3a, 0x3f, 0xe1,
  0x30, 0xde, 0x4f, 0x81, 0x2f, 0xb3, 0xc8, 0x95, 0x9d, 0x4d, 0xf0, 0xd0,
  0xdf, 0x8f, 0xd5, 0xc1, 0xb2, 0xc0, 0x09, 0x19, 0x54, 0x19, 0x8e, 0x29,
  0x44, 0x70, 0xb4, 0x9c, 0x96, 0x93, 0xaf, 0xb2, 0xeb, 0xb6, 0x50, 0x00,
  0x42, 0x35, 0x7d, 0xde, 0x7a, 0x16, 0x75, 0x6e, 0x10, 0x74, 0x2f, 0xf7,
  0x09, 0x24, 0x47, 0x4c, 0xd0, 0xca, 0x8c, 0x4d, 0x2b, 0x71, 0xf9, 0xd4,
  0x50, 0xba, 0x55, 0x7e, 0xf2, 0xe8, 0x49, 0xce, 0x72, 0xfc, 0xc0, 0xff,
  0xf7, 0x66, 0xfa, 0xdb, 0x10, 0x74, 0x2e, 0xf5, 0x18, 0x70, 0x8f, 0xcf,
  0x66, 0x26, 0x65, 0xa2, 0xd8, 0xe1, 0xe0, 0xef, 0xa8, 0x8a, 0xb2, 0xab,
  0xfc, 0x05, 0x0f, 0x0e, 0xed, 0x75, 0x3e, 0x75, 0x54, 0xa6, 0x43, 0x16,
  0x96, 0x3a, 0x7a, 0x57, 0x02, 0x03, 0x01, 0x00, 0x01, 0x02, 0x82, 0x01,
  0x00, 0x04, 0x3e, 0x76, 0xcf, 0x45, 0xbf, 0x98, 0x45, 0xa2, 0x15, 0x65,
  0x56, 0x3c, 0x43, 0x7d, 0xef, 0x1a, 0x64, 0x9d, 0x18, 0x33, 0xcb, 0xc3,
  0xa1, 0xa5, 0xe1, 0x70, 0xf5, 0x26, 0x8c, 0x5d, 0x6c, 0xd3, 0xf7, 0x89,
  0x1e, 0xfb, 0x4f, 0x2b, 0x00, 0xd2, 0x3f, 0x5d, 0x85, 0x32, 0xf9, 0x7c,
  0x87, 0x88, 0xfa, 0xc4, 0xf5, 0x56, 0xef, 0x60, 0x1a, 0x5d, 0xac, 0xd4,
  0x7e, 0x2d, 0xe9, 0x5b, 0xa6, 0x6a, 0xb5, 0xd5, 0xa5, 0xea, 0x53, 0x7b,
  0x68, 0xd4, 0xd1, 0xa2, 0x28, 0xd7, 0xf9, 0xf5, 0xbf, 0x4d, 0x6c, 0x9f,
  0x39, 0x84, 0xdc, 0x0f, 0x48, 0x61, 0x38, 0x99, 0x14, 0x1d, 0x50, 0x32,
  0x9b, 0xd1, 0xb4, 0xa1, 0x4b, 0x69, 0xf7, 0x24, 0xb0, 0x22, 0xe9, 0xe3,
  0xbb, 0xe5, 0x7f, 0x60, 0x79, 0xdb, 0xe9, 0x98, 0x37, 0x58, 0xf2, 0x9f,
  0x89, 0x52, 0x7f, 0x64, 0xc3, 0xf5, 0xb8, 0x83, 0x8b, 0xf8, 0x5b, 0x95,
  0x6b, 0xa7, 0x69, 0x7b, 0xea, 0x97, 0xda, 0xa4, 0x64, 0xc0, 0x89, 0x1b,
  0xf7, 0x8f, 0x79, 0x97, 0x5f, 0x0c, 0xe8, 0xae, 0x16, 0xc9, 0x54, 0x04,
  0x47, 0x17, 0x6b, 0x7b, 0xb2, 0x76, 0xb4, 0x5c, 0xc1, 0xa7, 0x43, 0xd3,
  0xc2, 0xbf, 0xf3, 0xc7, 0x66, 0x1e, 0x29, 0xfc, 0x90, 0x49, 0xe7, 0xb7,
  0x1b, 0x3f, 0x4b, 0x98, 0xd6, 0xcf, 0x9b, 0xb1, 0x52, 0xfc, 0x65, 0x1d,
  0x83, 0xf1, 0x79, 0x57, 0xcc, 0xb1, 0x6b, 0x0a, 0x00, 0x0e, 0x67, 0xd2,
  0xce, 0xd1, 0xa8, 0x09, 0x11, 0x26, 0x73, 0x9f, 0xad, 0xba, 0xbd, 0x00,
  0x0d, 0x28, 0x08, 0x6b, 0x5a, 0x76, 0x85, 0xb6, 0x69, 0x94, 0xce, 0xf9,
  0x7b, 0xe4, 0x95, 0x74, 0x78, 0x9b, 0xf0, 0x71, 0xbc, 0x50, 0x81, 0xdf,
  0x5d, 0x76, 0x1a, 0xfa, 0x7b, 0xd7, 0x91, 0x2b, 0x7f, 0x5d, 0x4e, 0xf3,
  0xee, 0x56, 0xe9, 0x98, 0xe1, 0x02, 0x81, 0x81, 0x00, 0xf2, 0x8f, 0x13,
  0xdc, 0x1d, 0xf5, 0x84, 0xc9, 0xbc, 0xfc, 0x11, 0x81, 0x68, 0xa1, 0x77,
  0x83, 0xdf, 0xe8, 0xaf, 0x25, 0x74, 0x02, 0x1c, 0x0a, 0x56, 0xc3, 0x54,
  0xcb, 0x39, 0xb4, 0x42, 0xdb, 0xac, 0x87, 0x32, 0x46, 0x07, 0x54, 0xc1,
  0x44, 0x82, 0xfd, 0xd8, 0xa2, 0x95, 0x7b, 0x21, 0x5f, 0x55, 0xdd, 0x98,
  0x61, 0x69, 0xd6, 0x9d, 0x26, 0x5b, 0x2c, 0x61, 0xb7, 0xe0, 0x9d, 0x92,
  0x06, 0x48, 0x0a, 0x96, 0xf1, 0x05, 0x9d, 0x11, 0xc9, 0x91, 0x8a, 0xae,
  0xf9, 0x00, 0x91, 0xf4, 0x12, 0x09, 0xfa, 0x90, 0x2e, 0xb4, 0x8c, 0x5d,
  0x87, 0x3b, 0xff, 0x88, 0xf6, 0x98, 0x20, 0xf2, 0x62, 0x79, 0x8b, 0x65,
  0xfd, 0x5a, 0xfa, 0x5e, 0xf5, 0x4c, 0x2a, 0x7a, 0x3d, 0xae, 0xa3, 0x13,
  0x29, 0x5b, 0x9e, 0x4c, 0x7c, 0x4c, 0xd5, 0x87, 0xb2, 0x89, 0xf9, 0xc0,
  0xe9, 0x48, 0x13, 0x58, 0xf1, 0x02, 0x81, 0x81, 0x00, 0xc2, 0xca, 0xcb,
  0x1a, 0x6a, 0x05, 0xbd, 0x04, 0x33, 0x30, 0x52, 0x67, 0x0a, 0x7d, 0x02,
  0x02, 0x04, 0xd4, 0x86, 0xf0, 0x27, 0xc7, 0x96, 0x8d, 0x12, 0xc3, 0x6a,
  0x0f, 0x8d, 0x4c, 0x59, 0xa8, 0xf8, 0xa9, 0x63, 0x23, 0x7a, 0x0b, 0x1f,
  0xb1, 0xac, 0x1f, 0xe6, 0xb2, 0x38, 0xcb, 0xfd, 0x35, 0x19, 0x28, 0x8e,
  0x58, 0xf5, 0x72, 0x0c, 0xfc, 0xac, 0x80, 0xef, 0xc0, 0xc4, 0x4a, 0x7c,
  0x77, 0x20, 0x59, 0xda, 0xd2, 0x28, 0xa7, 0x60, 0x81, 0xd5, 0x61, 0x39,
  0xc2, 0x9a, 0x3f, 0x5c, 0x4c, 0x34, 0xbb, 0x14, 0x9d, 0xcd, 0xf1, 0x87,
  0x49, 0xd5, 0x20, 0x8a, 0xb6, 0xcb, 0x88, 0x96, 0x8e, 0x56, 0x18, 0x3e,
  0xe8, 0x49, 0x1d, 0xea, 0xce, 0xc6, 0x76, 0xe7, 0x80, 0x29, 0x70, 0x0d,
  0xb3, 0xa3, 0xe4, 0xe0, 0x54, 0xc8, 0xe1, 0xc8, 0xad, 0x40, 0x2c, 0x36,
  0x88, 0x8d, 0xdd, 0xc7, 0xc7, 0x02, 0x81, 0x81, 0x00, 0xb6, 0x97, 0xfc,
  0x94, 0x4c, 0x3d, 0x41, 0xdb, 0xf8, 0x17, 0x2d, 0x3a, 0x64, 0xc0, 0x80,
  0x4e, 0xd6, 0xe4, 0xbb, 0xcc, 0x8e, 0xfa, 0xf3, 0x8e, 0xd8, 0xe7, 0xac,
  0x52, 0x5c, 0x3b, 0x15, 0xc5, 0x76, 0xa8, 0x43, 0xb1, 0x31, 0xdd, 0x9c,
  0xab, 0xed, 0x8c, 0x18, 0x63, 0xb3, 0xf1, 0x80, 0x1a, 0xdb, 0x71, 0xb8,
  0x3f, 0x44, 0xc4, 0x59, 0x82, 0xc1, 0x49, 0xb4, 0xba, 0x0e, 0x2d, 0xaf,
  0xfa, 0x68, 0x8b, 0x08, 0x3b, 0xa2, 0x45, 0x2e, 0x59, 0xb5, 0xe5, 0x57,
  0x78, 0x7c, 0xf4, 0x62, 0x6e, 0x9d, 0xf2, 0x70, 0x4d, 0xa8, 0x7e, 0xad,
  0xfd, 0x9a, 0xad, 0xef, 0xb5, 0xea, 0xa9, 0x8a, 0xad, 0x94, 0xdf, 0xda,
  0x44, 0x12, 0x13, 0xbe, 0x1e, 0x1c, 0x5b, 0x31, 0xb0, 0x73, 0x61, 0xb1,
  0x90, 0x05, 0xb7, 0x31, 0xce, 0xb4, 0xac, 0x5d, 0xbd, 0x8a, 0x5e, 0x67,
  0xd9, 0x74, 0xf7, 0x93, 0x91, 0x02, 0x81, 0x80, 0x3c, 0xa1, 0x80, 0xac,
  0x7d, 0x19, 0xc2, 0x26, 0xbf, 0x84, 0x27, 0xf1, 0xf8, 0xba, 0xc6, 0x58,
  0x38, 0x86, 0xaa, 0x65, 0xb0, 0x0c, 0x18, 0xd9, 0x5a, 0x60, 0xc6, 0xef,
  0x86, 0x4c, 0xa0, 0x8f, 0xce, 0xae, 0x72, 0xb4, 0x45, 0x35, 0x97, 0x2c,
  0xe7, 0x65, 0x9a, 0x9a, 0x18, 0xc1, 0xad, 0xd3, 0xcc, 0x3b, 0xea, 0x6a,
  0xd8, 0x23, 0x22, 0xd5, 0xb7, 0x1f, 0xff, 0x00, 0x62, 0x58, 0xc8, 0xfb,
  0x0f, 0x6f, 0xc8, 0x83, 0xa7, 0xb1, 0xc9, 0x81, 0x40, 0x18, 0x9e, 0x30,
  0xeb, 0x70, 0x13, 0x5b, 0x89, 0x02, 0x3e, 0x3d, 0x8c, 0x2f, 0xc4, 0xab,
  0x7f, 0x92, 0x3c, 0x7a, 0xff, 0x49, 0x49, 0x8b, 0x8e, 0x52, 0xc7, 0xa9,
  0xe7, 0xb0, 0x4f, 0x39, 0x03, 0x5f, 0x59, 0x59, 0x74, 0x0c, 0x73, 0xc7,
  0x59, 0x99, 0x51, 0x16, 0x32, 0xdc, 0xcc, 0x68, 0xd8, 0xd3, 0xaa, 0x39,
  0xc0, 0xe6, 0xaa, 0x5b, 0x02, 0x81, 0x80, 0x64, 0x20, 0x63, 0x31, 0xc8,
  0xb6, 0x34, 0x66, 0xcf, 0x7d, 0x92, 0x96, 0xb8, 0x5b, 0xb9, 0xe7, 0x57,
  0x42, 0xfc, 0xcd, 0xe5, 0x7e, 0x4f, 0x39, 0x66, 0x22, 0xe7, 0x3a, 0xc4,
  0x62, 0x15, 0xc9, 0x2a, 0xbf, 0xce, 0x25, 0xf4, 0x2b, 0x26, 0x50, 0x76,
  0x63, 0x3b, 0x43, 0x2e, 0x70, 0x27, 0x7d, 0xf7, 0xf9, 0xfa, 0xf1, 0x1d,
  0x22, 0x7d, 0xaa, 0xfb, 0xe7, 0x43, 0x3f, 0x3c, 0x7b, 0x73, 0xa2, 0x21,
  0x1b, 0x25, 0x5f, 0x02, 0x31, 0x55, 0x6a, 0xd8, 0x0e, 0x7f, 0x94, 0xb1,
  0xa8, 0xd2, 0x92, 0x9f, 0x0a, 0x46, 0xa8, 0x18, 0x8f, 0x0f, 0x9e, 0x6c,
  0x7e, 0xd2, 0xbc, 0x7c, 0x0a, 0x7e, 0x32, 0x28, 0x5f, 0xaa, 0xf1, 0xf3,
  0x0e, 0xc7, 0xd5, 0xbd, 0x69, 0x5d, 0x48, 0x46, 0x3b, 0x51, 0x74, 0x26,
  0xfd, 0xed, 0x30, 0x5a, 0x51, 0x16, 0x77, 0x84, 0x64, 0xb3, 0xc2, 0xac,
  0x11, 0x8c, 0x23
};

uint8_t BaseSock::_cert_der[] =
{
  0x30, 0x82, 0x03, 0xab, 0x30, 0x82, 0x02, 0x93, 0xa0, 0x03, 0x02, 0x01,
  0x02, 0x02, 0x02, 0x01, 0x09, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48,
  0x86, 0xf7, 0x0d, 0x01, 0x01, 0x0b, 0x05, 0x00, 0x30, 0x3c, 0x31, 0x0b,
  0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x49, 0x54, 0x31,
  0x0e, 0x30, 0x0c, 0x06, 0x03, 0x55, 0x04, 0x08, 0x0c, 0x05, 0x49, 0x74,
  0x61, 0x6c, 0x79, 0x31, 0x0e, 0x30, 0x0c, 0x06, 0x03, 0x55, 0x04, 0x07,
  0x0c, 0x05, 0x4d, 0x69, 0x6c, 0x61, 0x6e, 0x31, 0x0d, 0x30, 0x0b, 0x06,
  0x03, 0x55, 0x04, 0x03, 0x0c, 0x04, 0x4d, 0x79, 0x43, 0x41, 0x30, 0x1e,
  0x17, 0x0d, 0x31, 0x38, 0x31, 0x32, 0x30, 0x39, 0x31, 0x30, 0x35, 0x35,
  0x34, 0x34, 0x5a, 0x17, 0x0d, 0x32, 0x38, 0x31, 0x32, 0x30, 0x36, 0x31,
  0x30, 0x35, 0x35, 0x34, 0x34, 0x5a, 0x30, 0x53, 0x31, 0x0b, 0x30, 0x09,
  0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x49, 0x54, 0x31, 0x0e, 0x30,
  0x0c, 0x06, 0x03, 0x55, 0x04, 0x08, 0x0c, 0x05, 0x49, 0x74, 0x61, 0x6c,
  0x79, 0x31, 0x0e, 0x30, 0x0c, 0x06, 0x03, 0x55, 0x04, 0x07, 0x0c, 0x05,
  0x4d, 0x69, 0x6c, 0x61, 0x6e, 0x31, 0x10, 0x30, 0x0e, 0x06, 0x03, 0x55,
  0x04, 0x0a, 0x0c, 0x07, 0x49, 0x74, 0x61, 0x6c, 0x74, 0x65, 0x6c, 0x31,
  0x12, 0x30, 0x10, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x09, 0x6c, 0x6f,
  0x63, 0x61, 0x6c, 0x68, 0x6f, 0x73, 0x74, 0x30, 0x82, 0x01, 0x22, 0x30,
  0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01,
  0x05, 0x00, 0x03, 0x82, 0x01, 0x0f, 0x00, 0x30, 0x82, 0x01, 0x0a, 0x02,
  0x82, 0x01, 0x01, 0x00, 0xb8, 0x90, 0x92, 0x62, 0xf6, 0x1f, 0xf0, 0x94,
  0x87, 0xbb, 0xf3, 0xb4, 0xd9, 0x0d, 0x74, 0x1f, 0xfd, 0xe9, 0xd4, 0x4f,
  0xa6, 0xfa, 0xf9, 0xf4, 0x5d, 0x9d, 0x9d, 0x92, 0xdb, 0x65, 0x12, 0x0a,
  0x16, 0xd3, 0x2b, 0x10, 0xe3, 0xff, 0x08, 0xa9, 0x68, 0x2a, 0x16, 0xdb,
  0xe6, 0xa2, 0xa7, 0xab, 0x51, 0xc2, 0xd5, 0xb8, 0x4b, 0x8b, 0xa4, 0x90,
  0x5a, 0xea, 0x74, 0x5d, 0x7f, 0xd6, 0x22, 0xf4, 0xad, 0xfe, 0x5c, 0xb5,
  0x6b, 0xc4, 0x7f, 0xcc, 0x65, 0xa9, 0xde, 0xc9, 0x27, 0x60, 0x9c, 0x7f,
  0x9a, 0xbe, 0x07, 0x7f, 0x96, 0xf2, 0x43, 0xaf, 0x39, 0xc0, 0xdb, 0x99,
  0x1f, 0x7a, 0xce, 0x3a, 0xd4, 0x6b, 0x30, 0xc2, 0xe3, 0xb9, 0xc8, 0x7f,
  0xad, 0x72, 0x18, 0x6c, 0xed, 0xb3, 0x36, 0x66, 0x54, 0x81, 0x20, 0x44,
  0x25, 0xb4, 0xff, 0x7b, 0x2a, 0x14, 0x99, 0x74, 0xc9, 0x36, 0x10, 0xbc,
  0x8d, 0xb1, 0xd3, 0xf6, 0x29, 0xb9, 0x92, 0x56, 0x7a, 0x52, 0x24, 0xf3,
  0xa6, 0x3a, 0x3f, 0xe1, 0x30, 0xde, 0x4f, 0x81, 0x2f, 0xb3, 0xc8, 0x95,
  0x9d, 0x4d, 0xf0, 0xd0, 0xdf, 0x8f, 0xd5, 0xc1, 0xb2, 0xc0, 0x09, 0x19,
  0x54, 0x19, 0x8e, 0x29, 0x44, 0x70, 0xb4, 0x9c, 0x96, 0x93, 0xaf, 0xb2,
  0xeb, 0xb6, 0x50, 0x00, 0x42, 0x35, 0x7d, 0xde, 0x7a, 0x16, 0x75, 0x6e,
  0x10, 0x74, 0x2f, 0xf7, 0x09, 0x24, 0x47, 0x4c, 0xd0, 0xca, 0x8c, 0x4d,
  0x2b, 0x71, 0xf9, 0xd4, 0x50, 0xba, 0x55, 0x7e, 0xf2, 0xe8, 0x49, 0xce,
  0x72, 0xfc, 0xc0, 0xff, 0xf7, 0x66, 0xfa, 0xdb, 0x10, 0x74, 0x2e, 0xf5,
  0x18, 0x70, 0x8f, 0xcf, 0x66, 0x26, 0x65, 0xa2, 0xd8, 0xe1, 0xe0, 0xef,
  0xa8, 0x8a, 0xb2, 0xab, 0xfc, 0x05, 0x0f, 0x0e, 0xed, 0x75, 0x3e, 0x75,
  0x54, 0xa6, 0x43, 0x16, 0x96, 0x3a, 0x7a, 0x57, 0x02, 0x03, 0x01, 0x00,
  0x01, 0xa3, 0x81, 0x9f, 0x30, 0x81, 0x9c, 0x30, 0x09, 0x06, 0x03, 0x55,
  0x1d, 0x13, 0x04, 0x02, 0x30, 0x00, 0x30, 0x1d, 0x06, 0x03, 0x55, 0x1d,
  0x0e, 0x04, 0x16, 0x04, 0x14, 0x6e, 0xc2, 0x09, 0xa5, 0xb5, 0x94, 0xc8,
  0x35, 0x36, 0x28, 0xef, 0x4d, 0x47, 0x10, 0x4b, 0xcc, 0x0f, 0xd0, 0x0e,
  0xcc, 0x30, 0x1f, 0x06, 0x03, 0x55, 0x1d, 0x23, 0x04, 0x18, 0x30, 0x16,
  0x80, 0x14, 0xee, 0xb8, 0x31, 0x84, 0x77, 0xf7, 0xef, 0x23, 0x5c, 0x69,
  0x25, 0x4e, 0xcd, 0xf1, 0x5e, 0x29, 0x5d, 0xa7, 0xb3, 0xb4, 0x30, 0x11,
  0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x86, 0xf8, 0x42, 0x01, 0x01, 0x04,
  0x04, 0x03, 0x02, 0x06, 0x40, 0x30, 0x0b, 0x06, 0x03, 0x55, 0x1d, 0x0f,
  0x04, 0x04, 0x03, 0x02, 0x05, 0xe0, 0x30, 0x13, 0x06, 0x03, 0x55, 0x1d,
  0x25, 0x04, 0x0c, 0x30, 0x0a, 0x06, 0x08, 0x2b, 0x06, 0x01, 0x05, 0x05,
  0x07, 0x03, 0x01, 0x30, 0x1a, 0x06, 0x03, 0x55, 0x1d, 0x11, 0x04, 0x13,
  0x30, 0x11, 0x82, 0x09, 0x6c, 0x6f, 0x63, 0x61, 0x6c, 0x68, 0x6f, 0x73,
  0x74, 0x87, 0x04, 0x7f, 0x00, 0x00, 0x01, 0x30, 0x0d, 0x06, 0x09, 0x2a,
  0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x0b, 0x05, 0x00, 0x03, 0x82,
  0x01, 0x01, 0x00, 0x1e, 0x62, 0x99, 0x93, 0x84, 0x7d, 0xb7, 0x99, 0x95,
  0xa9, 0xdd, 0xf6, 0x87, 0x87, 0x30, 0x27, 0x60, 0x13, 0x09, 0x36, 0x87,
  0xcb, 0xdf, 0x02, 0x59, 0x15, 0x5f, 0x93, 0x56, 0x99, 0x86, 0xfe, 0x6a,
  0x1f, 0xb8, 0xdb, 0xb6, 0x8d, 0x80, 0x06, 0x4e, 0xdd, 0xc6, 0x54, 0xac,
  0x8a, 0x5d, 0x5c, 0x5b, 0xbb, 0xef, 0x86, 0x59, 0x01, 0x7d, 0x94, 0xf2,
  0x4a, 0x03, 0xe2, 0xc8, 0x43, 0xb6, 0xcd, 0x68, 0xe7, 0x02, 0xd3, 0xb7,
  0x7e, 0x77, 0x63, 0x0a, 0x6f, 0x13, 0x68, 0x72, 0x64, 0x0c, 0x99, 0x87,
  0x8e, 0x7d, 0xc8, 0xa1, 0xea, 0x3d, 0x4c, 0xd0, 0x9e, 0xe3, 0xc8, 0x70,
  0xc9, 0x3b, 0x57, 0x29, 0x01, 0x2c, 0x1a, 0x53, 0xc3, 0xfa, 0x7c, 0xf8,
  0x48, 0x55, 0x07, 0xa0, 0x8f, 0x72, 0x1c, 0x51, 0xd9, 0x8e, 0xb6, 0x02,
  0xb0, 0xbb, 0xfe, 0x9e, 0x42, 0x68, 0x76, 0xaa, 0x32, 0x41, 0x32, 0x87,
  0x2d, 0x1f, 0x7b, 0xb8, 0xb2, 0x46, 0x9b, 0xd6, 0xf4, 0x0a, 0x1d, 0x27,
  0xb9, 0xe0, 0x60, 0x7d, 0x7a, 0xaa, 0x33, 0x5c, 0x09, 0x76, 0x80, 0xa5,
  0x99, 0xc5, 0x73, 0x5d, 0x0a, 0xc5, 0xf1, 0xb5, 0x11, 0x1d, 0x95, 0xfc,
  0xab, 0x66, 0xb6, 0xb1, 0xb3, 0xdc, 0x86, 0xfe, 0xd1, 0x6e, 0x46, 0x3e,
  0x44, 0x27, 0xf3, 0xf4, 0x9a, 0xfb, 0x1f, 0xd5, 0xd6, 0xb7, 0xb9, 0x7d,
  0xda, 0x2c, 0x7a, 0xc8, 0x4f, 0x86, 0x43, 0x04, 0x5c, 0x9f, 0xb7, 0x5d,
  0x4e, 0xd1, 0x39, 0xa9, 0xa4, 0x47, 0xbe, 0x81, 0x73, 0x63, 0x6d, 0xdf,
  0x07, 0x91, 0xe2, 0xe7, 0x26, 0xbe, 0x4c, 0x75, 0x8f, 0xd0, 0x1a, 0x42,
  0xa8, 0x3f, 0x9c, 0x12, 0xe3, 0xb2, 0x2b, 0xdb, 0x64, 0x6a, 0x53, 0x86,
  0x23, 0xc4, 0x37, 0x23, 0x51, 0x89, 0xdc, 0xb7, 0x2d, 0x9e, 0x7b, 0xd3,
  0x22, 0x0c, 0xe7, 0x97, 0x11, 0x82, 0xdc
};
#endif

using namespace std;

BaseSock::BaseSock() : _sock(-1), _TimeOut(4)
#ifdef HAVE_SSL
                     , _sslsock(false), _ctx(0), _ssl(0), _ssl_id_context(666)
#endif
{
}

BaseSock::~BaseSock()
{
#ifdef HAVE_SSL
   if (_ctx)
      SSL_CTX_free(_ctx);
#endif
}

string BaseSock::GetErrorMessage()
{
   string errmsg;
   if (__lasterror == __EADDRNOTAVAIL)
      errmsg = "Address not available";
   else
   {
#ifdef MINGW
      int err = WSAGetLastError();
#else
      int err = errno;
#endif
      errmsg = Tools::GetErrorMessage(err);
   }
   return errmsg;
}

int BaseSock::BytesAvailable()
{
   unsigned long readable = 0;
   time_t started = time(0);

   while (readable == 0)
   {
      if (time(0) > started + _TimeOut)
         break;
      usleep(200000);
      ioctlsocket(_sock, FIONREAD, &readable);
   }
   return (int)readable;
}

int BaseSock::SetTimeOut(int nTO)
{
   int old = _TimeOut;
   _TimeOut = nTO;
   return old;
}

int BaseSock::GetReceiveBufferSize()
{
   if (_sock == (SOCKET)-1)
      return -1;
   int buf_size;
   socklen_t opt_len = sizeof(int);
   getsockopt(_sock, SOL_SOCKET, SO_RCVBUF, (char *)&buf_size, &opt_len);
   return buf_size;
}

void BaseSock::SetReceiveBufferSize(int size)
{
   if (_sock != (SOCKET)-1)
      setsockopt(_sock, SOL_SOCKET, SO_RCVBUF, (char *)&size, sizeof(int));
}

int BaseSock::GetSendBufferSize()
{
   if (_sock == (SOCKET)-1)
      return -1;
   int buf_size;
   socklen_t opt_len = sizeof(int);
   getsockopt(_sock, SOL_SOCKET, SO_SNDBUF, (char *)&buf_size, &opt_len);
   return buf_size;
}

void BaseSock::SetSendBufferSize(int size)
{
   if (_sock != (SOCKET)-1)
      setsockopt(_sock, SOL_SOCKET, SO_SNDBUF, (char *)&size, sizeof(int));
}

void BaseSock::SetTcpNoDelay(bool on)
{
   if (_sock != (SOCKET)-1)
   {
      int opt = (on) ? 1 : 0;
      setsockopt(_sock, IPPROTO_TCP, TCP_NODELAY, (char *)&opt, sizeof(int));
   }
}

int BaseSock::LowLevelRead(uint8_t *buf, int len, bool peek)
{
   int rc;
#ifdef HAVE_SSL
   if (_sslsock)
   {
      do
      {
         rc = (peek) ? SSL_peek(_ssl, buf, len) : SSL_read(_ssl, buf, len);
      } while (rc == -1 && errno == EINTR);
   }
   else
#endif
   {
      do
      {
         rc = (int)recv(_sock, (char *)buf, len, (peek) ? MSG_PEEK : 0);
      } while (rc == -1 && __lasterror == __EINTR);
   }
   return rc;
}

int BaseSock::LowLevelWrite(const uint8_t *buf, int len)
{
   int rc;
#ifdef HAVE_SSL
   if (_sslsock)
   {
      do
      {
         rc = SSL_write(_ssl, buf, len);
      } while (rc == -1 && errno == EINTR);
   }
   else
#endif
   {
      do
      {
         rc = (int)send(_sock, (const char *)buf, len, 0);
      } while (rc == -1 && __lasterror == __EINTR);
   }
   return rc;
}

int BaseSock::ExchangeData(DataAction action, uint8_t *buf, int len, bool peek)
{
   if (_sock == (SOCKET)-1)
   {
      __seterror(__ENOTCONN);
      return -1;
   }
   __seterror(0);
   if (len <= 0)
      return 0;

   int rc;
   rc = (action == DATA_READ) ? LowLevelRead(buf, len, peek) : LowLevelWrite(buf, len);
   bool to_poll;
#ifdef HAVE_SSL
   if (_sslsock)
   {
      int ssl_want = (action == DATA_READ) ? SSL_ERROR_WANT_READ : SSL_ERROR_WANT_WRITE;
      to_poll = (SSL_get_error(_ssl, rc) == ssl_want);
   }
   else
#endif
   to_poll = (rc == -1 && __lasterror == __EWOULDBLOCK);
   while (to_poll)
   {
      to_poll = false;
      short event = (action == DATA_READ) ? POLLIN : POLLOUT;
      pollfd fds[1] = { { _sock, event, 0 } };
      rc = 0;
      int loops = _TimeOut;
      while (rc == 0 && loops > 0)
      {
         do
         {
            rc = __poll(fds, sizeof(fds) / sizeof(fds[0]), 1000);
         } while (rc == -1 && __lasterror == __EINTR);
         --loops;
      }
      if (rc == 0)
      {
         __seterror(__ETIMEDOUT);
         rc = -1;
      }
      else if (rc > 0 && fds[0].revents & event)
      {
         __seterror(0);
         rc = (action == DATA_READ) ? LowLevelRead(buf, len, peek) : LowLevelWrite(buf, len);
#ifdef HAVE_SSL
         if (_sslsock)
         {
            int ssl_want = (action == DATA_READ) ? SSL_ERROR_WANT_READ : SSL_ERROR_WANT_WRITE;
            to_poll = (SSL_get_error(_ssl, rc) == ssl_want);
         }
         else
#endif
         to_poll = (rc == -1 && __lasterror == __EWOULDBLOCK);
         if (rc == 0)
         {
            __seterror(__ECONNRESET);
            rc = -1;
         }
      }
      else if (rc > 0 && fds[0].revents & POLLHUP)
      {
         __seterror(__ECONNRESET);
         rc = -1;
      }
   }
   return rc;
}

int BaseSock::ReadData(uint8_t *buf, int len, bool peek)
{
   return ExchangeData(DATA_READ, buf, len, peek);
}

int BaseSock::WriteData(const uint8_t *buf, int len)
{
   uint8_t chunk[1500];
   int chunk_size;
   int to_write = len;
   int bytes;
   int tot_bytes = 0;

   while (to_write > 0)
   {
      chunk_size = (to_write > (int)sizeof(chunk)) ? (int)sizeof(chunk) : to_write;
      memcpy(chunk, &buf[tot_bytes], chunk_size);
      if ((bytes = ExchangeData(DATA_WRITE, chunk, chunk_size)) > 0)
      {
         tot_bytes += bytes;
         to_write -= bytes;
      }
      else
         break;
   }
   return tot_bytes;
}

int BaseSock::ReadDataExact(uint8_t *buf, int len)
{
   uint8_t chunk[1500];
   int chunk_size;
   int to_read = len;
   int bytes;
   int tot_bytes = 0;

   while (to_read > 0)
   {
      chunk_size = (to_read > (int)sizeof(chunk)) ? (int)sizeof(chunk) : to_read;
      if ((bytes = ReadData(chunk, chunk_size)) > 0)
      {
         memcpy(&buf[tot_bytes], chunk, bytes);
         tot_bytes += bytes;
         to_read -= bytes;
      }
      else
         break;
   }
   return tot_bytes;
}

bool BaseSock::WriteLine(const string& line)
{
   string _line = line + "\r\n";
   if (WriteData((const uint8_t *)_line.c_str(), (int)_line.length()) != (int)_line.length())
      return false;
   return true;
}

bool BaseSock::ReadLine(string& line)
{
   char buf[1500];
   int tot_bytes = 0;
   time_t start = time(0);
   time_t elapsed = 0;
   int bytes;
   memset(buf, 0, sizeof(buf));
   do
   {
      int to_read = sizeof(buf) - tot_bytes;
      bytes = ReadData((uint8_t *)buf + tot_bytes, to_read, true);
      if (bytes > 0)
      {
         bool crlf;
         int i = FindEOL((const uint8_t *)buf, tot_bytes + bytes, &crlf);
         if (i != -1)
         {
            ReadData((uint8_t *)buf + tot_bytes, i - tot_bytes + ((crlf) ? 2 : 1));
            line = string((const char *)buf, i);
            return true;
         }
         else
            ReadData((uint8_t *)buf + tot_bytes, bytes);
         tot_bytes += bytes;
      }
      elapsed = time(0) - start;
   } while (bytes > 0 && elapsed < _TimeOut);
   if (bytes > 0)
      __seterror(__ETIMEDOUT);
   line = string((const char *)buf, tot_bytes);
   return false;
}

#ifdef HAVE_SSL
bool BaseSock::UseCertificate(const string& cert_file, const string& pk_file, const string& pwd)
{
   if (_sslsock)
      return false;
   _cert_file = cert_file;
   _pk_file = (pk_file.length() > 0) ? pk_file : cert_file;
   _pk_pwd = pwd;
   return true;
}

string BaseSock::GetSSLVersion()
{
   string ssl_ver;
   if (_ssl)
   {
      const char *ssl_name  = SSL_get_version(_ssl);
      if (ssl_name)
         ssl_ver = string(ssl_name);
   }
   return ssl_ver;
}

string BaseSock::GetPeerCertificateCommonName()
{
   string subject;
   X509 *cert;
   if (_ssl && (cert = SSL_get_peer_certificate(_ssl)) != 0)
   {
      X509_NAME* sname = cert ? X509_get_subject_name(cert) : 0;
      char buff[512];
      if (sname && X509_NAME_get_text_by_NID(sname, NID_commonName, buff, sizeof(buff)) >= 0)
         subject = string(buff);
      X509_free(cert);
   }
   return subject;
}

string BaseSock::GetPeerCertificateCN()
{
   return GetPeerCertificateCommonName();
}

string BaseSock::GetPeerCertificateOU()
{
   string subject;
   X509 *cert;
   if (_ssl && (cert = SSL_get_peer_certificate(_ssl)) != 0)
   {
      X509_NAME* sname = cert ? X509_get_subject_name(cert) : 0;
      char buff[512];
      if (sname && X509_NAME_get_text_by_NID(sname, NID_organizationalUnitName, buff, sizeof(buff)) >= 0)
         subject = string(buff);
      X509_free(cert);
   }
   return subject;
}

string BaseSock::GetPeerCertificateO()
{
   string subject;
   X509 *cert;
   if (_ssl && (cert = SSL_get_peer_certificate(_ssl)) != 0)
   {
      X509_NAME* sname = cert ? X509_get_subject_name(cert) : 0;
      char buff[512];
      if (sname && X509_NAME_get_text_by_NID(sname, NID_organizationName, buff, sizeof(buff)) >= 0)
         subject = string(buff);
      X509_free(cert);
   }
   return subject;
}

string BaseSock::GetPeerCertificateEmail()
{
   string email;
   X509 *cert;
   if (_ssl && (cert = SSL_get_peer_certificate(_ssl)) != 0)
   {
      X509_NAME* sname = cert ? X509_get_subject_name(cert) : 0;
      char buff[512];
      if (sname && X509_NAME_get_text_by_NID(sname, NID_pkcs9_emailAddress, buff, sizeof(buff)) >= 0)
         email = string(buff);
      X509_free(cert);
   }
   return email;
}

string BaseSock::GetPeerCertificateExpiry()
{
   string expiry;
   X509 *cert;
   if (_ssl && (cert = SSL_get_peer_certificate(_ssl)) != 0)
   {
      ASN1_TIME *tm = X509_get_notAfter(cert);
      if (tm && tm->length == 13)
      {
         string tmp(tm->data, tm->data + tm->length);
         expiry = "20" + tmp.substr(0, 2) + "-" + tmp.substr(2, 2) + "-" + tmp.substr(4, 2) + " " +
                  tmp.substr(6, 2) + ":" + tmp.substr(8, 2) + ":" + tmp.substr(10, 2);
      }
      X509_free(cert);
   }
   return expiry;
}

string BaseSock::GetPeerCertificateIssuer()
{
   string issuer;
   X509 *cert;
   if (_ssl && (cert = SSL_get_peer_certificate(_ssl)) != 0)
   {
      X509_NAME* iname = cert ? X509_get_issuer_name(cert) : 0;
      char buff[512];
      if (iname && X509_NAME_get_text_by_NID(iname, NID_commonName, buff, sizeof(buff)) >= 0)
         issuer = string(buff);
      X509_free(cert);
   }
   return issuer;
}

int BaseSock::ServerNameIndicationCB(SSL *ssl, int *ad, void *arg)
{
   BaseSock *bsock = (BaseSock *)arg;
   const char *sni = SSL_get_servername(ssl, TLSEXT_NAMETYPE_host_name);
   if (sni && sni[0])
      bsock->_sni = string(sni);
   else
      bsock->_sni = "";
   return SSL_TLSEXT_ERR_OK;
}

int BaseSock::CertVerifyCB(int preverify_ok, X509_STORE_CTX *x509_ctx)
{
#ifdef SOCK_DEBUG
   X509 *cert = X509_STORE_CTX_get_current_cert(x509_ctx);
   int depth = X509_STORE_CTX_get_error_depth(x509_ctx);
   string subject;
   string issuer;
   X509_NAME* sname = cert ? X509_get_subject_name(cert) : 0;
   X509_NAME* iname = cert ? X509_get_issuer_name(cert) : 0;
   char buff[512];
   if (sname && X509_NAME_get_text_by_NID(sname, NID_commonName, buff, sizeof(buff)) >= 0)
      subject = string(buff);
   if (iname && X509_NAME_get_text_by_NID(iname, NID_commonName, buff, sizeof(buff)) >= 0)
      issuer = string(buff);
   Tools::debug << "CertVerifyCB: preverify_ok = " << preverify_ok << ", depth = " << depth << endl;
   Tools::debug << "CertVerifyCB: subject = \"" << subject << "\"" << endl;
   Tools::debug << "CertVerifyCB: issuer = \"" << issuer << "\"" << endl;
#endif
   if (!preverify_ok)
      X509_STORE_CTX_set_error(x509_ctx, X509_V_ERR_CERT_REJECTED);
   return preverify_ok;
}

int BaseSock::CertKeyPasswordCB(char *buf, int size, int rwflag, void *password)
{
   string *pwd = (string *)password;
   if (pwd->length() == 0)
      return 0;
   string plain;
   if (pwd->length() >= 7 && pwd->substr(0, 7) == "crypt::")
      plain = Tools::DecryptPassword(pwd->substr(7));
   else if (pwd->length() >= 7 && pwd->substr(0, 7) == "passp::")
      plain = Tools::GeneratePassword(pwd->substr(7));
   else
      plain = *pwd;
   if ((int)plain.length() > size)
      return 0;
   memcpy(buf, plain.c_str(), plain.length());
   return plain.length();
}

bool BaseSock::StartSSL(bool use_cert, SSLMethod method)
{
   if (!_sslsock)
      SetSSL(true, use_cert, method);
   if ((_ssl = SSL_new(_ctx)) == 0)
      return false;
   if (_sni.length() > 0)
      SSL_set_tlsext_host_name(_ssl, _sni.c_str());
   SSL_set_fd(_ssl, _sock);
   SSL_set_connect_state(_ssl);
   unsigned long opt = 0;
   ioctlsocket(_sock, FIONBIO, &opt);
   int rc = SSL_connect(_ssl);
   opt = 1;
   ioctlsocket(_sock, FIONBIO, &opt);
   if (rc <= 0)
   {
      __seterror(__ECONNABORTED);
      return false;
   }
   return true;
}

bool BaseSock::SetSSL(bool ssl, bool use_cert, SSLMethod method, const string& ca_file, const string& crl_file)
{
   bool rc = true;;
   if (ssl)
   {
      if ((method & _method_mask) == 0)
         method = _best_method;
#if OPENSSL_VERSION_NUMBER < 0x10000000
      SSL_METHOD *_method = 0;
#else
      const SSL_METHOD *_method = 0;
#endif
      if (method == SSLv23)
         _method = SSLv23_method();
#if OPENSSL_VERSION_NUMBER >= 0x00900000
      else if (method == TLSv1)
         _method = TLSv1_method();
      else if (method == DTLSv1)
         _method = DTLSv1_method();
#endif
#if OPENSSL_VERSION_NUMBER >= 0x10001000
      else if (method == TLSv1_1)
         _method = TLSv1_1_method();
      else if (method == TLSv1_2)
         _method = TLSv1_2_method();
#endif
#if OPENSSL_VERSION_NUMBER >= 0x10100000
      else if (method == TLS)
         _method = TLS_method();
      else if (method == DTLS)
         _method = DTLS_method();
      else if (method == DTLSv1_2)
         _method = DTLSv1_2_method();
#endif
      if ((_ctx = SSL_CTX_new(_method)) == 0)
         return false;
      SSL_CTX_set_options(_ctx, SSL_OP_NO_SSLv2);
#if OPENSSL_VERSION_NUMBER >= 0x10001000
      const char *ciphers = "TLSv1.2+HIGH:TLSv1+HIGH:!SSLv2:!DSS:!PSK:!SRP:RC4+MEDIUM:!aNULL:@STRENGTH";
#else
      const char *ciphers = "TLSv1+HIGH:!SSLv2:!DSS:!PSK:!SRP:RC4+MEDIUM:!aNULL:@STRENGTH";
#endif
      SSL_CTX_set_cipher_list(_ctx, ciphers);
#ifdef SSL_OP_NO_COMPRESSION
      SSL_CTX_set_options(_ctx, SSL_OP_NO_COMPRESSION);
#endif
      if (method == DTLSv1 || method == DTLSv1_2 || method == DTLS)
         SSL_CTX_set_read_ahead(_ctx, 1);
      if (ca_file.length() > 0)
      {
         if ((rc = SSL_CTX_load_verify_locations(_ctx, ca_file.c_str(), 0)))
         {
            SSL_CTX_set_verify(_ctx, SSL_VERIFY_PEER | SSL_VERIFY_CLIENT_ONCE | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, CertVerifyCB);
#ifdef X509_V_FLAG_CRL_CHECK
            if (crl_file.length() > 0)
            {
               X509_STORE *store = SSL_CTX_get_cert_store(_ctx);
               if (store && (rc = X509_STORE_load_locations(store, crl_file.c_str(), 0)) == 1)
                  X509_STORE_set_flags(store, X509_V_FLAG_CRL_CHECK | X509_V_FLAG_CRL_CHECK_ALL);
            }
#endif
         }
      }
      if (use_cert && rc)
      {
         if (_cert_file.length() == 0)
         {
            rc = SSL_CTX_use_RSAPrivateKey_ASN1(_ctx, _pk_der, sizeof(_pk_der)) == 1 &&
                 SSL_CTX_use_certificate_ASN1(_ctx, sizeof(_cert_der), _cert_der) == 1;
         }
         else
         {
            SSL_CTX_set_default_passwd_cb_userdata(_ctx, (void *)&_pk_pwd);
            SSL_CTX_set_default_passwd_cb(_ctx, CertKeyPasswordCB);
            rc = SSL_CTX_use_RSAPrivateKey_file(_ctx, _pk_file.c_str(), SSL_FILETYPE_PEM) == 1 &&
                 SSL_CTX_use_certificate_chain_file(_ctx, _cert_file.c_str()) == 1;
         }
      }
      SSL_CTX_set_tlsext_servername_callback(_ctx, ServerNameIndicationCB);
      SSL_CTX_set_tlsext_servername_arg(_ctx, this);
   }
   else if (_ctx)
   {
      SSL_CTX_free(_ctx);
      _ctx = 0;
   }
   _sslsock = ssl;
   return rc;
}
#endif

void BaseSock::CloseConnection()
{
   int _errno = __lasterror;
   if (_sock != (SOCKET)-1)
   {
#ifdef HAVE_SSL
      if (_ssl)
      {
         SSL_shutdown(_ssl);
         SSL_free(_ssl);
         _ssl = 0;
      }
#endif
      closesocket(_sock);
      _sock = (SOCKET)-1;
   }
   __seterror(_errno);
}

int BaseSock::FindEOL(const uint8_t *buffer, int len, bool *crlf)
{
   *crlf = false;
   if (len > 0)
   {
      for (int i = 0; i < len; ++i)
      {
         bool found = false;
         if (buffer[i] == '\n')
         {
            found = true;
            if (i > 0 && buffer[i-1] == '\r')
            {
              *crlf = true;
               --i;
            }
         }
         if (found)
            return i;
      }
   }
   return -1;
}

string BaseSock::GetHostname()
{
   char hname[255] = { 0 };
   gethostname(hname, sizeof(hname) - 1);
   return string(hname);
}

int BaseSock::GetLocalAddresses(map<string,vector<string> >& ips)
{
   ips.clear();
#ifdef MINGW
   ULONG buflen = 15 << 10;
   ULONG flags = GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER;
   IP_ADAPTER_ADDRESSES *ifaces = (IP_ADAPTER_ADDRESSES *)HeapAlloc(GetProcessHeap(), 0, buflen);
   ULONG rc = GetAdaptersAddresses(AF_INET, flags, 0, ifaces, &buflen);
   if (rc == ERROR_BUFFER_OVERFLOW)
   {
      HeapFree(GetProcessHeap(), 0, ifaces);
      ifaces = (IP_ADAPTER_ADDRESSES *)HeapAlloc(GetProcessHeap(), 0, buflen);
      rc = GetAdaptersAddresses(AF_INET, flags, 0, ifaces, &buflen);
   }
   if (rc != NO_ERROR)
   {
      HeapFree(GetProcessHeap(), 0, ifaces);
      return 0;
   }
   IP_ADAPTER_ADDRESSES *iface = ifaces;
   while (iface)
   {
      if (iface->OperStatus == IfOperStatusUp)
      {
         vector<string> ipaddrs;
         IP_ADAPTER_UNICAST_ADDRESS *ip = iface->FirstUnicastAddress;
         while (ip)
         {
            string ipaddr(inet_ntoa(((sockaddr_in *)(ip->Address.lpSockaddr))->sin_addr));
            if (ipaddr != "0.0.0.0" && ipaddr.substr(0, 7) != "169.254")
               ipaddrs.push_back(ipaddr);
            ip = ip->Next;
         }
         if (ipaddrs.size() > 0)
            ips.insert(make_pair(Tools::Unicode2Ascii((uint8_t *)iface->FriendlyName, wcslen(iface->FriendlyName)), ipaddrs));
      }
      iface = iface->Next;
   }
   HeapFree(GetProcessHeap(), 0, ifaces);
#else
   struct ifaddrs *ifaces;
   if (getifaddrs(&ifaces) != 0)
      return 0;
   struct ifaddrs *iface = ifaces;
   while (iface)
   {
      vector<string> ipaddrs;
      if (iface->ifa_addr && iface->ifa_addr->sa_family == AF_INET && (iface->ifa_flags & 1))
      {
         string ipaddr(inet_ntoa(((sockaddr_in *)(iface->ifa_addr))->sin_addr));
         if (ipaddr != "0.0.0.0" && ipaddr.substr(0, 7) != "169.254")
            ipaddrs.push_back(ipaddr);
      }
      if (ipaddrs.size() > 0)
      {
         map<string, vector<string> >::iterator item = ips.find(iface->ifa_name);
         if (item == ips.end())
            ips.insert(make_pair(iface->ifa_name, ipaddrs));
         else
            item->second.push_back(ipaddrs[0]);
      }
      iface = iface->ifa_next;
   }
   freeifaddrs(ifaces);
#endif
   return (int)ips.size();
}

uint32_t BaseSock::GetIPv4(const string& host)
{
   uint32_t rc = 0;
   addrinfo *hostinfo;
   addrinfo hints;

   memset(&hints, 0, sizeof(hints));
   hints.ai_family = PF_INET;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_flags = AI_PASSIVE;
   if (getaddrinfo(host.c_str(), 0, &hints, &hostinfo) == 0)
   {
      rc = ((sockaddr_in *)(hostinfo->ai_addr))->sin_addr.s_addr;
      freeaddrinfo(hostinfo);
   }
   return rc;
}

void BaseSock::Setup()
{
#ifdef MINGW
   WSADATA wsa;
   WSAStartup(0x0202, &wsa);
#else
   sigset_t sigs_to_block; 
   sigemptyset(&sigs_to_block);  
   sigaddset(&sigs_to_block, SIGPIPE); 
   sigprocmask(SIG_BLOCK, &sigs_to_block, 0);  
#endif
#ifdef HAVE_SSL
   SSL_library_init();
#ifdef HAVE_PTHREAD
   MultiThreadSetup();
#endif
#endif
}

void BaseSock::Cleanup()
{
#if defined(HAVE_SSL) && defined(HAVE_PTHREAD)
   MultiThreadCleanup();
#endif
}

#if defined(HAVE_SSL) && defined(HAVE_PTHREAD)
void BaseSock::MultiThreadSetup()
{
   int nlock = CRYPTO_num_locks();

   _locks = (pthread_rwlock_t *)OPENSSL_malloc(nlock * sizeof(pthread_rwlock_t));
   for (int i = 0; i < nlock; ++i)
      pthread_rwlock_init(&_locks[i], 0);

   CRYPTO_set_locking_callback(ssl_locking);
   CRYPTO_set_dynlock_create_callback(ssl_dyn_create);
   CRYPTO_set_dynlock_lock_callback(ssl_dyn_lock);
   CRYPTO_set_dynlock_destroy_callback(ssl_dyn_destroy);
}

void BaseSock::MultiThreadCleanup()
{
   int nlock = CRYPTO_num_locks();

   CRYPTO_set_dynlock_destroy_callback(0);
   CRYPTO_set_dynlock_lock_callback(0);
   CRYPTO_set_dynlock_create_callback(0);
   CRYPTO_set_locking_callback(0);

   for (int i = 0; i < nlock; ++i)
      pthread_rwlock_destroy(&_locks[i]);

   OPENSSL_free(_locks);
   _locks = 0;
}

void BaseSock::ssl_locking(int mode, int index, const char *, int)
{
   if (mode == (CRYPTO_LOCK | CRYPTO_READ))
      pthread_rwlock_rdlock(&_locks[index]);
   else if (mode & CRYPTO_LOCK)
      pthread_rwlock_wrlock(&_locks[index]);
   else
      pthread_rwlock_unlock(&_locks[index]);
}

CRYPTO_dynlock_value *BaseSock::ssl_dyn_create(const char *, int)
{
   CRYPTO_dynlock_value *cdv = new CRYPTO_dynlock_value;
   pthread_rwlock_init(&cdv->rwlock, 0);
   return cdv;
}

void BaseSock::ssl_dyn_lock(int mode, CRYPTO_dynlock_value *cdv, const char *, int)
{
   if (mode == (CRYPTO_LOCK | CRYPTO_READ))
      pthread_rwlock_rdlock(&cdv->rwlock);
   else if (mode & CRYPTO_LOCK)
      pthread_rwlock_wrlock(&cdv->rwlock);
   else
      pthread_rwlock_unlock(&cdv->rwlock);
}

void BaseSock::ssl_dyn_destroy(CRYPTO_dynlock_value *cdv, const char *, int)
{
   pthread_rwlock_destroy(&cdv->rwlock);
   delete cdv;
}
#endif
