RM = del
OBJS = main.rel sysinit.rel usbdev.rel usbdfu.rel
usbdev.hex : usbdev.ihx clean
	packihx $< > $@
usbdev.ihx: $(OBJS)
	sdcc $(OBJS) -o $@
%.rel:%.c
	sdcc -c $^  --stack-auto
clean:
	$(RM) *.lk *.lst *.map *.mem *.rel *.sym *.rst *.asm 
distclean: dummy
	$(RM) *.ihx *.hex *.bin

