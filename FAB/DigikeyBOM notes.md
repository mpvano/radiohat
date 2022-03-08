## Digikey RadioHat 1.0 BOM Notes ##
7-Mar-2022

### Disclaimer ###
This dump of my last BOM list on Digikey should be close to what is needed to build a *RadioHat 1.0* Board like the 10 I fabricated in January and Febuary 2022. You are responsible for carefully reviewing each item and locating it, or an equivalent part. I provide this only as an aid, not a definitive list! Due to the current parts shortages, it's very difficult to confirm what parts were actually bought here - they had to change with changing inventories.

Of course, you cannot simply place an order for this list. Many parts will not be available, others will have different minimum quantities. Some parts are simply no longer available and probably never will be from Digikey. It's only meant to be a starting point!

### Notes ###

1.	DigiKey's inventory record for C48 has an incorrect description for that part. It is incorrectly listed as *1.5µf* instead of *1.5mf* (for *millifarad*) or *1500µf*. The part number is correct and the correct part arrived. It's only the inventory record that is wrong. For decades, *mf* meant *microfarad*, but it no longer does.

2. The Top Silk Screen layer of the version 1.0 board has two swapped parts designators. R23 and R24's labels are reversed. The schematic, Fab layer screen and everything else are correct - only the silk screen (and 3D rendering of it) are incorrect.

3.	Some late changes in components to op-amp U4's feedback components resulted in too much gain and narrowing the bandwidth. The BOM still reflects the incorrect parts. I recommend reducing the feedback resistors R11 and R12 from 10k to 2.4k. This will allow an rf input level of about -16dbm before clipping occurs.

	Instead of adjusting the bandwidth to the old goal of approximately 8khz, most people seem to prefer operating this stage with less selectivity to allow spectrum display graphs of the full audio bandwidth (+-19khz or so before the ADC's filtering cuts in). Removing C6 and C7 completely seems to achieve this goal.

	With some very high performance op-amps, it may be desirable to prevent the bandwidth from exceeding this range, and if so values of 200pf can be used for C6 and C7. Some experimentation in this area is probably worthwhile.

	The QSD itself still has some deliberate roll-off due to the high values of capacitors used for C2, C3, C4, and C5. It has been suggested that values as low as .1µfd may be more appropriate, but I have not tried that substitution.

4.	Be cautious about the si5351 (U3). The original part I was using was all I could obtain and the part number shown in the original BOM, while correct, is for a part with custom programming to give it a *different I2C address*. All these parts were removed from boards and no more boards should be built using them so that the same software builds will work on everything! The *Digikey* BOM reflects the standard version of the part (but of course it's not available!) use the MS5351 instead.

5. The original *RadioHat* operated with a crystal frequency of 27Mhz. All current boards have had the crystal replaced with the one shown in the *Digikey* BOM (or another 25Mhz equivalent). No more 27Mhz boards should ever be built!

6. R35 is intended to be a zero ohm jumper. It turns out that's a hard value to obtain! The current parts list shows it as 1 ohm and that works fine (Or you can use a piece of wire).

7. J7, the JST PH-4 connector used for *STEMMA* compatibility is no longer available in the form used to build the first 10 boards. A compatible connector is available from Adafruit Industries and is stocked by many suppliers including DigiKey. That is what is shown in this BOM. It's only available in packages of 10 connectors at a time, however!

8. There are many variant GPIO connectors (J8). I recommend making sure the ones you buy have square pins, or you may have contact problems if you wish to connect the popular *dupont* female connectors with them. For some uses, taller connectors may be needed to clear the heat sinks on some cases (or just to isolate the board from heat).

9.	Due to costs and parts availability, I recommend building boards without the clock calendar chip at this time.

10. It has become very clear to me that boards should not be built with the SMA connectors unless they are going to be used! They are a nuisance in may cases due to their protruberance from the side of the board. Only install them if you plan to use them!

11. The eeprom (U9) is not actually required or used at this time. It is needed, and needs to be programmed if you intend to sell boards and call them "hats". There are other plug-and-play advantages it can provide, but these parts have become very difficult to find at this time and can be added later if needed!

12. There are obviously many replacements available for J4 and J5. Obtaining *brand-name* headers of this type can be very difficult and expensive. I suggest ensuring that the pins are of high quality, square, and long enough to provide low contact resistance - especially for J5 - which is used to carry power.


Mario Vano






