CC	= ../bin/lcc -Wa-l -Wl-m -Wl-j

BINS	   = bannerprint.gb
CSOURCES   = $(foreach dir,./,$(notdir $(wildcard $(dir)/*.c)))
ASMSOURCES = $(foreach dir,./,$(notdir $(wildcard $(dir)/*.s)))
OBJS       = $(CSOURCES:%.c=%.o) $(ASMSOURCES:%.s=%.o)

all:	$(BINS)

compile.bat: Makefile
	@echo "REM Automatically generated from Makefile" > compile.bat
	@make -sn | sed y/\\//\\\\/ | grep -v make >> compile.bat

%.o:	%.c
	$(CC) $(LCCFLAGS) -c -o $@ $<

%.o:	%.s
	$(CC) $(LCCFLAGS) -c -o $@ $<

# Link the compiled object files into a .gb ROM file
$(BINS):	$(OBJS)
	$(CC) $(LCCFLAGS) -o $(BINS) $(OBJS)

clean:
	rm -f *.gb *.o *.lst *.map *~ *.rel *.cdb *.ihx *.lnk *.sym *.asm *.noi

