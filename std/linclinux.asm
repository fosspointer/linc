%define SYS_READ 0
%define SYS_WRITE 1
%define SYS_OPEN 2
%define SYS_CLOSE 3
%define SYS_MMAP 9
%define SYS_MUNMAP 11
%define SYS_EXIT 60
%define STR_BUFFER_SIZE 1024

%define STDOUT 1
%define STDIN 0

section .bss
    __std_string_buffer: resb STR_BUFFER_SIZE

section .text
    extern __memory_copy:function
    global __memory_alloc:function
    global __memory_free:function
    
    global putc:function
    global puts:function
    global readc:function
    global readraw:function

    global sys_read:function
    global sys_write:function
    global sys_open:function
    global sys_close:function
    global sys_exit:function

__memory_alloc:
    add rdi, 8
    mov rsi, rdi
    xor rdi, rdi
    mov rdx, 3
    mov r10, 0x22
    mov r8, -1
    mov r9, 0
    mov rax, SYS_MMAP
    syscall

    mov [rax], rsi
    add rax, 8
    ret

__memory_free:
    sub rdi, 8
    mov rsi, [rdi]
    mov rax, SYS_MUNMAP
    syscall
    ret

; putc(char): void
putc:
    push rdi
    mov rax, SYS_WRITE
    mov rdi, STDOUT
    lea rsi, [rsp] 
    mov rdx, 1
    syscall
    pop rdi
    ret

; puts(string): void
puts:
    xor rdx, rdx
.loop:
    inc rdx
    mov cl, [rdi + rdx]
    cmp cl, 0
    jnz .loop
    mov rsi, rdi
    mov rax, SYS_WRITE
    mov rdi, STDOUT
    syscall
    ret

; readc(): char
readc:
    mov rdx, 1
    lea rsi, [rel __std_string_buffer]
    mov rax, SYS_READ
    mov rdi, STDIN
    mov rax, [rel __std_string_buffer]
    syscall
    ret

; readraw(): string
readraw:
    push r11
    mov rdx, STR_BUFFER_SIZE
    lea rsi, [rel __std_string_buffer]
    mov rax, SYS_READ
    mov rdi, STDIN
    syscall
    lea rax, [rel __std_string_buffer]
    lea rsi, [rel __std_string_buffer]
.find_newline:
    cmp byte [rsi], 0xa
    je .found
    inc rsi
    jmp .find_newline
.found:
    mov byte [rsi], 0
    sub rsi, rax
    mov rdi, rsi
    mov r11, rsi
    call __memory_alloc
    mov rdx, r11
    mov rdi, rax
    lea rsi, [rel __std_string_buffer]
    call qword [__memory_copy wrt ..got]
    pop r11
    ret

; sys_read(u32, string, u64): void
sys_read:
    mov rax, SYS_READ
    syscall
    ret

; sys_write(u32, string, u64): void
sys_write:
    mov rax, SYS_WRITE
    syscall
    ret

; sys_open(string, i32, i32): u32
sys_open:
    mov rax, SYS_OPEN
    syscall
    ret

; sys_close(u32): void
sys_close:
    mov rax, SYS_CLOSE
    syscall
    ret

; exit(i32): void
sys_exit:
    mov rax, SYS_EXIT
    syscall
