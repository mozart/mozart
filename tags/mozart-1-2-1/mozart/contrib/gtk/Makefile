# Generated automatically from Makefile.in by configure.

BUILDTOP	= /home/bruni/devel/mozart
SRCTOP		= /home/bruni/devel/mozart
PREFIX		= /usr/local/oz
SRCDIR          = /home/bruni/devel/mozart/contrib/gtk
VPATH           = /home/bruni/devel/mozart/contrib/gtk
PLATFORM	= win32-i486
LIB_DIR		= $(PREFIX)/cache/x-oz/contrib
HOMEURL         = http://www.mozart-oz.org/home-1.3.0

OZC		= /cygdrive/c/Programme/Mozart/bin/ozc
COMPILE		= $(OZC) -c
OZTOOL		= /cygdrive/c/Programme/Mozart/bin/oztool
OZENGINE        = @OZENGINE@
CXX             = c++
CXXFLAGS	= -O
CPPFLAGS	= -I$(SRCTOP)/platform/emulator \
		  -I$(BUILDTOP)/platform/emulator \
		  -I$(PREFIX)/include 
LDFLAGS		= 
LIBS		= 

INSTALL		= /usr/bin/install -c
INSTALL_DIR	= /home/bruni/devel/mozart/mkinstalldirs
INSTALL_LIB	= $(INSTALL) -m 444
INSTALL_DLL	= $(INSTALL) -m 555

#---------------------------------------------------------------------
# Extra Variables
#---------------------------------------------------------------------

GTKFLAGS       = $(shell gtk-config --cflags)
GTKLIBS        = $(shell gtk-config --libs)
GTKCANVASFLAGS = -I$(CANVASBUILDDIR)/include
GTKCANVASLIBS  = -L$(CANVASBUILDDIR)/lib -lgtk-canvas lart_lgpl -lgdk_imlib

CPPEXTRAFLAGS = $(GTKFLAGS)

ifeq ($(PLATFORM), win32-i486)
CXXEXTRAFLAGS = -fnative-struct $(GTKFLAGS) $(GTKCANVASFLAGS)
else
CXXEXTRAFLAGS = $(GTKFLAGS) $(GTKCANVASFLAGS)
endif

NATEXTRAFLAGS = -s $(GTKCANVASLIBS) $(GTKLIBS)

#---------------------------------------------------------------------
# C99 Parser
#---------------------------------------------------------------------

PARSERSRCS = lex.yy.c \
             c.bison.tab.c
PARSEROBJS = $(PARSERSRCS:%.c=%.o)
PARSERCMPS = parser.so-$(PLATFORM)

#---------------------------------------------------------------------
# Oz Binding Generator
#---------------------------------------------------------------------

OZSRCS    = GOZCore.oz \
            Util.oz \
            Prepare.oz \
            Flatten.oz \
            Collect.oz \
            NativeEmitter.oz \
            ToolKit.oz \
            Main.oz \
            OzWrapper.oz
OZOBJS    = $(OZSRCS:%.oz=%.ozf)
OZINSTALL = $(LIB_DIR)/GOZCore.ozf

CSRCS = GOZSignal.c
COBJS = $(COBJS: %.c=%.o)
CCMPS = $(CSRCS:%.c=%.so-$(PLATFORM))

#---------------------------------------------------------------------
# Native Wrapper
#---------------------------------------------------------------------

NATSRCS    = GdkNative.c \
             GtkNative.c \
             GtkCanvasNative.c
NATOBJS    = $(NATSRCS:%.c=%.o)
NATCMPS    = $(NATSRCS:%.c=%.so-$(PLATFORM))
NATINSTALL = $(NATSRCS:%.c=$(LIB_DIR)/%.so-$(PLATFORM))

#---------------------------------------------------------------------
# Oz Class Wrapper
#---------------------------------------------------------------------

WRPSRCS    = GDK.oz \
             GTK.oz \
             GTKCANVAS.oz
WRPOBJS    = $(WRPSRCS:%.oz=%.ozf)
WRPINSTALL = $(WRPSRCS:%.oz=$(LIB_DIR)/%.ozf)

#---------------------------------------------------------------------
# Target Variables
#---------------------------------------------------------------------

TARGETS     = $(PARSERCMPS) $(OZOBJS) $(CCMPS)
GEN_TARGETS = $(NATCMPS) $(WRPOBJS)
LIB_TARGETS = $(OZINSTALL) $(WRPINSTALL) $(NATINSTALL)

#---------------------------------------------------------------------
# Rules
#---------------------------------------------------------------------

$(PARSEROBJS):%.o: %.c
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(COBJS):%.o: %.c
	$(CXX) $(CXXFLAGS) $(CXXEXTRAFLAGS) -c $< -o $@

$(NATOBJS):%.o: %.c
	$(CXX) $(CXXFLAGS) $(CXXEXTRAFLAGS) -c $< -o $@

%.ozf: %.oz
	$(COMPILE) $< -o $@

$(PARSERCMPS):%.so-$(PLATFORM): $(PARSEROBJS)
	$(OZTOOL) ld -o $@ $<

$(NATCMPS):%.so-$(PLATFORM): $(NATOBJS)
	$(OZTOOL) ld -o $@ $< $(NATEXTRAFLAGS)

#---------------------------------------------------------------------
# Building
#---------------------------------------------------------------------

.PHONY: all wrapper install clean veryclean distclean bootstrap depend

all:		$(TARGETS) wrapper $(GEN_TARGETS)
install:	$(LIB_DIR) $(LIB_TARGETS)

#---------------------------------------------------------------------
# Create Binding Files
#---------------------------------------------------------------------

wrapper:
	$(OZENGINE) Main.ozf OzWrapper.ozf

#---------------------------------------------------------------------
# Installing
#---------------------------------------------------------------------

$(LIB_DIR):
	$(INSTALL_DIR) $@

$(LIB_DIR)/%.ozf: %.ozf
	$(INSTALL_LIB) $< $@

$(LIB_DIR)/%.so-$(PLATFORM): %.so-$(PLATFORM)
	$(INSTALL_DLL) $< $@

#---------------------------------------------------------------------
# Cleaning up
#---------------------------------------------------------------------

clean veryclean:
	-$(RM) -f *~ *.ozf *.o *.so-*

distclean: clean
	-$(RM) config.* Makefile

#---------------------------------------------------------------------
# Bootstrap Stuff
#---------------------------------------------------------------------

include $(SRCTOP)/share/Makefile.boot

bootstrap: boot-all

#---------------------------------------------------------------------
# Automatic Makefile update
#---------------------------------------------------------------------

Makefile: Makefile.in config.status
	./config.status

config.status: configure
	./config.status --recheck
