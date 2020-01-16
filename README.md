# RasPreenFM2

![image](https://github.com/styro2000/RasPreenFM2/blob/master/RasPreenFM2_test.jpg)

BareMetal RasperryPi Port of the PreenFM2 Synthesizer https://github.com/Ixox/preenfm2
included extrafilters from https://github.com/pvig/preenfm2

The PreenFM2-Synth looked really good but i was not happy with the STM32-uController...
I really like the STM32 uControllers and did some Projects with them but i was constantly annoyed with the buggy and rather strange Software-Support of STM (HAL-Layer rather kludgy, CubeMX-Code-Generator produces buggy code, poor support etc, its a pitty because i really liked the STM32F-Disco Kits, but an SPI BUSY Errata that costed me to much time & hair....).
Then i stumbled over the Circle-Framework for BareMetal Programming of the Raspberry Pi https://github.com/rsta2/minisynth
and ported PreenFM2 with it to Raspi 3B, HiFiBerry DAC+ Audiocard and a MAX6957 for the connection to the encoders and buttons.
(see hardware/Encoders.cpp for details) and got a latency of about < 3ms @ 48kHz

The Port is a really ugly hack, was does not work:
- Bootmode
- usb-midi doesnt work (now...), MIDI is done thru the serial-interface UART0 (GPIO 14/TX Pin8, GPIO 15/RX Pin10 )
- Oled-Timer
- Clock-Led
- probably more :-(

About the code, i wanted to get this quick up and running without changing to much, so i made some evil things...

Its compiled as 64Bit, just put the kernel8.img, bootcode.bin, cmdline.txt, start.elf & fixup.dat in the root of the sd-card
of your Raspi 3B(+) and off you go!

The best way to build it is to get https://github.com/rsta2/minisynth

set in circle/Rules.mk

```
AARCH	 ?= 64
RASPPI	 ?= 3
PREFIX	 ?= doesent_care-
PREFIX64 ?= /home/where/your/crosscompiler/lives/gcc-arm-8.3-2019.03-x86_64-aarch64-elf/bin/aarch64-elf-
```

Change the Sounddevice in src/config.h

`#define USE_I2S`

and build it to make & test the framework (it won't sound (HiFiBerry is not supported), just to test if its compiling
with the library)
the addon/fatfs must also be build.

To Compile the RasPreenFM2

in the minisynth directory do 

`git clone https://github.com/styro2000/RasPreenFM2.git RasPreenFM2`  to get the Source

i had to change line 63 in circle/Rules.mk to

`ARCH    ?= -DAARCH=64 -march=armv8-a+fp+simd -mtune=cortex-a53 -mlittle-endian  -mcmodel=small`

and the flags (line 121) to

`CFLAGS    += $(ARCH) -Wall -fsigned-char -ffreestanding -mstrict-align $(DEFINE) $(INCLUDE) $(OPTIMIZE) -g`

i added at line 156 at the `clean:` section
`	find . -type f -name '*.o' -delete`
to clean up all subdirs

cd in the RasPreenFM2 directory and do 
make

What i didn't find out was a compatible assembler instruction for

```
#define __USAT(ARG1,ARG2) \
({                          \
  uint32_t __RES, __ARG1 = (ARG1); \
  __ASM ("usat %0, %1, %2" : "=r" (__RES) :  "I" (ARG2), "r" (__ARG1) ); \
  __RES; \
 })
 ```
 
 
but it worked without :-) (but if somebody could give me a hint it would be great!)

The soundbuffer-calculating async in the main loop didn't work to well, gave glitches while screen
writes, so i put the calculating in the DMA-Irq of the sounddevice.
A strange thing happened then, it crashed in prepareMatrixForNewBlock in the LFO Section
while loading a new patch (during the usb-file operations). A long and fruitless search
why? didnt help so i did a really ugly hack and disabled the prepareMatrixForNewBlock()
while loading patches.

Other things i changed:
- Changed BLOCK_SIZE to 16 for smaller latency, gave me first horrible aliasing in the higher notes, changing 
  Osc.h Line 128
    for (int k=0; k<32; ) 
    to
    for (int k=0; k<BLOCK_SIZE; ) {
  seemed to help

  the Circle-framework has to be changed at lib/i2ssoundbasedevice.cpp accordingly to 
  https://www.raspberrypi.org/forums/viewtopic.php?f=72&t=90130&sid=977ea3a6593937475847a5a61421587e&start=225#p1568424 

```
  @@ -71,6 +71,9 @@
  #define TXC_A_CH2POS__SHIFT	4
  #define TXC_A_CH2WID__SHIFT	0
  
  +#define DREQ_A_TX__SHIFT	8
  +#define DREQ_A_TX__MASK		(0x7F << 8)
  +
  @@ -263,6 +266,8 @@ boolean CI2SSoundBaseDevice::Start (void)
  
    // enable I2S DMA operation
    PeripheralEntry ();
  +	write32 (ARM_PCM_DREQ_A,   (read32 (ARM_PCM_DREQ_A) & ~DREQ_A_TX__MASK)
  +				 | (0x18 << DREQ_A_TX__SHIFT));
    write32 (ARM_PCM_CS_A, read32 (ARM_PCM_CS_A) | CS_A_DMAEN);
    PeripheralExit ();
```


- the BPM/LFO/Env/ARP/etc internal clock changed to match the smaller BLOCKSIZE

- the Operators start a 0°, didn't like the Click to much at 90° 

- When the same note is struck again it allocates a new voice (if voice > 1) instead cutting off the voice.
  On NoteOff it switches of the longer running voice first.

- The ComboName is memorized on load/save and used as default for saving 

- changed the glide-times to longer

- integrated the extra filters (thanks to Toltekradiation!)

- made the Filter polyphonic (Filter per Voice) instead paraphonic (Filter per Timbre)

- some checks reading the settings values and prevent loading DX7-Patches > 31 over MIDI ProgramChange
  that would crash the Raspreen.

- put Polyphony to 20 and Operators to 120 (takes about 160us to calculate on full load with Filter h3o+ on Voice, ca 45% of the 
  Sound-DMA IRQ Rate of 333uS, with Filter LP it takes about 120uS)

- the length of the StepLFOs can be set from 2-16 steps (value 1 over F on the reset-step)

- new modmatrix sources 
  p1u,p2u,p3u,p4u  -> unipolar performance control (0.0 - 1.0)
  cnst  -> constant (1.0)

- display is flipped 180° (for contrast reason of the used DFROBOT SKU_DFR0550 5" display (colors looks better upside down))

- clicking an encoderbutton in the performance-screen opens a volumes-screen to adjust the volumes of all 4 timbres

- midi ctrl 7 adjusts the volume of the timbre

- an 'X' is shown in the upper left corner when the audio is clipping

- Changes of the PreenFM2 2.11 Firmware are include (Global Tuning, New Filters etc but without CV-In)

- in Menu->Set->OutLvl its possibile to set the Output Level of the Timbres: 
  dynamic (Default as PreenFM2), fixTimbre -6dB (all timbres have -6dB, beware of clipping)
  fixTimbre -12dB or fixTimbre -18dB

- if it steals a voice in release with the same note it doesnt restart the Operators (less clicking when long release)

- the the MIDIClock synced StepLFOs can be restarted with a Note
  BPM Parameter of the StepLFO has new the entrys "RC/4","RC/2","RC","RC*2","RC*4" for RetriggerClock MidiClock synchronization values


known Bugz:
- sometimes it ignores a note, no idea what it could be, will take probably long to figure out, till then
  it goes as "mandatory humanize"-function :-)   

ToDo:

- usb-midi

- sort of bootmode to access the SD-Card or USB-Stick from outside, the RaspberryPi does not have a USB-Device/OTG
  functionality, so probably over ethernet or so, but in the meantime popping them out and putting them in the
  computer to transfer data is not so terrible....    

- nicer GUI

  actually i find the usability very good for the limited amount of Buttons/Encoders and the editor is great,
  and changing this would take quite some time so this is not on the priority-list


Hope that anybody has fun using the synth and many thanks for bug reports!

Many thanks goes out to Xavier Hosxe for the great PreenFM2-Synth and R. Stange for the amazing Circle Framework!

 * THIS IS ALPHA-SOFTWARE! DONT USE IT FOR LIVE PURPOSES! 
 * I AM NOT RESPONSIBLE FOR ANY TRAGEDIES AT GIGS :-)






