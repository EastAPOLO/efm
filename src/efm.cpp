#include <filesystem>
#include <vector>
#include <ncurses.h>
#include "file.hpp"

constexpr char down = 'j';
constexpr char up = 'k';
constexpr char left = 'h';
constexpr char right = 'l';
constexpr char top = 'g';
constexpr char bottom = 'G';
constexpr int default_pos = 1;

int row, col;

static void start_ncurses(){
    initscr();
    cbreak();
    noecho();
    refresh();
}

static std::vector<file> left_window(WINDOW *left_win, std::filesystem::path &window_path, int cursor_position, int scroll_position){
    int y{default_pos};
    file item;
    std::vector<file> files;

    getmaxyx(stdscr, row, col);
    left_win = newwin(row, col / 2, 0, 0);

    wmove(left_win, 0, 0);
    box(left_win, 0, 0);
    wprintw(left_win, window_path.c_str());

    for(auto& p: std::filesystem::directory_iterator(window_path)){
        item.name = p.path().filename();
        item.is_dir = p.is_directory();
        files.push_back(item);
    }

    // Handle empty directory
    if(files.empty()){
        item.name = {std::string{""}};
        item.is_dir = false;
        files.push_back(item);
        curs_set(0);
        wrefresh(left_win);
        return files;
    }

    // Reset cursor position
    curs_set(1);

    for(auto &f : files){
        mvwprintw(left_win, y++, default_pos, f.name.c_str());        
    }

    wmove(left_win, cursor_position, default_pos); 
    wrefresh(left_win);
    return files;
}

static void resize_window(WINDOW *left_win, std::filesystem::path &window_path, std::vector<file> &files, int cursor_position, int scroll_position){
    int y{default_pos};
    
    getmaxyx(stdscr, row, col);
    left_win = newwin(row, col / 2, 0, 0);

    wmove(left_win, 0, 0);
    box(left_win, 0, 0);
    wprintw(left_win, window_path.c_str());

    for(int i = scroll_position; i < files.size(); ++i){
        mvwprintw(left_win, y++, default_pos, files[i].name.c_str());        
    }
    
    wmove(left_win, cursor_position, default_pos);
    wrefresh(left_win);
}

int main(){
    int ch;
    int cursor_pos = default_pos;
    int scroll_pos = 0;
    int selected = 0;
    WINDOW *left_win = nullptr;
    std::vector<file> files;
    std::filesystem::path window_path = std::filesystem::current_path();

    start_ncurses();   

    files = left_window(left_win, window_path, default_pos, scroll_pos);

    while((ch = getch()) != 'q'){
        // Handle window resizing
        if(ch == KEY_RESIZE){
            endwin();
            refresh();
            resize_window(left_win, window_path, files, cursor_pos, scroll_pos);
        }

        switch(ch){
            case down: {
                // Handle scrolling down
                if(ch == down && scroll_pos < files.size() - row + 1 && cursor_pos == row - 1){
                    ++selected;
                    ++scroll_pos;
                    resize_window(left_win, window_path, files, cursor_pos, scroll_pos);
                    refresh();
                } else if(cursor_pos < files.size() && cursor_pos < row - 1){
                    ++cursor_pos;
                    ++selected;
                    move(cursor_pos, default_pos); 
                }
                break;
            }

            case up: {
                // Handle scrolling up
                if(ch == up && scroll_pos > 0 && cursor_pos == default_pos){
                    --selected;
                    --scroll_pos;
                    resize_window(left_win, window_path, files, cursor_pos, scroll_pos);
                    refresh();
                } else if(cursor_pos > default_pos){
                    --cursor_pos;
                    --selected;
                    move(cursor_pos, default_pos);
                }                 
                break;
            }

            case top: 
                cursor_pos = default_pos;
                scroll_pos = 0;
                selected = 0;
                resize_window(left_win, window_path, files, cursor_pos, scroll_pos);
                break;

            case left: 
                window_path = window_path.parent_path();
                cursor_pos = default_pos;
                scroll_pos = 0;
                selected = 0;
                files = left_window(left_win, window_path, cursor_pos, scroll_pos);
                break;
            
            case right: {
                // Opening files not supported yet
                if(files[selected].is_dir){
                    window_path += '/' + files[selected].name;
                    cursor_pos = default_pos;
                    scroll_pos = 0;
                    selected = 0;
                    files = left_window(left_win, window_path, cursor_pos, scroll_pos);
                    break;
                }
                break;
            }                
        }
    }

    delwin(left_win);
    endwin();
}
