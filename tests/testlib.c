#include <unistd.h>
#include <android/log.h>

void __attribute__ ((constructor)) yoyoyo() {
  __android_log_print( ANDROID_LOG_INFO, "ANDROSWAT", "Yo, I'm the injected library inside process %d!\n", getpid() );
}
