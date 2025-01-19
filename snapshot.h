/*  snapshot.h | Intel, Arm64 and Mips replacement machine-states from the libtask library. */

#if defined __x86_64__

#include <signal.h>

struct mcontext {
  /*
   * The first 20 fields must match the definition of
   * sigcontext. So that we can support sigcontext
   * and ucontext_t at the same time.
   */
   long mc_onstack;     /* XXX - sigcontext compat. */
   long mc_rdi;         /* machine state (struct trapframe) */
   long mc_rsi, mc_rdx, mc_rcx, mc_r8, mc_r9, mc_rax, mc_rbx, mc_rbp, 
   mc_r10, mc_r11, mc_r12, mc_r13, mc_r14, mc_r15, mc_trapno, mc_addr, 
   mc_flags, mc_err, mc_rip, mc_cs, mc_rflags, mc_rsp, mc_ss;
   
   long mc_len; /* sizeof(mcontext_t) */
#define _MC_FPFMT_NODEV     0x10000 /* device not present or configured */
#define _MC_FPFMT_XMM       0x10002
   long mc_fpformat;
#define _MC_FPOWNED_NONE    0x20000 /* FP state not used */
#define _MC_FPOWNED_FPU     0x20001 /* FP state came from FPU */
#define _MC_FPOWNED_PCB     0x20002 /* FP state came from PCB */
   long mc_ownedfp;
 /*
  * See <machine/fpu.h> for the internals of mc_fpstate[].
  */
   long mc_fpstate[64];
   long mc_spare[8];
};

struct ucontext {
   /*
    * Keep the order of the first two fields. Also,
    * keep them the first two fields in the structure.
    * This way we can have a union with struct
    * sigcontext and ucontext_t. This allows us to
    * support them both at the same time.
    * note: the union is not defined, though.
    */
   sigset_t uc_sigmask;
   struct mcontext uc_mcontext;
   
   struct ucontext * uc_link;
   stack_t uc_stack;
   int __spare__[8];
};

#elif defined __mips__

struct mcontext {
    /*
     * These fields must match the corresponding fields in struct 
     * sigcontext which follow 'sc_mask'. That way we can support
     * struct sigcontext and ucontext_t at the same time.
     */
    int     mc_onstack; /* sigstack state to restore */
    int     mc_pc; /* pc at time of signal */
    int     mc_regs[32]; /* processor regs 0 to 31 */
    int     sr; /* status register */
    int     mullo, mulhi; /* mullo and mulhi registers... */
    int     mc_fpused; /* fp has been used */
    int     mc_fpregs[33]; /* fp regs 0 to 31 and csr */
    int     mc_fpc_eir; /* fp exception instruction reg */
    void    *mc_tls; /* pointer to TLS area */
    int	__spare__[8]; /* XXX reserved */
};

struct __sigaltstack { 
    void     *ss_sp;         /* signal stack base */
    long     ss_size;        /* signal stack length */
    int      ss_flags;       /* SA_DISABLE and/or SA_ONSTACK */
};
typedef struct __sigaltstack stack_t;

struct ucontext {
    /*
     * Keep the order of the first two fields. Also,
     * keep them the first two fields in the structure.
     * This way we can have a union with struct
     * sigcontext and ucontext_t. This allows us to
     * support them both at the same time.
     * note: the union is not defined, though.
     */
    long uc_sigmask;
    struct mcontext uc_mcontext;
    
    struct ucontext * uc_link;
    stack_t uc_stack;
    int uc_flags;
    int __spare__[4];
};

#elif defined __arm64__

#include <signal.h>

struct mcontext {
   unsigned long fault_address;
   unsigned long regs[31];
   unsigned long sp,pc,pstate;
   unsigned long __reserved[256];
};

struct ucontext {
   sigset_t uc_sigmask;
   struct mcontext uc_mcontext;
   struct ucontext * uc_link;
   stack_t uc_stack;
};

#endif

#define	setcontext(u) setmcontext(&(u)->uc_mcontext)
#define	getcontext(u) getmcontext(&(u)->uc_mcontext)
int swapcontext(struct ucontext *, const struct ucontext *);
void makecontext(struct ucontext *, void (*)(void), int, ...);
int getmcontext(struct mcontext *);
void setmcontext(const struct mcontext *);

