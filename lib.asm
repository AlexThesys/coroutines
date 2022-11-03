; %define SPINLOCK

    extern thread_stack_ptr    

    extern execute_task
    extern semaphore_signal ; void semaphore_signal(semaphore* sem) 
    extern set_free_stack ; void set_free_stack(u64)
%ifndef SPINLOCK
    extern pthread_mutex_unlock
%endif
    global save_and_yield_impl ; void save_and_yield_impl(yielded_task* yt, volatile s64* || pthread_mutex_lock*, semaphore* sem)
    global launch_task ; void launch_task(void*, task, u8*, u64)
    global resume_yielded_task ; void resume_yielded_task(yielded_task* yt)
    global set_thread_stack_ptr ; u8* set_thread_stack_ptr()

    section .text
save_and_yield_impl:
    mov rax, qword [rsp] ; save return address ; pop rax
    mov qword [rdi], rax
    ; save all callee-preserved registers
    mov qword [rdi+0x08], rbx
    mov qword [rdi+0x10], rbp
    mov qword [rdi+0x18], rsp
    mov qword [rdi+0x20], r12
    mov qword [rdi+0x28], r13
    mov qword [rdi+0x30], r14
    mov qword [rdi+0x38], r15
    push rsi
    mov rdi, rdx
    call semaphore_signal
    pop rsi
%ifdef SPINLOCK
    xor eax, eax
    mov dword [rsi], eax     
%else
    mov rdi, rsi
    call pthread_mutex_unlock
%endif 
    jmp execute_task

launch_task:
    mov rsp, rdx
    push rcx ; save stack_id
    mov rdx, rsp
    mov r8, 0x0F
    and rdx, r8
    not r8
    and rsp, r8 ; align the stack
    push rdx
    sub rsp, 0x08
    ; void* params is already in rdi
    call rsi
    add rsp, 0x08
    pop rdx
    or rsp, rdx
    pop rdi ; restore stack_id
    call set_free_stack
    mov rax, qword [rel thread_stack_ptr wrt ..gottpoff] ; get_address of the original stack of this thread
    mov rsp, qword [fs:rax] ; restore stack
    mov rbp, rsp
    mov rax, execute_task ; lea rax, [execute_task-0x01]
    sub rax, 0x01 ; skip push rbp
    jmp rax
    

resume_yielded_task:
    ; save all callee-preserved registers
    mov rbx, qword [rdi+0x08]
    mov rbp, qword [rdi+0x10]
    mov rsp, qword [rdi+0x18]
    mov r12, qword [rdi+0x20]
    mov r13, qword [rdi+0x28]
    mov r14, qword [rdi+0x30]
    mov r15, qword [rdi+0x38]
    jmp [rdi]
    
set_thread_stack_ptr:
    mov rax, rbp
    ret

    ;section .data
    ;section .bss
