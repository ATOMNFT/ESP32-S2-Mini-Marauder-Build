![Header](Images/esppt.png)
<br>

## 🌟 Flash Tool Fully working as of 04/05/24 🌟

<b>Thought I'd make it easy for those wanting to try out "ESP32 Wi-Fi Penetration Tool" by <a href="https://github.com/risinek/esp32-wifi-penetration-tool">"risinek"</a>. This is a universal tool for ESP32 boards  and it allows implementing various Wi-Fi attacks. 
### More info about "ESP32 Wi-Fi Penetration Tool" can be located <a href="https://github.com/risinek/esp32-wifi-penetration-tool">HERE</a>.  
<br>
<br>
I have also designed a case and lid that can be 3d printed and it houses a small lipo battery, a usb/micro usb charger module salvaged from a portable powerpack, a esp32 wroom 32U with external antenna. <br>
It's a simple parts list to put together and the parts do not take to long to print. I will be adding the files for printing soon.</b>

## Pics of build 
![Outside](Images/Outside-1.jpg)![Inside](Images/Inside-1.jpg)
<br>   
<hr>
<br>

## Simple Flash Method & Usage:
1. Use the <a href=https://atomnft.github.io/ESP32-Wi-Fi-Penetration-Tool/flash0.html>ESP32 Wi-Fi Penetration Flasher Tool</a> in google chrome to flash the project onto ESP32 (DevKit or module)
2. Once the flash is complete, unplug the ESP32 for 30 seconds. Plug the usb back in to power up ESP32.
3. Management AP will display in your cell phone or pc/mac list of available wifi networks within 30 seconds of powering on the ESP32.
4. Connect to this AP\
By default: 
*SSID:* `ManagementAP` and *password:* `mgmtadmin`
5. In a browser like brave or chrome on cell phone open `192.168.4.1` and you should see a web client to configure and control tool like this:

![Web client UI](Images/ui-img.png)
  
