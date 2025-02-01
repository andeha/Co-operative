/*  coro-builtin.c | coroutines and UNIX. */

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#define _XOPEN_SOURCE
#include <ucontext.h>

#define FALSE 0
#define TRUE (!FALSE)
typedef int64_t __builtin_int_t;

typedef struct __coro_t coro_t;
typedef int (* coro_function_t)(coro_t * coro);
typedef union { void * pointer; uint32_t part[2]; } AAPLArm64TwoArguments;

typedef struct __coro_t {
  coro_function_t function;
  ucontext_t original;
  __builtin_int_t buffer1[50];
  ucontext_t overlay;
  __builtin_int_t buffer2[50];
  __builtin_int_t yield;
  int isFinished;
} coro_t;

coro_t * coro_await(coro_function_t function);
__builtin_int_t coro_resume(coro_t * coro);
void coro_feedback(coro_t * coro, __builtin_int_t value);
void coro_free(coro_t * coro);

#pragma clang diagnostic ignored "-Wdeprecated-declarations"

void __coroutine_entry_point(uint32_t one, uint32_t two)
{ AAPLArm64TwoArguments argh; argh.part[0]=one; argh.part[1]=two;
   coro_t * coro = argh.pointer;
   __builtin_int_t value = coro->function(coro);
   coro->isFinished = TRUE;
   coro_feedback(coro,value);
}

coro_t * coro_await(coro_function_t function)
{
   int bytes = sizeof(struct __coro_t);
   coro_t * coro = malloc(bytes);
   if (getcontext(&coro->overlay) == -1) { return NULL; }
   coro->function = function;
   coro->isFinished = FALSE;
#define BYTES_STACK 8372224
   coro->overlay.uc_stack.ss_sp = malloc(BYTES_STACK);
   coro->overlay.uc_stack.ss_size = BYTES_STACK;
   coro->overlay.uc_stack.ss_flags = 0;
   coro->overlay.uc_link = &coro->original;
   sigemptyset(&coro->overlay.uc_sigmask);
   AAPLArm64TwoArguments two = { .pointer=coro };
   makecontext(&coro->overlay,(void (*)(void))__coroutine_entry_point,2,two.part[0],two.part[1]);
   if (swapcontext(&coro->original,&coro->overlay) == -1) { return NULL; }
   return coro;
}

__builtin_int_t coro_resume(coro_t * coro)
{
   if (coro->isFinished) { return -1; }
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
   /* free(coro->overlay.uc_stack.ss_sp); */
   free(coro);
}

#pragma recto Program

int corout_helloworld(coro_t * coro)
{
    printf("Hello - part 1\n");
    coro_feedback(coro,1);
    printf("Hello - part 2\n");
    return 2;
}

int
main(
  int argc,
  char ** argv
)
{
    coro_t * coro = coro_await(corout_helloworld);
    if (coro == NULL) { return 1; }
    printf("yield = %lld\n", coro->yield);
    __builtin_int_t second = coro_resume(coro);
    printf("yield = %lld\n", second);
    coro_free(coro);
    printf("done\n");
    return 0;
}

/* clang -o coroutine2 -g coro-builtin.c */

