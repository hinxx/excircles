#
# Rules.mk
#

LIBEXCIRCLESHOME ?= ../..

-include $(LIBEXCIRCLESHOME)/Config.mk

CIRCLEHOME ?= $(LIBEXCIRCLESHOME)/circle

INCLUDE += -I $(LIBEXCIRCLESHOME)/include

include $(CIRCLEHOME)/Rules.mk

-include $(DEPS)
