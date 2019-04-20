# Atmospheric pressure analyzing system “Lyolik”
Laura Madiyar, Zhalgas Khassenov, Dias Zhasanbay
### Necessary components and devices:
1. Sunfolder Mega-2560
2. Barometer-BMP280
3. LCD display-2004
4. Breadboard
5. Jumper wires
6. Bluetooth module HC-06
7. 3 x Buttons
### Connecting scheme
![Untitled Sketch 2_bb](https://user-images.githubusercontent.com/37871005/56460480-388d4400-63c5-11e9-8ea5-1767eb8e6c34.jpg)
### Code working principle
In setup we firstly assign variables for time (we need two variables for time to count the time interval), barometer, buttons (up, down, select) and display. After initializing display it displays the name of project. Furthermore if barometer is not connected informs about error. Then menu is printed. It has 4 line and user can navigate with help of buttons. The first is “Pressure graph”. It draws a graph for pressure in 10 different time. Also maximum, minimum and average pressure is given. The same for “Temperature graph”. Data for the graphs is collected to the memory storage of microprocessor after the user-set time interval in array of structs. After the special time interval barometer collects new data and it is added to arrays and first measurements are deleted.  Then they are mapped according to features of display. As  we know print on the bar is determined by the array. So we just change values in each array to draw graph. Same for loading. User can exit those pages by pressing select button. Hence in order to program identify whether to go to menu or to selected page boolean was introduced. “Send report” automatically sends message to phone with information on current weather conditions such as rain probability, pressure, temperature. With a help of serial bluetooth terminal user receives this information. And the last one is settings. In settings user can choose the time interval between measurements. Default time interval is 20 minutes. It is also can be navigated by buttons. In order to calculate probability of rain change of pressure in 1 hour is calculated. It is how the slope is found. Higher slope - higher probability of rain.
