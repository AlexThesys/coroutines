    %define STACK_ALIGN 0xFFFFFFFFFFFFFFF0

    section .text

    extern semaphore_signal ; void semaphore_signal(semaphore* sem) 
    extern restore_state ; void restore_state(exec_state* state)
    extern set_free_stack ; void set_free_stack(u64 stack_id)
    extern execute_task_inner ; void* execute_task_inner(void*)
    extern set_thread_stack_ptr ; void set_thread_stack_ptr(u8*)

    global co_yield_impl ; void co_yield_impl (yielded_task* yt, volatile s64* || pthread_mutex_lock*, semaphore* sem)
    global launch_task ; void launch_task(void*, task, u8*, u64)
    global resume_yielded_task ; void resume_yielded_task(yielded_task* yt)
    global execute_task ; void* execute_task(void*)

co_yield_impl:
;%ifdef RELEASE
    pop rax ; pop return address from the stack
;%else
;    mov rax, qword [rsp] ; save return address ; pop rax
;%endif
    mov qword [rdi], rax
    ; save all callee-preserved gp registers
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
    xor eax, eax
    mov dword [rsi], eax     
    ; restore state
    sub rsp, 0x10
    mov rdi, rsp
    call restore_state 
    pop r8 ; execute_task address
    pop rax ; thread_stack_ptr
    lea rsp, [rax] ; restore thread stack
    ;mov rbp, rax
    lea rax, [r8]
    jmp rax

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
    sub rsp, 0x10
    mov rdi, rsp
    call restore_state
    pop r8 ; execute_task address
    pop rax ; thread_stack_ptr
    pop rdi ; current stack_id -- argumet for set_free_stack
    lea rsp, [rax] ; restore thread stack
    mov rbp, rax
    push r8
    call set_free_stack
    pop r8
    lea rax, [r8] ; skip push rbp
    jmp rax

resume_yielded_task:
    ; save all callee-preserved gp registers
    mov rbx, qword [rdi+0x08]
    mov rbp, qword [rdi+0x10]
    mov rsp, qword [rdi+0x18]
    mov r12, qword [rdi+0x20]
    mov r13, qword [rdi+0x28]
    mov r14, qword [rdi+0x30]
    mov r15, qword [rdi+0x38]
    jmp [rdi]

execute_task:
    ; save original stack ptr in a thread local variable
    mov rdi, rsp
    call set_thread_stack_ptr
    ; align stack ptr
    push rbp
    mov rbp, rsp
    and rsp, STACK_ALIGN
    call execute_task_inner
    pop rbp
    ret 

    ;section .data
    ;section .bss
