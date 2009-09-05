#if 0 /* -*- mode: c; c-file-style: "gnu"; tab-width: 8; -*-
 set -e; TRG=`basename $0 .c`; rm -f "$TRG"
 WARN="-Wall -Wstrict-prototypes -pedantic -Wno-long-long"
 WARN="$WARN -Wcast-align -Wpointer-arith " # -Wfloat-equal #-Werror
 WARN="$WARN -W -Wwrite-strings -Wcast-qual -Wshadow" # -Wconversion
 date=`date`; set -x
 #${CC:-gcc} -ggdb $WARN "$@" -o "$TRG" "$0" -DCDATE="\"$date\""
 ${CC:-gcc} -O2 $WARN "$@" -o "$TRG" "$0" -DCDATE="\"$date\""
 exit 0
 */
#endif


#ifndef __i386__
# error x86 assembly code
#endif

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <pthread.h>
#include <poll.h>

#define rdtscll(val) \
    do { __asm__ __volatile__("rdtsc" : "=A" (val)); } while (0)


#define measure_start(ntrials) do {  \
    volatile unsigned int __mv_ini, __mv_end, __mv_now, __mv_i; mv_res = ~0; \
    for (__mv_i = 0; __mv_i < ntrials; __mv_i++) { rdtscll(__mv_ini);

#define measure_end() \
  rdtscll(__mv_end); __mv_now = __mv_end - __mv_ini; \
  if (__mv_now < mv_res) mv_res = __mv_now; }} while(0)

unsigned int mv_res, mv_overhead;

unsigned int result(void) { return mv_res - mv_overhead; }

void init_measure(void)
{
  measure_start(10000);
  measure_end();
  mv_overhead = mv_res;
}

void measure_constant_memx(void)
{
  char buf1[32768], buf2[32768];

  /* memset and memcpy with constant values. compiler
     may optimize these (and gcc usually does) */

#define BUFLEN 1024

  measure_start(10000);
  memset(buf1, 0, BUFLEN);
  measure_end();
  printf("memset(%d) (gcc-gen): %i ticks\n", BUFLEN, result());

  measure_start(10000);
  memcpy(buf2, buf1, BUFLEN);
  measure_end();
  printf("memcpy(%d) (gcc-gen): %i ticks\n", BUFLEN, result());


#undef BUFLEN
#define BUFLEN 8192

  measure_start(10000);
  memset(buf1, 0, BUFLEN);
  measure_end();
  printf("memset(%d) (gcc-gen): %i ticks\n", BUFLEN, result());

  measure_start(10000);
  memcpy(buf2, buf1, BUFLEN);
  measure_end();
  printf("memcpy(%d) (gcc-gen): %i ticks\n", BUFLEN, result());

#undef BUFLEN
#define BUFLEN (sizeof buf1)

  measure_start(10000);
  memset(buf1, 0, BUFLEN);
  measure_end();
  printf("memset(%d) (gcc-gen): %i ticks\n", BUFLEN, result());

  measure_start(10000);
  memcpy(buf2, buf1, BUFLEN);
  measure_end();
  printf("memcpy(%d) (gcc-gen): %i ticks\n", BUFLEN, result());

  measure_start(10000);
  memmove(buf2, buf1, BUFLEN);
  measure_end();
  printf("memmove(%d) (gcc-gen): %i ticks\n", BUFLEN, result());


#undef BUFLEN
}

char * measure_variable_memx0(int buflen, char * buf2)
{
  char buf1[32768];

  measure_start(10000);
  memset(buf2, 0, buflen);
  measure_end();
  printf("memset(%d): %i ticks\n", buflen, result());

  measure_start(10000);
  memcpy(buf2, buf1, buflen);
  measure_end();
  printf("memcpy(%d): %i ticks\n", buflen, result());

  measure_start(10000);
  memmove(buf2, buf1, buflen);
  measure_end();
  printf("memmove(%d): %i ticks\n", buflen, result());

  return buf2;
}

void bbb_memzero(char * dst, int len)
{
  while (len--)
      *dst++ = '\0';
}

void bbb_memcpy(char * dst, char * src, int len)
{
  while (len--)
      *dst++ = *src++;
}

char * measure_byte_by_byte(int buflen, char * buf2)
{
  char buf1[32768];

  measure_start(10000);
  bbb_memzero(buf2, buflen);
  measure_end();
  printf("byte_by_byte zero (%d): %i ticks\n", buflen, result());

  measure_start(10000);
  bbb_memcpy(buf1, buf2, buflen);
  measure_end();
  printf("byte_by_byte copy (%d): %i ticks\n", buflen, result());

  return buf2;
}

void measure_variable_memx(void)
{
  char buf2[32768];
  (void)measure_variable_memx0(1024, buf2);
  (void)measure_variable_memx0(8192, buf2);
  (void)measure_variable_memx0(32768, buf2);
  puts("unaligned:");
  (void)measure_variable_memx0(32765, buf2 + 3);
  (void)measure_byte_by_byte(32768, buf2);

}

void memzeroq(void * dst, size_t len);
void memcpyql(void * dst, void const * src, size_t len);

char * measure_variable_memxq0(int buflen, char * buf2)
{
  char buf1[32768];

  measure_start(10000);
  memzeroq(buf1, buflen);
  measure_end();
  printf("memzeroq(%d): %i ticks\n", buflen, result());

  measure_start(10000);
  memcpyql(buf2, buf1, buflen);
  measure_end();
  printf("memcpyql(%d): %i ticks\n", buflen, result());

  return buf2;
}


void measure_variable_memxq(void)
{
  char buf2[32768];
  (void)measure_variable_memxq0(1024, buf2);
  (void)measure_variable_memxq0(8192, buf2);
  (void)measure_variable_memxq0(32768, buf2);

}


void measure_pthread(void)
{
  pthread_mutex_t mutex;

  pthread_mutex_init(&mutex, NULL);
  measure_start(10000);
  pthread_mutex_lock(&mutex);
  pthread_mutex_unlock(&mutex);
  measure_end();
  printf("pthread_mutex_(un)lock(): %i ticks\n", result());
}


void measure_poll(void)
{
  #define POLLTO 10
  measure_start(100);
  poll(NULL, 0, POLLTO);
  measure_end();
  printf("poll(0,0, %d): %i Kticks\n", POLLTO, result() / 1000);
  #undef POLLTO
}

void measure_read_syscall_overhead(void)
{
  char c;

  measure_start(10000);
  read(0, &c, 0);
  measure_end();
  printf("read(0,0,0): %i ticks\n", result());

}


int main(void)
{
  init_measure();
  printf("overhead: %d\n-\n", mv_overhead);
  measure_constant_memx();
  puts("-");
  measure_variable_memx();
  puts("-");
  measure_variable_memxq();
  puts("-");
  measure_pthread();
  puts("-");
  measure_poll();
  puts("-");
  measure_read_syscall_overhead();
  puts("");
  return 0;
}

/* helper functions */


/*
 * modified gnu (which is byte-by-byte) (from "optimized memcpy improves speed)
 * overlapping copies to lower memory access work
 * ( but we do not care return value )
 */
void memcpyql(void * dst, void const * src, size_t len)
{
    uint32_t * plDst = (uint32_t *) dst;
    uint32_t const * plSrc = (uint32_t const *) src;
    uint8_t * pcDst;
    uint8_t const * pcSrc;
#if 1
    if (!((intptr_t)src & 3) && !((intptr_t)dst & 3))
    {
        while (len >= 4)
	{
            *plDst++ = *plSrc++;
            len -= 4;
	}
    }
#endif
#if 1
    pcDst = (uint8_t *) plDst;
    pcSrc = (uint8_t const *) plSrc;

    while (len--)
    {
        *pcDst++ = *pcSrc++;
    }
#endif
}

/*
 * "optimized" memset with '\0'. derived from above.
 * note that compiled may optimize memset() away, so do not use this always
 */
void memzeroq(void * dst, size_t len)
{
    uint32_t * plDst = (uint32_t *) dst;
    uint8_t * pcDst;
#if 1
    if (!((intptr_t)dst & 3)) {
	while (len >= 4) { *plDst++ = 0; len -= 4; } }
#endif
#if 1
    pcDst = (uint8_t *) plDst;
    while (len--) *pcDst++ = 0;
#endif
}

/*
 * Local variables:
 * mode: c
 * c-file-style: "stroustrup"
 * tab-width: 8
 * End:
 */
