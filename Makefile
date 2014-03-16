RM = del
CFLAGS = -pstack-mode=large
OBJS = main.rel sysinit.rel usbdev.rel usbdfu.rel
usbdev.hex : usbdev.ihx clean
	packihx $< > $@
usbdev.ihx: $(OBJS)
	sdcc $(OBJS) -o $@
%.rel:%.c
	sdcc -c $^  $(CFLAGS)
clean:
	$(RM) *.lk *.lst *.map *.mem *.rel *.sym *.rst
distclean: clean
	$(RM) *.ihx *.hex *.bin

