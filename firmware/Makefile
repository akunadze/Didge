include makefile.conf

NAME=Didge

STARTUP_DEFS=

STARTUP=startup/startup_stm32f103.S

LDSCRIPTS=-L. -Lldscripts -T gcc.ld
LFLAGS=$(USE_NANO) $(USE_NOHOST) $(LDSCRIPTS) $(GC) $(MAP)

HPPFILES=$(wildcard *.hpp)
CPPFILES=retarget.cpp devices.cpp boot.cpp main.cpp control.cpp

# Update paths below to reflect your file system
INCLUDES=-Id:/els/DidgeLib/Kvasir/Lib -ID:/ELS/DidgeLib/boost

# Following Boost library definitions are not required if you use my
# modified rational implementation.
BOOST_FLAGS=-DBOOST_NO_EXCEPTIONS -DBOOST_EXCEPTION_DISABLE -DBOOST_NO_IOSTREAM

CXXFLAGS+=$(INCLUDES) -std=c++17 -fno-builtin -ggdb $(BOOST_FLAGS)

# Currently everything is compiled in one fell swoop
$(NAME).elf: $(STARTUP) $(CFILES) $(CPPFILES) $(HPPFILES)
	$(CXX) $(STARTUP) $(CFILES) $(CPPFILES) $(CXXFLAGS) $(LFLAGS) -o $@

bin: $(NAME).elf
	$(OBJCP) -O binary $(NAME).elf $(NAME).bin
	
clean: 
	del $(NAME).elf *.map
