/**
 * ESP8266 WiFi Beacon Spammer
 * 
 * This program uses an ESP8266 microcontroller to broadcast multiple WiFi beacon frames. 
 * It demonstrates how WiFi beacon frames work and how they can be manipulated using low-cost hardware.
 * 
 * The program follows these steps:
 * - Initializes the ESP8266 in station mode and enables promiscuous mode.
 * - In a loop, the program does the following:
 *   1. Generates a random WiFi channel.
 *   2. Composes a WiFi beacon frame with a random MAC address and a predefined SSID.
 *   3. Sends the beacon frame multiple times.
 *   4. Cycles to the next predefined SSID.
 * 
 * Please use this program responsibly. Broadcasting false SSIDs can disrupt WiFi communications 
 * and may be illegal in your jurisdiction. This program is provided for educational and research 
 * purposes only. The author is not responsible for any misuse of this program.
 */

#include <ESP8266WiFi.h>
extern "C" {
  #include "user_interface.h"
}

// The number of SSIDs to be used in the beacon frames.
const byte SSIDS_SIZE = 5;
// The minimum Wi-Fi channel number to be used for beacon transmission.
const byte CHANNEL_MIN = 1;
// The maximum Wi-Fi channel number to be used for beacon transmission.
const byte CHANNEL_MAX = 11;
// The maximum size of the beacon frame in bytes.
const byte FRAME_SIZE = 128;
// The size of the frame header in bytes.
const byte FRAME_HEAD_SIZE = 37;
// The size of the frame footer in bytes.
const byte FRAME_FOOT_SIZE = 13;
// The number of times each beacon frame should be transmitted.
const int NUM_FRAME_TRANSMISSIONS = 3;

// WiFi SSIDS.
String SSIDS[SSIDS_SIZE] = {
  "Spaghetti e mandolino",
  "Free WiFi come in",
  "TestWiFi",
  "Ciao, sono purpetta",
  "SPAM*SPAM*SPAM"
};

// Array to hold the frame data
uint8_t frame[FRAME_SIZE];
// The index pointing to the current SSID in the SSIDS array.
byte SSIDS_index = 0;
// An integer value, randomly generated within the range [CHANNEL_MIN, CHANNEL_MAX], 
// representing the Wi-Fi channel to be used.
byte channel;

// Frame head and foot.
// Remember that casting to/from uint8_t to/from char will always work when char is used for 
// storing ASCII characters, since there are no symbol tables with negative indices.
// Frame header and body.
const uint8_t frame_head[FRAME_HEAD_SIZE] = {
    // FRAME HEADER
    // 0-1 (2 byte): Frame control
    // The specific sequence of 0x80, 0x00 (instead of 0x00, 0x80) is due to the little-endian format.
    0x80, 0x00,
    // 2-3 (2): Duration (time in microseconds)
    0x00, 0x00,
    // 4-9 (6): RA (receiver address/destination address), layer-2 broadcast.
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    // 10-15 (6): Source MAC address (to be randomized).
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // 16-21 (6): BSSID.
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    // 22-23 (2): Sequence control (4 bit: fragment number; 12 bit: sequence number)
    0xc0, 0x6c,
    
    // FRAME BODY
    // 24-31 (8): Timestamp.
    0x83, 0x51, 0xf7, 0x8f, 0x0f, 0x00, 0x00, 0x00,
    // 32-33 (2): Beacon interval.
    0x64, 0x00, 
    // 34-35 (2): Capability info.
    0x01, 0x04,

    // 36: 
    0x00
    
    // SSID length and string are set dynamically in the frame_composer function.
    // 37 (1 byte): SSID length.
    // 0x06,
    // 38-43: SSID string (max 32 octect).
    // 0x72, 0x72, 0x72, 0x72, 0x72, 0x72
};

// The footer of the frame, containing supported rates and DS parameter set.
const uint8_t frame_foot[FRAME_FOOT_SIZE] = { 
  // 44-45 (2 bytes): Tag number (1) for supported rates and tag length (8) indicating 8 parameters for supported rates.
  0x01, 0x08,
  // 46-53 (8): Supported rates (1, 2, 5.5, 11, 18, 24, 36, 54); Note: these values are not decimal.
  0x82, 0x84, 0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c,
  // 54 (1): Tag number for DS parameter set (3).
  0x03,
  // 55 (1): Tag length (1).
  0x01,
  // 56: Channel (dynamically and randomly composed).
  0x00
};


void setup() {
  delay(500);
  wifi_set_opmode(STATION_MODE);
  wifi_promiscuous_enable(1);
  Serial.begin(115200);
  Serial.println("Setup completed");
}

// Function to randomize source MAC address.
void randomize_mac_address() {
  // Iterate over the MAC address bytes in the frame
  // (bytes at positions 10 through 15 and 16 through 21).
  for (int i = 10; i <= 15; i++) {
    // Assign a random byte (value from 0 to 255) to each byte of the MAC address.
    frame[i] = frame[i + 6] = random(256);
  }
  
  // Print a message to the console to indicate that the MAC address has been randomized.
  Serial.println("Source MAC address randomized");
}

// Function to set SSID in frame.
int set_ssid_in_frame() {
  // Obtain the current SSID and its length.
  String current_ssid = SSIDS[SSIDS_index];
  int ssid_length = current_ssid.length();

  // Ensure SSID fits within frame.
  if (ssid_length > FRAME_SIZE - FRAME_HEAD_SIZE - FRAME_FOOT_SIZE) {
    Serial.println("Error: SSID too long to fit in frame");
  }

  // Set the SSID length in the frame.
  frame[FRAME_HEAD_SIZE] = (uint8_t) ssid_length;

  // Copy the SSID into the frame.
  memcpy(&frame[FRAME_HEAD_SIZE + 1], current_ssid.c_str(), ssid_length);

  Serial.println("SSID set in frame");

  return ssid_length;
}

// Function to compose frame.
int compose_frame() {
  // Initialize frame.
  memset(frame, 0, FRAME_SIZE);

  // Copy the first part of the frame prototype.
  memcpy(frame, frame_head, FRAME_HEAD_SIZE);

  // Randomize source MAC address.
  randomize_mac_address();

  // Set SSID in frame.
  int ssid_length = set_ssid_in_frame();

  // Assemble the rest of the frame with frame_foot.
  int i = FRAME_HEAD_SIZE + ssid_length;
  
  // Make sure we have enough space left for the footer.
  if (i + FRAME_FOOT_SIZE <= FRAME_SIZE) {
    // Copy everything but the last byte of the footer.   
    memcpy(&frame[i], frame_foot, FRAME_FOOT_SIZE - 1);
    // Set the channel in the last byte of the footer.
    frame[i + FRAME_FOOT_SIZE - 1] = (uint8_t) channel;

    Serial.println("Frame composed");

    // Return frame length.
    return i + FRAME_FOOT_SIZE;
  } else {
    Serial.println("Error: Frame overflow");
    return 0;
  }
}

// Function to send frame.
void send_frame(int frame_length) {
  // Send the frame.
  for (int i = 0; i < NUM_FRAME_TRANSMISSIONS; i++) {
    int result = wifi_send_pkt_freedom(frame, frame_length, 0);
    if (result != 0) {
      Serial.println("Failed to send frame");
      return;
    }
  }
  Serial.println("Frame sent");
}

// Function to generate channel.
void channel_gen() {
  // Generate random channel
  channel = random(CHANNEL_MIN, CHANNEL_MAX);
  wifi_set_channel(channel);
  Serial.println("Channel generated: " + String(channel));
}

// Main loop
void loop() {
  // Generate a new channel and compose a frame.
  channel_gen();
  int frame_length = compose_frame();

  // Send frame.
  send_frame(frame_length);

  // Cycle through SSIDS.
  SSIDS_index = (SSIDS_index + 1) % SSIDS_SIZE;

  // Delay before next frame.
  delay(1000);
  Serial.println("Cycle complete, preparing for the next round...");
}

