ldr w0 GPSEL0_ADDRESS           // setting pin2 to output; w0 = GPSEL0_ADDRESS
ldr w1 FSEL2                    // w1 = FSEL2
str w1 [w0]                     // *(GPSEL0_ADDRESS) = FSEL2
mov w30 wzr                     // use w30 as bool, w30 = 0

set_pin2:
    ldr w0 GPCLR0_ADDRESS       // clear CLR pin2; w0 = GPCLR0_ADDRESS
    str wzr [w0]                // *(GPCLR0_ADDRESS) = 0
    ldr w0 GPSET0_ADDRESS       // set SET pin2; w0 = GPSET0_ADDRESS
    ldr w1 PIN2                 // w1 = PIN2
    str w1 [w0]                 // *(GPSET0_ADDRESS) = PIN2. continue to wait.
wait:
    mov w0 wzr                  // w0 as a counter; w0 = 0
    ldr w1 NUM_LOOP             // w1 = NUM_LOOP
    loop:
        add w0 w0 #1            // w0++       
        cmp w0 w1               // if (w0 != NUM_LOOP);
        b.ne loop               // jump to loop
    mvn w30 w30                 // w30 = !w30
    cmp w30 wzr                 // if (w30 == 0) // boolean check
    b.eq set_pin2               // jump to set_pin2. else continue to clear_pin2.
clear_pin2:
    ldr w0 GPSET0_ADDRESS       // clear SET pin2; w0 = GPSET0_ADDRESS
    str wzr [w0]                // *(GPSET0_ADDRESS) = 0
    ldr w0 GPCLR0_ADDRESS       // set CLEAR pin2; w0 = GPCLR0_ADDRESS
    ldr w1 PIN2                 // w1 = PIN2
    str w1 [w0]                 // *(GPCLR0_ADDRESS) = PIN2
    b wait                      // jump to wait

GPSEL0_ADDRESS:
    .int 0x3f200000
GPSEL1_ADDRESS:
    .int 0x3f200004
GPSEL2_ADDRESS:
    .int 0x3f200008
GPSEL3_ADDRESS:
    .int 0x3f20000c
GPSEL4_ADDRESS:
    .int 0x3f200010
GPSEL5_ADDRESS:
    .int 0x3f200014

GPSET0_ADDRESS:
    .int 0x3f20001c
GPSET1_ADDRESS:
    .int 0x3f200020

GPCLR0_ADDRESS:
    .int 0x3f200028
GPCLR1_ADDRESS:
    .int 0x3f20002c

FSEL2:
    .int 0x00000040
PIN2:
    .int 0x00000004
NUM_LOOP:
    .int 0x002fffff
