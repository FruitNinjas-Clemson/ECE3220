Austin Johnson
CPSC 3220
Project #2


KNOWN PROBLEMS: Interrupts are supposed to be enabled but aren't somewhere in my
			    code. I'm not entirely sure the exact location.

DESCRIPTION: Project #2 is my implementation of a user mode thread library. It
             provides programmers with mutex locks and condition variables for
             synchronizing data access. It uses multiple functions that allow
             the user to create, yield, join and exit threads. It also allows
             the user to lock, unlock, wait and signal threads for synchronization.

DESIGN: My implementation uses lists that function as a queue (FIFO). Any new
        new threads are added to the end of the list and the head of the list
        is always the thread that will execute next.There are three lists being
        used, one for currently running threads (thread_pool), one for exited
        threads (completed_threads) and one for threads waiting on a condition
        variable (waiting_threads). The waiting thread list is a 2D array where
		each cell in the array is its own linked list. This list contains threads
		waiting on a condition with a lock number. If a thread is currently running
		and has not exited, it is ALWAYS at the tail of the thread_pool.
