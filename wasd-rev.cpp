// wasd-rev.cpp

#include "wasd-rev.h"

int main()
{
     // Create screen buffer
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;

    // @FILE I/O
    out_file.open(OUTFILE_NAME, std::ios::out | std::ios::trunc);
    if (!out_file.is_open()) {
        exit(1);
    }
    out_print("START OUTPUT STREAM\n");


    init();

    // @LOOP
    const double delta = 1 / (double) FPS_MAX;
    unsigned frame_count = 0;
    double total_frame_time = 0;

    frame = 0;
    std::string fps_str = "";

    while (1) {

        // #### TIMER BEGIN ####
        using namespace std::chrono;
        high_resolution_clock::time_point t1 = high_resolution_clock::now();
        // #####################


        // @UPDATE
        update_keystates();
        if (update()) break;

        // @DRAW OVERLAY
        write_string(fps_str, SCREEN_WIDTH - fps_str.length(), 0);
        write_string(std::to_string(frame), SCREEN_WIDTH - std::to_string(frame).length(), 1);


        unsigned line_x = FUNNEL_X * 2 + INPUT_C * (INPUT_WIDTH + INPUT_SPACING);
        std::string d_hit_str = "SCORE: " + std::to_string(dropped_hit);
        write_string(d_hit_str, line_x + 15, CEIL + 5);

        std::string d_passed_str = "MISSED: " + std::to_string(dropped_total - dropped_hit);
        write_string(d_passed_str, line_x + 15, CEIL + 7);

        std::string d_total_str = "TOTAL: " + std::to_string(dropped_total);
        write_string(d_total_str, line_x + 15, CEIL + 8);

        //@TITLE
        write_string("<<<< W-A-S-D REVOLUTION!! >>>>", line_x + 4, CEIL + 2);
        write_string("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^", line_x + 4, CEIL + 1);
        write_string("vvvvvvvvvvvvvvvvvvvvvvvvvvvvvv", line_x + 4, CEIL + 3);

        // @DRAW SCREEN
        screen[SCREEN_WIDTH * SCREEN_HEIGHT] = '\0';
        WriteConsoleOutputCharacter(hConsole, screen, SCREEN_WIDTH * SCREEN_HEIGHT, { 0, 0 }, &dwBytesWritten);




        // #### TIMER END ####
        high_resolution_clock::time_point t2 = high_resolution_clock::now();
        duration<double> time_span = duration_cast<duration<double>>(t2 - t1);

        double time_remaining = delta - time_span.count();
        if (time_remaining > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds( (int)(1000*time_remaining) ));
        }

        high_resolution_clock::time_point t3 = high_resolution_clock::now();
        duration<double> time_extra = duration_cast<duration<double>>(t3 - t1);

        total_frame_time += time_extra.count();
        frame_count += 1;

        if (total_frame_time >= 1) {
            fps_str = std::to_string(frame_count);
            out_file << "[" << frame << "]  " << fps_str << "  frametime: " << 
                        (time_span.count() + time_extra.count()) << "\n" << std::flush;
            total_frame_time = 0;
            frame_count = 0;
        }

        frame++;
        // #####################
    }

    out_print("END OUTPUT STREAM\n");
    out_file.close();

    return 0;
}


void init() {
    ents.clear();
    dropped_total = 0;
    dropped_hit = 0;
    hit_frame = 0;
    next_frame = 0;
    on_hit = false;
}


bool update() {

    backdrop();

    // @INPUT TILES
    for (unsigned key = 0; key < INPUT_C; key++) {

        unsigned x = FUNNEL_X + key * (INPUT_WIDTH + INPUT_SPACING);
        unsigned y = SCREEN_HEIGHT - FLOOR - INPUT_HEIGHT;

        for (unsigned i = 1; i < INPUT_WIDTH; i++) {
            screen[(x + i) + y * SCREEN_WIDTH] = '_';
        }
        for (unsigned i = 0; i < INPUT_HEIGHT - 1; i++) {
            screen[x + (y + i + 1) * SCREEN_WIDTH] = '|';
        }
        for (unsigned i = 0; i < INPUT_HEIGHT - 1; i++) {
            screen[x + INPUT_WIDTH + (y + i + 1) * SCREEN_WIDTH] = '|';
        }

        screen[(x + 1) + (y + 1) * SCREEN_WIDTH] = inputs[key];

        // if key pressed
        if (get_key_down_char(inputs[key])) {
            screen[(x + 2) + (y + 1) * SCREEN_WIDTH] = '#';
            screen[(x + 2) + (y + 2) * SCREEN_WIDTH] = '#';
            screen[(x + 1) + (y + 2) * SCREEN_WIDTH] = '#';
            screen[(x + 3) + (y + 1) * SCREEN_WIDTH] = '#';
            screen[(x + 3) + (y + 2) * SCREEN_WIDTH] = '#';
        }
    }


    // @ENTITIES
    if (frame - track_frame > next_frame) {
        int idx = rand() % INPUT_C;
        ents.push_back(entity_t(FUNNEL_X + idx * (INPUT_WIDTH + INPUT_SPACING), CEIL, idx));
        track_frame = frame;
        next_frame = SPAWN_TIME + (rand() % 5 - 2) * 4;
    }

    for (unsigned e = 0; e < ents.size(); e++) {

        ents.at(e).y_pos += ENTITY_SPEED;

        // hit possible
        if (ents.at(0).y_pos > SCREEN_HEIGHT - FLOOR - INPUT_HEIGHT + 1 - INPUT_HIT_WINDOW) {
            std::string target(1, inputs[ents.at(0).id]);
            if (get_key_pressed(target)) {
                dropped_hit++;
                dropped_total++;
                on_hit = true;
                hit_frame = frame;

                ents.erase(ents.begin() + e);
                e--;
                continue;
            }
        }

        // out of range
        if (ents.at(0).y_pos > SCREEN_HEIGHT - FLOOR - INPUT_HEIGHT + 1) {
            dropped_total++;

            ents.erase(ents.begin() + e);
            e--;
            continue;
        }

        screen[INPUT_WIDTH / 2 + (int)floor(ents.at(e).x_pos) +
                (int)floor(ents.at(e).y_pos) * SCREEN_WIDTH] = '@';
    }

    // @CHECK MISS
    // ... dec. score


    // @HIT DISPLAY
    if (frame - hit_frame > HIT_TIME) {
        on_hit = false;
    }
    if (on_hit) {
        /*unsigned yw = SCREEN_WIDTH * (SCREEN_HEIGHT - FLOOR - 1);
        for (int x = FUNNEL_X - 1; x < FUNNEL_X + INPUT_C * (INPUT_WIDTH + INPUT_SPACING); x++) {
            screen[x + yw] = '!';
        }*/
    }

    return false;
}


void backdrop() {
    for (unsigned int i = CEIL*SCREEN_WIDTH; i < SCREEN_SIZE - FLOOR*SCREEN_WIDTH; i++) {
        screen[i] = ' ';
    }
    for (unsigned int i = 0; i < CEIL * SCREEN_WIDTH; i++) {
        screen[i] = '+';
    }
    for (unsigned int i = SCREEN_SIZE - FLOOR * SCREEN_WIDTH; i < SCREEN_SIZE; i++) {
        screen[i] = 'X';
    }

    unsigned line_x = FUNNEL_X * 2 + INPUT_C * (INPUT_WIDTH + INPUT_SPACING);
    for (unsigned int i = CEIL; i < SCREEN_HEIGHT - FLOOR; i++) {
        screen[line_x + i * SCREEN_WIDTH] = '|';
    }
}


void write_string(std::string msg, unsigned int x, unsigned int y) {
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) return;
    const unsigned int offset = x + y * SCREEN_WIDTH;
    for (unsigned i = 0; i < msg.length(); i++) {
        if (offset >= SCREEN_WIDTH * SCREEN_HEIGHT) break; // don't overwrite EOF
        screen[offset + i] = msg.at(i);
    }
}


bool get_key_down(std::string key_action) {
    for (unsigned i = 0; i < keybinds.size(); i++) {
        if (keybinds.at(i).first.compare(key_action) == 0) {
            return keybinds.at(i).second.down;
        }
    }
    return false;
}

bool get_key_pressed(std::string key_action) {
    for (unsigned i = 0; i < keybinds.size(); i++) {
        if (keybinds.at(i).first.compare(key_action) == 0) {
            return keybinds.at(i).second.pressed;
        }
    }
    return false;
}

bool get_key_released(std::string key_action) {
    for (unsigned i = 0; i < keybinds.size(); i++) {
        if (keybinds.at(i).first.compare(key_action) == 0) {
            return keybinds.at(i).second.released;
        }
    }
    return false;
}

bool get_key_down_char(char key_char) {
    for (unsigned i = 0; i < keybinds.size(); i++) {
        if (keybinds.at(i).second.key == key_char) {
            return keybinds.at(i).second.down;
        }
    }
    return false;
}

void update_keystates() {
    for (unsigned i = 0; i < keybinds.size(); i++) {
        keystate_t& state = keybinds.at(i).second;
        if (GetAsyncKeyState((unsigned short)state.key) & 0x8000) {
            if (!state.down) {
                state.pressed = true;
            }
            else {
                state.pressed = false;
            }
            state.down = true;
        }
        else {
            if (state.down) {
                state.released = true;
            }
            else {
                state.released = false;
            }
            state.down = false;
        }
    }
}


void out_print(std::string str) {
    if (!out_file.is_open()) return;
    out_file << str;
}

void out_println(std::string str) {
    if (!out_file.is_open()) return;
    out_file << str << "\n";
}