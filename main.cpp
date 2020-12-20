#include <stdint.h>
#include <stdio.h>

int main()
{
    bool is_8086_88 = false;
    bool is_cpuid_supported = false;
    int32_t cpuid_max = 0;
    char cpuid_string[13] = { 0 };
    int32_t cpuid_additional_info = 0;
    int32_t cpuid_signature = 0;
    int32_t cpuid_ecx_flags = 0;
    int32_t cpuid_edx_flags = 0;

    __asm
    {
        ; 1. Используя регистр флагов, необходимо убедиться в наличии 32-разрядного процессора в системе.
        pushf ; сохранить копию регистра флагов
        pop bx ; восстановить в BX
        mov ax, 0x0FFF ; очистить биты 12-15 в AX
        and ax, bx
        push ax ; сохранить новое значение регистра флагов
        popf
        pushf ; установить регистр флагов
        pop ax ; сохранить копию нового регистра флагов в AX

        mov cx, 0xF000
        and ax, cx
        cmp ax, cx ; если биты 12-15 установлены, это процессор 8086/88
        jne non_8086 ; переход, если не 8086/88

        cputype_8086:
        mov [is_8086_88], 1 ; это 8086/88 процессор
        jmp end

        non_8086:
        mov [is_8086_88], 0 ; это не 8086/88 процессор

        ; 2. Убедиться в поддержке команды CPUID (посредством бита ID регистра EFLAGS) и определить максимальное значение параметра ее вызова.
        pushfd
        pop eax ; загрузить регистр флагов в eax
        mov ebx, eax ; сохраняем копию текущих значений регистра флагов в ebx
        xor eax, 0x200000 ; меняем 21-й бит регистра флагов
        push eax
        popfd ; записываем измененный регистр флагов
        pushfd
        pop eax ; еще раз загружаем регистр флагов в eax
        cmp eax, ebx ; проверяем сохранились ли наши изменения в 21-м бите
        jne cpuid_supported_label ; если сохранились, то данный процессор поддерживает CPUID

        cpuid_not_supported_label:
        mov [is_cpuid_supported], 0 ; процессор НЕ поддерживает CPUID
        jmp end

        cpuid_supported_label:
        mov [is_cpuid_supported], 1 ; процессор поддерживает CPUID
        mov eax, 0
        cpuid
        mov [cpuid_max], eax ; определяем и сохраняем в память максимальное значение входного параметра CPUID

        ; 3. Получить строку идентификации производителя процессора и сохранить ее в памяти.
        ; строка идентификация была загружена на предыдущем шаге, поэтому сразу сохраняем ее в память
        mov dword ptr [cpuid_string], ebx
        mov dword ptr [cpuid_string + 4], edx
        mov dword ptr [cpuid_string + 8], ecx

        ; 4. Получить сигнатуру процессора и определить его модель, семейство и т.п. Выполнить анализ дополнительной информации о процессоре.
        mov eax, 1
        cpuid ; вызываем CPUID с аргументом 1
        mov [cpuid_signature], eax ; записываем сигнатуру процессора в память
        mov [cpuid_additional_info], ebx ; записываем дополнительную информацию о процессоре

        ; 5. Получить флаги свойств. Составить список поддерживаемых процессором свойств.
        ; используя результаты вызова CPUID на прошлом шаге, сохраняем значения флагов в память
        mov [cpuid_ecx_flags], ecx
        mov [cpuid_edx_flags], edx

        end:
    }

    printf("8086/88: %s\n", is_8086_88 ? "yes" : "no");
    printf("Is CPUID supported: %s\n", is_cpuid_supported ? "yes" : "no");
    printf("Maximum supported number of CPUID parameters: %d\n", cpuid_max);
    printf("CPUID string: %s\n", cpuid_string);
    printf("CPUID signature: %08X\n", cpuid_signature);
    printf("CPUID additional information: %08X\n", cpuid_additional_info);
    printf("CPUID ECX flags: %08X, EDX flags: %08X\n", cpuid_ecx_flags, cpuid_edx_flags);

    printf("\n");
}


