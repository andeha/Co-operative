/* context-2.c | makecontext and swapcontext replacements originally from libtask. */

#include "snapshot.h"

#if defined __x86_64__

void * memset(void *b, int c, size_t len);

void makecontext(struct ucontext *ucp, void (*func)(void), int argc, ...)
{
   long *sp;
   __builtin_va_list va;
   memset(&ucp->uc_mcontext, 0, sizeof ucp->uc_mcontext);
   /* if(argc != 2) *(int*)0 = 0; */
   __builtin_va_start(va,argc);
   ucp->uc_mcontext.mc_rdi = (long)__builtin_va_arg(va, void *);
   /* ucp->uc_mcontext.mc_rsi = va_arg(va, int); */
   __builtin_va_end(va);
   sp = (long*)ucp->uc_stack.ss_sp+ucp->uc_stack.ss_size/sizeof(long);
   sp -= argc;
   sp = (void*)((uintptr_t)sp - (uintptr_t)sp%16);	/* 16-align for OS X */
   *--sp = 0; /* return address */
   ucp->uc_mcontext.mc_rip = (long)func;
   ucp->uc_mcontext.mc_rsp = (long)sp;
}

#elif defined __mips__

void makecontext(struct ucontext *uc, void (*fn)(void), int argc, ...)
{
   int i, *sp;
   __builtin_va_list arg;
   
   __builtin_va_start(arg,argc);
   sp = (int*)uc->uc_stack.ss_sp+uc->uc_stack.ss_size/4;
   for(i=0; i<4 && i<argc; i++)
   	uc->uc_mcontext.mc_regs[i+4] = __builtin_va_arg(arg, int);
   __builtin_va_end(arg);
   uc->uc_mcontext.mc_regs[29] = (int)sp;
   uc->uc_mcontext.mc_regs[31] = (int)fn;
}

#elif defined __arm64__

void makecontext(struct ucontext *uc, void (*fn)(void), int argc, ...)
{
   unsigned long *sp;
   unsigned long *regp;
   va_list va;
   int i;
   
   sp = (unsigned long *) ((uintptr_t) ucp->uc_stack.ss_sp + ucp->uc_stack.ss_size);
   sp -= argc < 8 ? 0 : argc - 8;
   sp = (unsigned long *) (((uintptr_t) sp & -16L));
   
   ucp->uc_mcontext.sp = (uintptr_t) sp;
   ucp->uc_mcontext.pc = (uintptr_t) func;
   ucp->uc_mcontext.regs[19] = (uintptr_t) ucp->uc_link;
   ucp->uc_mcontext.regs[30] = (uintptr_t) &__start_context;
   
   va_start(va, argc);
   
   regp = &(ucp->uc_mcontext.regs[0]);
   
   for (i = 0; (i < argc && i < 8); i++)
      *regp++ = va_arg (va, unsigned long);
   
   for (; i < argc; i++)
      *sp++ = va_arg (va, unsigned long);
   
   va_end(va);
}

#endif

int swapcontext(struct ucontext *oucp, const struct ucontext *ucp)
{
   if (getcontext(oucp) == 0) setcontext(ucp);
   return 0;
}

