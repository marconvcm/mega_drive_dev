#include <genesis.h>

// Game constants
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 224
#define PADDLE_WIDTH 8
#define PADDLE_HEIGHT 32
#define BALL_SIZE 8
#define PADDLE_SPEED 3
#define BALL_SPEED 2

// Colors for our 16-color palette
#define COLOR_BLACK 0
#define COLOR_WHITE 15

// Game states
typedef enum
{
   GAME_STATE_TITLE,
   GAME_STATE_PLAYING
} GameState;

// Game variables
GameState current_state = GAME_STATE_TITLE;
typedef struct
{
   s16 x, y;
   s16 vel_x, vel_y;
} Ball;

typedef struct
{
   s16 x, y;
} Paddle;

Ball ball;
Paddle player1, player2;
u16 score_p1 = 0, score_p2 = 0;
char score_text[20];

// Tile data for solid rectangles
static const u32 solid_tile[8] = {
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF};

static const u32 empty_tile[8] = {
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000};

// Function prototypes
void init_game();
void update_paddles();
void update_ball();
void draw_game();
void reset_ball();
void draw_filled_rect(s16 x, s16 y, s16 width, s16 height, u16 color);
void draw_paddle(s16 x, s16 y, u16 color);
void draw_ball_rect(s16 x, s16 y, u16 color);
void draw_center_line();
void show_title_screen();
void update_title_screen();
void clear_screen();

void init_game()
{
   // Initialize paddles
   player1.x = 16;
   player1.y = SCREEN_HEIGHT / 2 - PADDLE_HEIGHT / 2;

   player2.x = SCREEN_WIDTH - 24;
   player2.y = SCREEN_HEIGHT / 2 - PADDLE_HEIGHT / 2;

   // Reset scores
   score_p1 = 0;
   score_p2 = 0;

   // Initialize ball
   reset_ball();
}

void reset_ball()
{
   ball.x = SCREEN_WIDTH / 2 - BALL_SIZE / 2;
   ball.y = SCREEN_HEIGHT / 2 - BALL_SIZE / 2;
   ball.vel_x = (score_p1 + score_p2) % 2 == 0 ? BALL_SPEED : -BALL_SPEED;
   ball.vel_y = BALL_SPEED;
}

void draw_filled_rect(s16 x, s16 y, s16 width, s16 height, u16 color)
{
   // Convert pixel coordinates to tile coordinates
   s16 tile_x = x / 8;
   s16 tile_y = y / 8;
   s16 tile_width = (width + 7) / 8;   // Round up
   s16 tile_height = (height + 7) / 8; // Round up

   u16 tile_index = (color == COLOR_WHITE) ? 1 : 2;

   // Draw the rectangle using tiles
   for (s16 ty = 0; ty < tile_height; ty++)
   {
      for (s16 tx = 0; tx < tile_width; tx++)
      {
         if (tile_x + tx < 40 && tile_y + ty < 28)
         {
            VDP_setTileMapXY(BG_B, TILE_ATTR_FULL(PAL0, 0, 0, 0, tile_index),
                             tile_x + tx, tile_y + ty);
         }
      }
   }
}

void draw_paddle(s16 x, s16 y, u16 color)
{
   draw_filled_rect(x, y, PADDLE_WIDTH, PADDLE_HEIGHT, color);
}

void draw_ball_rect(s16 x, s16 y, u16 color)
{
   draw_filled_rect(x, y, BALL_SIZE, BALL_SIZE, color);
}

void draw_center_line()
{
   // Draw dashed center line
   for (s16 y = 0; y < SCREEN_HEIGHT; y += 16)
   {
      draw_filled_rect(SCREEN_WIDTH / 2 - 1, y, 2, 8, COLOR_WHITE);
   }
}

void update_paddles()
{
   u16 joy1 = JOY_readJoypad(JOY_1);
   u16 joy2 = JOY_readJoypad(JOY_2);

   // Check for SELECT button to return to title screen
   if (joy1 & BUTTON_C)
   {
      current_state = GAME_STATE_TITLE;
      show_title_screen();
      return;
   }

   // Clear old paddle positions
   draw_paddle(player1.x, player1.y, COLOR_BLACK);
   draw_paddle(player2.x, player2.y, COLOR_BLACK);

   // Player 1 controls (Controller 1)
   if (joy1 & BUTTON_UP && player1.y > 0)
   {
      player1.y -= PADDLE_SPEED;
   }
   if (joy1 & BUTTON_DOWN && player1.y < SCREEN_HEIGHT - PADDLE_HEIGHT)
   {
      player1.y += PADDLE_SPEED;
   }

   // Player 2 controls (Controller 2, or A/B buttons for single controller)
   if (joy2 & BUTTON_UP || (joy1 & BUTTON_A))
   {
      if (player2.y > 0)
         player2.y -= PADDLE_SPEED;
   }
   if (joy2 & BUTTON_DOWN || (joy1 & BUTTON_B))
   {
      if (player2.y < SCREEN_HEIGHT - PADDLE_HEIGHT)
         player2.y += PADDLE_SPEED;
   }

   // Simple AI for player 2 if no second controller input
   if (!(joy2 & (BUTTON_UP | BUTTON_DOWN)) && !(joy1 & (BUTTON_A | BUTTON_B)))
   {
      if (ball.y < player2.y + PADDLE_HEIGHT / 2 && player2.y > 0)
      {
         player2.y -= PADDLE_SPEED - 1; // Slightly slower than player
      }
      else if (ball.y > player2.y + PADDLE_HEIGHT / 2 && player2.y < SCREEN_HEIGHT - PADDLE_HEIGHT)
      {
         player2.y += PADDLE_SPEED - 1;
      }
   }
}

void update_ball()
{
   // Clear old ball position
   draw_ball_rect(ball.x, ball.y, COLOR_BLACK);

   const s16 SCREEN_HEIGHT_LIMIT = SCREEN_HEIGHT - BALL_SIZE - 3;
   const s16 SCREEN_TOP_LIMIT = 3;

   // Update ball position
   ball.x += ball.vel_x;
   ball.y += ball.vel_y;

   // Ball collision with top and bottom walls
   if (ball.y <= SCREEN_TOP_LIMIT || ball.y >= SCREEN_HEIGHT_LIMIT)
   {
      ball.vel_y = -ball.vel_y;
      // Keep ball in bounds
      if (ball.y < SCREEN_TOP_LIMIT)
         ball.y = SCREEN_TOP_LIMIT;
      if (ball.y > SCREEN_HEIGHT_LIMIT)
         ball.y = SCREEN_HEIGHT_LIMIT;
   }

   // Ball collision with paddles
   // Player 1 paddle collision
   if (ball.x <= player1.x + PADDLE_WIDTH &&
       ball.x + BALL_SIZE >= player1.x &&
       ball.y + BALL_SIZE >= player1.y &&
       ball.y <= player1.y + PADDLE_HEIGHT)
   {
      ball.vel_x = -ball.vel_x;
      ball.x = player1.x + PADDLE_WIDTH; // Prevent ball from getting stuck
   }

   // Player 2 paddle collision
   if (ball.x + BALL_SIZE >= player2.x &&
       ball.x <= player2.x + PADDLE_WIDTH &&
       ball.y + BALL_SIZE >= player2.y &&
       ball.y <= player2.y + PADDLE_HEIGHT)
   {
      ball.vel_x = -ball.vel_x;
      ball.x = player2.x - BALL_SIZE; // Prevent ball from getting stuck
   }

   // Ball goes off screen (scoring)
   if (ball.x < -BALL_SIZE)
   {
      score_p2++;
      reset_ball();
   }
   else if (ball.x > SCREEN_WIDTH)
   {
      score_p1++;
      reset_ball();
   }
}

void draw_game()
{
   // Draw center line
   draw_center_line();

   // Draw paddles
   draw_paddle(player1.x, player1.y, COLOR_WHITE);
   draw_paddle(player2.x, player2.y, COLOR_WHITE);

   // Draw ball
   draw_ball_rect(ball.x, ball.y, COLOR_WHITE);

   // Draw scores using text overlay on plane A
   sprintf(score_text, "P1:%d", score_p1);
   VDP_drawText(score_text, 1, 2);

   sprintf(score_text, "P2:%d", score_p2);
   VDP_drawText(score_text, 34, 2);

   // Draw controls info
   VDP_drawText("UP/DOWN    -    A/B", 8, 26);
   VDP_drawText("C: Title Screen", 11, 27);

   VDP_drawText("HELLO PONG", 1, 1);
}

void clear_screen()
{
   // Clear the entire screen by filling it with empty tiles
   for (s16 y = 0; y < 28; y++)
   {
      for (s16 x = 0; x < 40; x++)
      {
         VDP_setTileMapXY(BG_B, TILE_ATTR_FULL(PAL0, 0, 0, 0, 2), x, y);
      }
   }

   // Clear text plane A as well
   VDP_clearPlane(BG_A, TRUE);
}

void show_title_screen()
{
   clear_screen();

   // Draw title
   VDP_drawText("      MEGA DRIVE PONG      ", 7, 8);
   VDP_drawText("      ==============      ", 7, 9);

   // Draw instructions
   VDP_drawText("    Press START to Play    ", 7, 14);
   VDP_drawText("                           ", 7, 15);
   VDP_drawText("  Player 1: UP/DOWN        ", 7, 17);
   VDP_drawText("  Player 2: A/B or 2P pad  ", 7, 18);

   // Draw some decorative elements
   VDP_drawText("* * * * * * * * * * * * * *", 3, 5);
   VDP_drawText("* * * * * * * * * * * * * *", 3, 22);

   // Credits
   VDP_drawText("     A Retro Game Demo     ", 7, 25);
}

void update_title_screen()
{
   u16 joy1 = JOY_readJoypad(JOY_1);
   u16 joy2 = JOY_readJoypad(JOY_2);

   // Check for START button on either controller
   if ((joy1 & BUTTON_START) || (joy2 & BUTTON_START))
   {
      current_state = GAME_STATE_PLAYING;
      clear_screen();
      init_game();
   }
}

int main(bool hard_reset)
{
   VDP_init();

   // Set up video mode and basic graphics
   VDP_setScreenWidth320();

   // Set up color palette - simple black and white
   VDP_setPaletteColor(0, RGB24_TO_VDPCOLOR(0x000000));  // Black
   VDP_setPaletteColor(15, RGB24_TO_VDPCOLOR(0xFFFFFF)); // White

   // Load our tile graphics into VRAM
   VDP_loadTileData(solid_tile, 1, 1, 0); // Tile index 1 = solid white
   VDP_loadTileData(empty_tile, 2, 1, 0); // Tile index 2 = empty/black

   // Set background color to black
   VDP_setBackgroundColor(COLOR_BLACK);

   // Start with title screen
   current_state = GAME_STATE_TITLE;
   show_title_screen();

   while (TRUE)
   {
      if (current_state == GAME_STATE_TITLE)
      {
         update_title_screen();
      }
      else if (current_state == GAME_STATE_PLAYING)
      {
         // Update game logic
         update_paddles();
         update_ball();

         // Draw everything
         draw_game();
      }

      // Wait for VBlank
      SYS_doVBlankProcess();
   }

   return 0;
}
