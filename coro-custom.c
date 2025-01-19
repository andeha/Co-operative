/*  coro-custom.c | coroutines in c and assembler. */

#include "snapshot.h"
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

#define FALSE 0
#define TRUE (! FALSE)
typedef int64_t __builtin_int_t;

typedef struct __coro_t coro_t;
typedef int (* coro_function_t)(coro_t * coro);

typedef struct __coro_t {
  coro_function_t function;
  struct ucontext original;
  struct ucontext overlay;
  __builtin_int_t yield;
  int isFinished;
} coro_t;

coro_t * coro_await(coro_function_t function);
__builtin_int_t coro_resume(coro_t * coro);
void coro_feedback(coro_t * coro, __builtin_int_t value);
void coro_free(coro_t * coro);

void __coroutine_entry_point(coro_t * coro)
{
   int value = coro->function(coro);
   coro->isFinished = TRUE;
   coro_feedback(coro,value);
}

coro_t * coro_await(coro_function_t function)
{
   coro_t * coro = malloc(sizeof(struct __coro_t));
   if (getcontext(&coro->overlay) == -1) { return NULL; }
   coro->isFinished = FALSE;
   coro->function = function;
#define BYTES_STACK 8388608
   coro->overlay.uc_stack.ss_sp = malloc(BYTES_STACK);
   coro->overlay.uc_stack.ss_size = BYTES_STACK; /* MINSIGSTKSZ alt. SIGSTKSZ */
   coro->overlay.uc_stack.ss_flags = 0;
   coro->overlay.uc_link = &coro->original;
   if (sigemptyset(&coro->overlay.uc_sigmask) < 0) { }
   makecontext(&coro->overlay, (void (*)())__coroutine_entry_point, 1, coro);
   if (swapcontext(&coro->original,&coro->overlay) == -1) { return NULL; }
   return coro;
}

__builtin_int_t coro_resume(coro_t * coro)
{
   if (coro->isFinished) return -1;
   if (swapcontext(&coro->original,&coro->overlay)) { return -1; }
   return coro->yield;
}

void coro_feedback(coro_t * coro, __builtin_int_t value)
{
   coro->yield = value;
   if (swapcontext(&coro->overlay,&coro->original)) { return; }
}

void coro_free(coro_t * coro)
{
   free(coro->overlay.uc_stack.ss_sp);
   free(coro);
}

#pragma recto Program

int corout₋helloworld(coro_t * coro)
{
   printf("Hello - part 1\n");
   coro_feedback(coro,1); /* ⬷ suspension point that returns the value `1`. */
   printf("World - part 2\n");
   return 2;
}

int
main(
  int argc, 
  const char * argv[]
)
{
   coro_t * coro = coro_await(corout₋helloworld);
   if (coro == NULL) { return 1; }
   printf("yield = %lld\n", coro->yield);
   __builtin_int_t second = coro_resume(coro);
   printf("yield = %lld\n", second);
   __builtin_int_t third = coro_resume(coro); /* checking restarting finalized call. */
   printf("yield = %lld\n", third);
   coro_free(coro);
   printf("done\n");
   return 0;
}

/* clang -o coroutines1 -g coxrout-1.S coxrout-2.c coro-custom.c */


