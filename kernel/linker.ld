ENTRY(_start)
SECTIONS
{
    . = 0x1000000;
    .text : {
        *(.text.entry)  /* Спочатку наша точка входу */
        *(.text)
    }
    /DISCARD/ : {
        *(.note*)       /* Викидаємо ті самі 04 00 00 */
        *(.comment)
    }
}
