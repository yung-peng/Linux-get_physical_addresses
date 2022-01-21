# Linux-get_physical_addresses

## Problem 1
寫一個新的system call(my_get_physical_addresses)，讓process可以使用此system call獲得一些virutal addressed的physical addresses。

寫一個具有三個threads的multi-thread程式，透過新的system call顯示這些thread如何共享以下的記憶體區段(code segments, data segments, BSS segments, heap segments, libraries, stack segments, and thread local storages)。

## Problem 2
寫一個程式透過新的system call去檢查同時執行該程式的兩個process是如何共享記憶體區段。

## Compile
User space的code須以以下指令編譯：gcc xxx.c -lpthread -o xxx

SYSCALL_DEFINEn(syscall_name ), n需與後面傳入參數量一致，若不相同會出現"error: macro “__MAP2” requires 4 arguments, but only 2 given" 的錯誤
## Problem 1執行結果
```
=========main==============
==========virtual address===========
code: 0x5643594ba89a
data: 0x5643596bc014
bss: 0x5643596bc020
heap: 0x56435ab3c260    
share_lib: 0x7fee46acaf70
stack: 0x7ffd75c03090    
=========physical address===========
code: 0x112b3a89a
data: 0x1115cd014
bss: 0x1115cd020
heap: 0x111631260       
share_lib: 0x13fbbcf70
stack: 0x1115cc090       
==========Thread 1===============
==========virtual address===========
code: 0x5643594ba89a
data: 0x5643596bc014
bss: 0x5643596bc020
heap: 0x7fee40000b20    
share_lib: 0x7fee46acaf70
stack: 0x7fee46a64ed0    
=========physical address===========
code: 0x112b3a89a 
data: 0x1115cd014
bss: 0x1115cd020
heap: 0x1344a3b20       
share_lib: 0x13fbbcf70
stack: 0x111632ed0       
=========thread2=============
==========virtual address===========
code: 0x5643594ba89a
data: 0x5643596bc014
bss: 0x5643596bc020
heap: 0x7fee40000b40    
share_lib: 0x7fee46acaf70
stack: 0x7fee46263ed0    
=========physical address===========
code: 0x112b3a89a
data: 0x1115cd014 
bss: 0x1115cd020 
heap: 0x1344a3b40       
share_lib: 0x13fbbcf70
stack: 0x11296ced0       
```
## Problem 2執行結果
```
process1

=========main==============
==========virtual address===========
code:     0x55e707cfe7ea
data:     0x55e707eff014
bss:      0x55e707eff020
heap:     0x55e709cc7260    
share_lib:0x7f27daf3df70
stack:    0x7ffeecb21f10    
=========physical address===========
code:     0x92d8a7ea
data:     0x1dd47014
bss:      0x1dd47020
heap:     0x1dd47018        
share_lib:0x13fbbcf70
stack:    0x93d5ff10 

process2
=========main==============
==========virtual address===========
code:     0x5630027a57ea 
data:     0x5630029a6014 
bss:      0x5630029a6020 
heap:     0x563004445260    
share_lib:0x7f67ee8fbf70
stack:    0x7ffc7dfc3340    
=========physical address===========
code:     0x92d8a7ea        
data:     0x92e28014
bss:      0x92e28020
heap:     0x92e28018        
share_lib:0x13fbbcf70
stack:    0x944d8340  
```

## Q&A
1. 當程式調用memory allocation的方法，如：malloc、new時，OS會如何處理？
答: Process分配memory有兩種方式：分別由brk和mmap完成。brk是將data段的最高地址指針_edata往高地址推，mmap是在process的虛擬地址空間中，找一塊空閒的虛擬空間
2. 說明multithread和multiprocess記憶體共用情形並畫圖。
答: multithread除了stack與heap段不共用外，其餘均共用。 multiprocess除了code與share library段共用外，其餘均不共用。
![](https://i.imgur.com/DX8u3Up.jpg)


3. 說明當程式執行pthread_create時，system call的執行流程。
答: 
* 使用strace看使用到的system call(mmap, mprotect, clone)
* 接著看clone的code，呼叫到_do_fork
```
SYSCALL_DEFINE5(clone, unsigned long, clone_flags, unsigned long, newsp,
                int __user *, parent_tidptr,
                int __user *, child_tidptr,                                                                                          
                unsigned long, tls)         
{                
       return _do_fork(clone_flags, newsp, 0, parent_tidptr, child_tidptr, tls);
}      
```
* 接著看_do_fork，呼叫到copy_process來複製一個新的process
```
/*
 *  Ok, this is the main fork-routine.
 *
 * It copies the process, and if successful kick-starts
 * it and waits for it to finish using the VM if required.
 */
long do_fork(unsigned long clone_flags,
	      unsigned long stack_start,
	      struct pt_regs *regs,
	      unsigned long stack_size,
	      int __user *parent_tidptr,
	      int __user *child_tidptr)
{
	struct task_struct *p; // 宣告一個 process descriptor
	int trace = 0;
	struct pid *pid = alloc_pid(); // 要求一個 PID 給新的 process 使用
	long nr;

	if (!pid)
		return -EAGAIN;
	nr = pid->nr;
	if (unlikely(current->ptrace)) {
		trace = fork_traceflag (clone_flags);
		if (trace)
			clone_flags |= CLONE_PTRACE;
	}
	// 呼叫 copy_process()，以複制出新的 process
	p = copy_process(clone_flags, stack_start, regs, stack_size, parent_tidptr, child_tidptr, nr);
	/*
	 * Do this prior waking up the new thread - the thread pointer
	 * might get invalid after that point, if the thread exits quickly.
	 */
	if (!IS_ERR(p)) {
		struct completion vfork;

		if (clone_flags & CLONE_VFORK) {
			p->vfork_done = &vfork;
			init_completion(&vfork);
		}

		if ((p->ptrace & PT_PTRACED) || (clone_flags & CLONE_STOPPED)) {
			/*
			 * We'll start up with an immediate SIGSTOP.
			 */
			sigaddset(&p->pending.signal, SIGSTOP);
			set_tsk_thread_flag(p, TIF_SIGPENDING);
		}

		if (!(clone_flags & CLONE_STOPPED))
			wake_up_new_task(p, clone_flags);
		else
			p->state = TASK_STOPPED;

		if (unlikely (trace)) {
			current->ptrace_message = nr;
			ptrace_notify ((trace << 8) | SIGTRAP);
		}

		if (clone_flags & CLONE_VFORK) {
			wait_for_completion(&vfork);
			if (unlikely (current->ptrace & PT_TRACE_VFORK_DONE))
				ptrace_notify ((PTRACE_EVENT_VFORK_DONE << 8) | SIGTRAP);
		}
	} else {
		free_pid(pid);
		nr = PTR_ERR(p);
	}
	return nr;
}
```
* copy_process開始複製current給新的process：
```
	if ((retval = copy_semundo(clone_flags, p)))
		goto bad_fork_cleanup_audit;
	if ((retval = copy_files(clone_flags, p)))
		goto bad_fork_cleanup_semundo;
	if ((retval = copy_fs(clone_flags, p)))
		goto bad_fork_cleanup_files;
	if ((retval = copy_sighand(clone_flags, p)))
		goto bad_fork_cleanup_fs;
	if ((retval = copy_signal(clone_flags, p)))
		goto bad_fork_cleanup_sighand;
	if ((retval = copy_mm(clone_flags, p)))
		goto bad_fork_cleanup_signal;
	if ((retval = copy_keys(clone_flags, p)))
		goto bad_fork_cleanup_mm;
	if ((retval = copy_namespace(clone_flags, p)))
		goto bad_fork_cleanup_keys;
	retval = copy_thread(0, clone_flags, stack_start, stack_size, p, regs);
	if (retval)
		goto bad_fork_cleanup_namespace;
```
    流程依序為：clone->_do_fork->copy_process->copy_mm
4. virtual address 什麼時候連上 physical address ?
答：在第一次訪問到資料時，對應的PTE的valid bit是沒有被設置的，此時為page fault。當遇到page fault時，MMU會將控制權還給OS，這時交由Page fault handler來處理，handler會選擇某個在DRAM中的page來替換。(page fault handler)

5. current macro 在不同thread用到時是同個task struct?
答: 不同thread皆有自己的pid，因此不同thread的task struct不會一樣。



## Reference
1. [Stack Overflow](https://stackoverflow.com/questions/41090469/linux-kernel-how-to-get-physical-address-memory-management?fbclid=IwAR1hAHO4eZy7BhUIFCfVxtTsBkP5njKV31jj7kRU1p10Y3mqefzmfLFiOic)
2. [Module教學](https://jerrynest.io/how-to-write-a-linux-kernel-module/)
3. [Page Table Management](https://www.kernel.org/doc/gorman/html/understand/understand006.html?fbclid=IwAR3gGfrQmiSeGSsTO2b_0cHaneq1TVs-lnjimZLG2MaTojJM-25CCHQG8ZU)
4. [copy_process()](https://www.jollen.org/blog/2007/01/process_creation_5_copy_process.html?fbclid=IwAR0Hrlti9YLluxAwbRxj9DMzpytCN_GSAPQB2YHMKH1EtGLWrsEFjH6vGeA)
5. [內存的分配與管理](https://codertw.com/%E7%A8%8B%E5%BC%8F%E8%AA%9E%E8%A8%80/676150/)

## Contributor
Yung-Peng Hsu、Ruei-Yuan Wang、Cheng-Kai Wang
