RM = del
CFLAGS = -DAPP_START=0x1000
OBJS = main.rel sysinit.rel usbdev.rel usbdfu.rel
dfu.hex : dfu.ihx clean
	packihx $< > $@
dfu.ihx: $(OBJS)
	sdcc $(OBJS) -o $@
%.rel:%.c
	sdcc -c $^  $(CFLAGS)
clean:
	$(RM) *.lk *.lst *.map *.mem *.rel *.sym *.rst
distclean: clean
	$(RM) *.ihx *.hex *.bin *.asm

