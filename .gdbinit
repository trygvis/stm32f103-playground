define target hookpost-extended-remote
    monitor arm semihosting enable
    monitor reset halt
    set remotetimeout 10
end

define run_max7219
    make -C build max7219.elf

    monitor reset halt

    set confirm off
    file build/apps/max7219/max7219.elf
    set confirm on

    load
    continue
end

target extended-remote :3333
