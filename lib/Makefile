#
# Makefile
#

LIBEXCIRCLESHOME = ..

OBJS	= ft6206.o ili9341.o tsc2046.o ili9325d.o tscalibration.o ssd1351.o

libexcircles.a: $(OBJS)
	@echo "  AR    $@"
	@rm -f $@
	@$(AR) cr $@ $(OBJS)

include $(LIBEXCIRCLESHOME)/Rules.mk
