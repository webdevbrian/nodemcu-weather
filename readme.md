# WeatherNodeMCU
 
![NodeMCU Weather station](https://imgur.com/dnuezaX.jpg)

Every year for christmas I try to make a zany project for my neice Riley. She's 6 this year.

This year it was a "Weather 'Tellinator' 3000" - which uses two SG90 servos (this is my first arduino servo project) to rotate a few different arms, some WS8211b LEDs and [openweathermap.com's](https://openweathermap.org/) free weather API to grab weather data. What I wanted to accomplish is a device where she can wake up in the morning and visually see what the weather would be like outside and for the day. It doubles as a night light when the moon rotates after the sun sets.

It uses the sunrise and sunset timestamp from OWM's api to rotate the "sun and moon" servo between a moon (which has four WS8211b LEDs in it's housing, that shows the current phase of the moon) and a sun that has a single RGB NeoPixel which illuminates it yellow.

It has a "cloud servo" that rotates up and down if the weather is anything less than clear skies. It has a single WS8211b in it as well, which illuminates different colors for cloud conditions (thunderstorm = yellow, rain = blue and so on).

Finally it has a "temperature tower" that labels how cold it is, also having another set of WS8211bs that I soldered in it's own little strip. Each temperature label has it's own color, and uses Adafruit's NeoPixel library.

Everything is powered by the 5V usb connection, no need for VIN power.

The enclosure and arms I designed in 3D CAD are all 3D Printed and can be found here for download: https://www.thingiverse.com/thing:4070408

Feel free to fork an add pull requests on bugs / improvements so I can check out and review!
