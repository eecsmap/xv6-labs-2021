#include "types.h"
#include "param.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

int pageaccess(uint64 base, int len, uint64 mask)
{
    if (len > 32) return -1; // only support 32 pages at most
    base = PGROUNDDOWN(base);
    struct proc *p = myproc();
    uint data = 0;
    for (int i = 0; i < len; i++) {
        uint64 page_va = base + i * PGSIZE;
        pte_t *pte = walk(p->pagetable, page_va, 0);
        if (*pte & PTE_A) {
            data |= (1 << i);
            *pte &= ~PTE_A;
        }
    }
    if (copyout(p->pagetable, mask, (char *)&data, sizeof data) < 0) {
        return -1;
    }
    return 0;
}
