#include <dlfcn.h>
#include <execinfo.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#define DYLD_INTERPOSE(_replacment,_replacee) \
   __attribute__((used)) static struct{ const void* replacment; const void* replacee; } _interpose_##_replacee \
            __attribute__ ((section ("__DATA,__interpose"))) = { (const void*)(unsigned long)&_replacment, (const void*)(unsigned long)&_replacee };

#define MAX_FRAMES (32)

typedef struct {
  int     count;
  void *  frames[MAX_FRAMES];
} tlsinfo_t;

static pthread_key_t tls_key = 0;

static tlsinfo_t * get_info() {
  if(tls_key == 0) {
    return NULL;
  }

  return (tlsinfo_t *)pthread_getspecific(tls_key);
}

void swift_willThrow(void *unused, void *error);

void swift_unexpectedError(void *object,
                           void *filenameStart,
                           long filenameLength,
                           bool isAscii,
                           unsigned long line);


void interposed_swift_willThrow(void *unused, void *error) {
  if(tls_key == 0) {
    pthread_key_create(&tls_key, NULL);
  }

  tlsinfo_t *info = get_info();
  if(!info) {
    info = (tlsinfo_t *)malloc(sizeof(tlsinfo_t));

    memset(info, 0, sizeof(tlsinfo_t));

    pthread_setspecific(tls_key, info);
  }

  if(info) {
    info->count = backtrace(info->frames, MAX_FRAMES);
  }

  swift_willThrow(unused, error);
}
DYLD_INTERPOSE(interposed_swift_willThrow, swift_willThrow)

void interposed_swift_unexpectedError(void *object,
                           void *filenameStart,
                           long filenameLength,
                           bool isAscii,
                           unsigned long line) {

  tlsinfo_t *info = get_info();
  if(info) {

    backtrace_symbols_fd(info->frames, info->count, 2);

  }

  swift_unexpectedError(object, filenameStart, filenameLength, isAscii, line);
}
DYLD_INTERPOSE(interposed_swift_unexpectedError, swift_unexpectedError)


