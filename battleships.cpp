#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <vector>

#define SCREEN_HEIGHT 1000
#define SCREEN_WIDTH 1000
#define CELL_SIZE 40
#define GRID_SIZE 10
#define GRID_START_X 50  // Offset for the grid's x start
#define GRID_START_Y 100 // Offset for the grid's y start

ALLEGRO_COLOR shipColors[5] = {
    al_map_rgb(255, 0, 0),   // Red for AirCraftCarrier
    al_map_rgb(0, 255, 0),   // Green for BattleShip
    al_map_rgb(0, 0, 255),   // Blue for Submarine
    al_map_rgb(255, 255, 0), // Yellow for Cruiser
    al_map_rgb(255, 165, 0)  // Orange for Destroyer
};

// Structure to store a ship's information
struct Ship
{
    char row;
    int col;
    char direction;
    int length;
};

// Ship definitions
const char *shipArray[5] = {"AirCraftCarrier", "BattleShip", "Submarine", "Cruiser", "Destroyer"};
const int shipLengths[5] = {5, 4, 3, 3, 2};

// Ensure we can initialize Allegro correctly
void must_init(bool test, const char *description)
{
    if (test)
        return;
    printf("Couldn't initialize %s\n", description);
    exit(1);
}

// Convert row letter to index (e.g., 'A' -> 0, 'J' -> 9)
int rowToIndex(char row)
{
    int rowIndex = row - 'A';
    return rowIndex;
}

// Check if a ship can be placed within the board's bounds
bool is_within_bounds(char row, int col, int ship_length, char direction)
{
    int row_idx = rowToIndex(row);
    // Check if starting position is out of bounds
    if (row_idx < 0 || row_idx >= GRID_SIZE || col < 0 || col >= GRID_SIZE)
        return false;

    // Check based on direction
    switch (direction)
    {
    case 'U': // Upward
        return row_idx - (ship_length - 1) >= 0;
    case 'R': // Rightward
        return col + (ship_length - 1) < GRID_SIZE;
    case 'D': // Downward
        return row_idx + (ship_length - 1) < GRID_SIZE;
    case 'L': // Leftward
        return col - (ship_length - 1) >= 0;
    }
    return false; // Return false for invalid direction input
}

// Draw the battle boards
void drawBattleBoard()
{
    // Draw vertical and horizontal lines for both grids
    for (int i = 0; i <= GRID_SIZE; i++)
    {
        // Left board
        al_draw_line(GRID_START_X + i * CELL_SIZE, GRID_START_Y, GRID_START_X + i * CELL_SIZE, GRID_START_Y + GRID_SIZE * CELL_SIZE, al_map_rgb(150, 150, 150), 2.0); // Vertical
        al_draw_line(GRID_START_X, GRID_START_Y + i * CELL_SIZE, GRID_START_X + GRID_SIZE * CELL_SIZE, GRID_START_Y + i * CELL_SIZE, al_map_rgb(150, 150, 150), 2.0); // Horizontal

        // Right board
        al_draw_line(550 + i * CELL_SIZE, 100, 550 + i * CELL_SIZE, 500, al_map_rgb(150, 150, 150), 2.0); // Vertical
        al_draw_line(550, 100 + i * CELL_SIZE, 950, 100 + i * CELL_SIZE, al_map_rgb(150, 150, 150), 2.0); // Horizontal
    }
}

// Draw the labels (A-J for rows, 0-9 for columns) for both grids
void drawLabelForBoard(ALLEGRO_FONT *font)
{
    const char *letters = "ABCDEFGHIJ";

    for (int i = 0, x = GRID_START_X; i < 10; ++i, x += CELL_SIZE)
    {
        al_draw_textf(font, al_map_rgb(255, 255, 255), x + 20, GRID_START_Y - 20, ALLEGRO_ALIGN_CENTER, "%c", letters[i]);
        al_draw_textf(font, al_map_rgb(255, 255, 255), x + 520, GRID_START_Y - 20, ALLEGRO_ALIGN_CENTER, "%c", letters[i]); // Right grid
    }

    for (int j = 0, y = GRID_START_Y; j < 10; ++j, y += CELL_SIZE)
    {
        al_draw_textf(font, al_map_rgb(255, 255, 255), GRID_START_X - 20, y + 20, ALLEGRO_ALIGN_CENTER, "%d", j); // Left grid
        al_draw_textf(font, al_map_rgb(255, 255, 255), 530, y + 20, ALLEGRO_ALIGN_CENTER, "%d", j);               // Right grid
    }
}

// Draw a ship on the board
void drawShipOnBoard(char row, int col, char direction, int ship_length, ALLEGRO_COLOR ship_color)
{
    int row_idx = rowToIndex(row); // Convert row letter to index
    printf("Drawing ship at row: %c, col: %d, direction: %c\n", row, col, direction); // Debug

    // Loop through the length of the ship
    for (int i = 0; i < ship_length; ++i)
    {
        int draw_row = row_idx;  // Row index to draw
        int draw_col = col;      // Column index to draw

        // Move the position based on direction
        if (direction == 'L') // Move ship left (decrease row)
            draw_row = row_idx - i;
        else if (direction == 'D') // Move ship down (increase column)
            draw_col = col + i;
        else if (direction == 'R') // Move ship right (increase row)
            draw_row = row_idx + i;
        else if (direction == 'U') // Move ship up (decrease column)
            draw_col = col - i;

        // Check bounds and debug information
        printf("Part %d of ship: row = %d, col = %d\n", i, draw_row, draw_col);
        if (draw_row < 0 || draw_row >= GRID_SIZE || draw_col < 0 || draw_col >= GRID_SIZE)
        {
            printf("Ship part out of bounds at row %d, col %d\n", draw_row, draw_col);
            continue; // Skip drawing if out of bounds
        }

        // Draw the rectangle for each part of the ship
        al_draw_filled_rectangle(
            GRID_START_X + draw_row * CELL_SIZE,              // X position
            GRID_START_Y + draw_col * CELL_SIZE,              // Y position
            GRID_START_X + (draw_row + 1) * CELL_SIZE,        // X position (right)
            GRID_START_Y + (draw_col + 1) * CELL_SIZE,        // Y position (bottom)
            ship_color);                                      // Ship color
    }
}



// Draw all ships that are currently stored in the ship list
void drawAllShips(const std::vector<Ship> &ships)
{
    for (int i = 0; i < ships.size(); i++)
    {
        // Pass the appropriate color for each ship based on the ship index
        drawShipOnBoard(ships[i].row, ships[i].col, ships[i].direction, ships[i].length, shipColors[i]);
    }
}

// Show the input prompt
void drawInputWindow(ALLEGRO_FONT *font, const char *input, const char *currentShip)
{
    al_draw_filled_rectangle(50, 600, 550, 700, al_map_rgb(0, 0, 0));   // Input box background
    al_draw_rectangle(50, 600, 550, 700, al_map_rgb(255, 255, 255), 2); // Border
    al_draw_textf(font, al_map_rgb(255, 255, 255), 100, 620, ALLEGRO_ALIGN_LEFT, "Enter Position for %s (e.g., A3R):", currentShip);
    al_draw_textf(font, al_map_rgb(255, 255, 255), 100, 660, ALLEGRO_ALIGN_LEFT, "%s", input);
}

int main()
{
    al_init();
    al_init_primitives_addon();
    al_init_font_addon();

    ALLEGRO_DISPLAY *disp = al_create_display(SCREEN_WIDTH, SCREEN_HEIGHT);
    must_init(disp != NULL, "display");

    ALLEGRO_FONT *font = al_create_builtin_font();
    must_init(font != NULL, "font");

    ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
    must_init(queue != NULL, "queue");

    al_install_keyboard();
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_display_event_source(disp));

    bool done = false;
    bool redraw = true;
    char input[4] = "";
    int input_len = 0;
    bool input_complete = false;

    // Track which ship we are currently placing
    int currentShipIndex = 0;

    // Store all placed ships
    std::vector<Ship> ships;

    al_clear_to_color(al_map_rgb(0, 0, 0));
    drawBattleBoard();
    drawLabelForBoard(font);
    al_flip_display();

    while (!done)
    {
        ALLEGRO_EVENT event;
        al_wait_for_event(queue, &event);

        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
            done = true;

        if (event.type == ALLEGRO_EVENT_KEY_CHAR)
        {
            char c = event.keyboard.unichar;

            if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
                done = true;

            if (input_len < 3)
            {
                if (input_len == 0 && c >= 'A' && c <= 'J') // Row
                    input[input_len++] = c;
                else if (input_len == 1 && c >= '0' && c <= '9') // Column
                    input[input_len++] = c;
                else if (input_len == 2 && (c == 'U' || c == 'R' || c == 'D' || c == 'L')) // Direction
                    input[input_len++] = c;
                input[input_len] = '\0';
                redraw = true;
            }

            if (input_len == 3 && (event.keyboard.keycode == ALLEGRO_KEY_ENTER || event.keyboard.keycode == ALLEGRO_KEY_PAD_ENTER))
            {
                // Parse input: example input = "A3R"
                for (int i = 0; i < 4; i++)
                {
                    printf("Character on %d is : %c\n", i, input[i]);
                }
                char row = input[0];
                printf("row variable: %c\n", row);
                int col = input[1] - '0'; // Convert char digit to int
                printf("col variable: %d\n", col);
                char direction = input[2];
                printf("direction variable: %c\n", direction);
                printf("Parsed input: Row = %c, Col = %d, Direction = %c\n", row, col, direction);
                if (is_within_bounds(row, col, shipLengths[currentShipIndex], direction))
                {
                    // Add the ship to the list
                    ships.push_back({row, col, direction, shipLengths[currentShipIndex]});
                    currentShipIndex++; // Move to the next ship

                    if (currentShipIndex >= 5)
                    {
                        // All ships placed, finish the loop
                        done = false;
                    }
                }
                // Reset input
                input_complete = false;
                input_len = 0;
                input[0] = '\0';
            }
        }

        if (redraw && al_is_event_queue_empty(queue))
        {
            al_clear_to_color(al_map_rgb(0, 0, 0));

            drawBattleBoard();       // Draw the battle grid
            drawLabelForBoard(font); // Redraw the labels
            drawAllShips(ships);     // Redraw all ships stored in the list

            if (!input_complete)
            {
                drawInputWindow(font, input, shipArray[currentShipIndex]); // Display current ship
            }

            al_flip_display();
            redraw = false;
        }
    }

    al_destroy_font(font);
    al_destroy_display(disp);
    al_destroy_event_queue(queue);
    return 0;
}
