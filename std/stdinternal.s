%define SYS_WRITE 1
%define SYS_READ 0
%define SYS_EXIT 60
%define SYS_MMAP 9
%define STR_BUFFER_SIZE 1024

%define STDOUT 1
%define STDIN 0

section .bss
    string_buffer: resb STR_BUFFER_SIZE
section .text
    global readraw
    global readchar
    global putc
    global puts
    global memory_alloc
    global i32_length
    global i32_to_string
    global u64_to_string
    global exit
    global __puts_literal

; readraw(): string
readraw:
    mov rdx, STR_BUFFER_SIZE
    mov rsi, string_buffer
    mov rax, SYS_READ
    mov rdi, STDIN
    syscall
    mov rax, string_buffer
    mov rsi, string_buffer
.find_newline:
    cmp byte [rsi], 0xa
    je .found
    inc rsi
    jmp .find_newline
.found:
    mov byte [rsi], 0
    ret

readchar:
    mov rdx, 1
    mov rsi, string_buffer
    mov rax, SYS_READ
    mov rdi, STDIN
    mov rax, [string_buffer]
    syscall
    ret

; exit(i32): void
exit:
    mov rax, SYS_EXIT
    syscall

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
    push rbp
    mov rbp, rsp
    mov rdx, 0
.loop:
    inc rdx
    mov cl, [rdi + rdx]
    cmp cl, 0
    jnz .loop

    mov rsi, rdi
    mov rax, SYS_WRITE
    mov rdi, STDOUT
    syscall
    leave
    ret

; memory_alloc(u64): u64
memory_alloc:
    mov rcx, rdi
    xor rdi, rdi
    mov rsi, rcx
    mov rdx, 3 ; -rw
    mov r10, 0x22 ; private | anonymous
    mov r8, -1
    mov r9, 0
    mov rax, SYS_MMAP
    syscall
    ret

; i32_to_string(i32): string
i32_to_string:
    mov edx, edi
    push rdx
    mov rdi, 12
    call memory_alloc
    pop rdx
    mov esi, 10
    lea rcx, [rax + 12]
    mov byte [rcx], 0
    dec rcx
    mov eax, edx
    xor bl, bl
    test eax, eax
    jz .zero
    jns .loop
    neg eax
    inc bl
.loop:
    test rax, rax
    jz .exit
    dec rcx
    xor rdx, rdx
    div esi
    add dl, '0'
    mov byte [rcx], dl
    jmp .loop
.exit:
    test bl, bl
    jz .non_negative
    dec rcx
    mov byte [rcx], '-'
.non_negative:
    mov rax, rcx
    ret
.zero:
    mov byte [rcx], '0'
    mov rax, rcx
    ret

; u64_to_string(u64): string
u64_to_string:
    mov rdx, rdi
    mov rdi, 22
    push rdx
    call memory_alloc
    pop rdx
    mov rsi, 10
    lea rcx, [rax + 22]
    mov byte [rcx], 0
    dec rcx
    mov rax, rdx
    xor bl, bl
    test rax, rax
    jnz .loop
    mov byte [rcx], '0'
    mov rax, rcx
    ret
.loop:
    test rax, rax
    jz .exit
    dec rcx
    xor rdx, rdx
    div rsi
    add dl, '0'
    mov byte [rcx], dl
    jmp .loop
.exit:
    mov rax, rcx
    ret

; __puts_literal(string, u64): void
__puts_literal:
    mov rdx, rsi
    mov rsi, rdi
    mov rax, SYS_WRITE
    mov rdi, STDOUT
    syscall
    ret