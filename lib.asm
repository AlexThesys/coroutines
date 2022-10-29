; %define SPINLOCK

    extern execute_task
%ifndef SPINLOCK
    extern pthread_mutex_unlock
%endif
    global save_and_yield_impl ; void save_and_yield(yielded_task* yt, volatile s64* || pthread_mutex_lock*)
    global launch_task ; void launch_task(void*, task, u8*)
    global resume_yielded_task ; void resume_yielded_task(yielded_task* yt)

    section .text
save_and_yield_impl:
    pop rax ; save return address 
    mov qword [rdi], rax
    ; save all callee-preserved registers
    mov qword [rdi+0x08], rbx
    mov qword [rdi+0x10], rbp
    mov qword [rdi+0x18], rsp
    mov qword [rdi+0x20], r12
    mov qword [rdi+0x28], r13
    mov qword [rdi+0x30], r14
    mov qword [rdi+0x38], r15
%ifdef SPINLOCK
    xor eax, eax
    mov dword [rsi], eax     
%else
    push rdi
    mov rdi, rsi
    call pthread_mutex_unlock
    pop rdi
%endif 
    jmp execute_task

launch_task:
    push rbp
    mov rbp, rsp
    mov rsp, rdx
    push rbp
    sub rsp, 0x08
    ; void* params is already in rdi
    call rsi
    add rsp, 0x08
    pop rbp
    mov rsp, rbp
    pop rbp
    ret
    

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
    

    ;section .data
    ;section .bss
