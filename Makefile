#
# Makefile
#

-include ../Config.mk

CIRCLEHOME ?= ../circle

CPPFLAGS += -Wno-write-strings

# INCLUDE	+= -I./third/ -I./library/STM32_USB_HOST_Library/Class/MSC/inc -I./library/STM32_USB_HOST_Library/Core/inc -I./library/STM32_USB_OTG_Driver/inc -I./library/fat_fs/inc -I./library/STM32_USB_Device_Library/Core/inc -I./library/STM32_USB_Device_Library/Class/midi/inc -I./library/STM32F4xx_StdPeriph_Driver/inc/ -I./library/CMSIS/Include/ -I./utils/ -I./filesystem -I./hardware -I./usb -I./synth -I./midi -I./midipal -I./ -I./library/STM32_USB_Device_Library/Class/msc/inc
INCLUDE	+= -I./third/   -I./utils/ -I./filesystem -I./hardware -I./usb -I./synth -I./midi -I./midipal -I./

OBJS	= main.o kernel.o serial_irq.o screenx.o serialmidi.o soundmanager.o \
	  global_obj.o utils/RingBuffer.o \
	usb/usbKey_usr.o \
	utils/Hexter.o \
	hardware/LiquidCrystal.o \
	hardware/Menu.o \
	hardware/FMDisplay.o \
	hardware/Encoders.o \
	filesystem/ComboBank.o \
	filesystem/ConfigurationFile.o \
	filesystem/DX7SysexFile.o \
	filesystem/PatchBank.o \
	filesystem/ScalaFile.o \
	filesystem/Storage.o \
	filesystem/FileSystemUtils.o \
	filesystem/PreenFMFileType.o \
	filesystem/UserWaveform.o \
	midi/MidiDecoder.o \
	synth/Env.o \
	synth/Lfo.o \
	synth/LfoEnv.o \
	synth/LfoEnv2.o \
	synth/LfoOsc.o \
	synth/LfoStepSeq.o \
	synth/Matrix.o \
	synth/Osc.o \
	synth/Presets.o \
	synth/Synth.o \
	synth/SynthMenuListener.o \
	synth/SynthParamListener.o \
	synth/SynthStateAware.o \
	synth/SynthState.o \
	synth/Voice.o \
	synth/Timbre.o \
	synth/Common.o \
	midipal/note_stack.o \
	midipal/event_scheduler.o

LIBS	= $(CIRCLEHOME)/addon/ugui/libugui.a \
	  $(CIRCLEHOME)/addon/littlevgl/liblvgl.a \
	  $(CIRCLEHOME)/addon/littlevgl/liblv_examples.a \
	  $(CIRCLEHOME)/addon/Properties/libproperties.a \
	  $(CIRCLEHOME)/addon/SDCard/libsdcard.a \
	  $(CIRCLEHOME)/addon/fatfs/libfatfs.a \
	  $(CIRCLEHOME)/lib/usb/libusb.a \
	  $(CIRCLEHOME)/lib/input/libinput.a \
	  $(CIRCLEHOME)/lib/fs/fat/libfatfs.a \
	  $(CIRCLEHOME)/lib/fs/libfs.a 
#	  $(CIRCLEHOME)/lib/libcircle.a

EXTRALIBS += $(CIRCLEHOME)/lib/libcircle.a

include $(CIRCLEHOME)/app/Rules.mk
