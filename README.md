# ESP8266 WiFi Beacon Spammer

This project is a simple demonstration of WiFi beacon frames using an ESP8266 microcontroller. The program will broadcast multiple WiFi beacon frames, cycling through predefined SSIDs and using random MAC addresses.

## Prerequisites

- An ESP8266 microcontroller
- Arduino IDE or any other preferred IDE supporting ESP8266 development
- ESP8266 board version 2.0.0

## Setup

1. Clone this repository to your local machine.
2. Open the project using your favorite IDE (Arduino IDE is recommended).
3. In the board manager, select the ESP8266 board version 2.0.0.
4. Navigate to your ESP8266 package location, typically found in `~/.arduino15/packages/esp8266/hardware/esp8266/2.0.0/tools/sdk/include/`.
5. Open the file `user_interface.h` with a text editor.
6. Scroll down and before `#endif`, add the following lines to enable `wifi_send_pkt_freedom()`:

```cpp
typedef void (*freedom_outside_cb_t)(uint8 status);
int wifi_register_send_pkt_freedom_cb(freedom_outside_cb_t cb);
void wifi_unregister_send_pkt_freedom_cb(void);
int wifi_send_pkt_freedom(uint8 *buf, int len, bool sys_seq);
```

7. Save and close the file.
8. Connect your ESP8266 board to your computer.
9. Compile and upload the program to your ESP8266 board.

## Usage
Once the program is uploaded to the ESP8266, it will start broadcasting WiFi beacon frames. The SSIDs used for broadcasting are defined in the SSIDS array in the program. The program cycles through the SSIDs, broadcasting each one multiple times, then moves on to the next one. This cycle is repeated indefinitely.

Please use this program responsibly. Broadcasting false SSIDs can disrupt WiFi communications and may be illegal in your jurisdiction. This program is provided for educational and research purposes only. The author is not responsible for any misuse of this program.

