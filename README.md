# Weerstation
We zullen een weerstation realiseren dat bestaat uit een module buiten en een scherm binnen waar alle informatie getoond wordt.
De modulle buiten zal sensoren bevatten voor de temperatuur, luchtvochtigheid, luchtdruk, neerslag en de lichtintensiteit. 
De twee arduino’s van zowel binnen als buiten zullen door het net gevoed worden en communiceren draadloos met elkaar via lora modules.
Om verschillende zaken in te stellen zoals de tijd of eenheden zullen er ook 3 drukknoppen aanwezig zijn op de module binnen.

V1.0: version 1 does not yet have all the requirements of the design documentation(found in docs/design documentation.pdf),
but it has the following features:

  - RTC clock, not yet configurable in settings, also not saved in ROM
  - temp and hum units can be changed in settings via 3 button navigation
  - note: getting to the settings menu can take some time, just keep pressing the middle button.(not yet with an interrupt)
  - minimal error detection
  - LoRa connection
  - On Screen
  - note: LoRa and screen seem to be incompatible
  - no screen on and off with movement detection to consume less power

