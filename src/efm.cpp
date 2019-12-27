#include <filesystem>
#include <future>
#include <ncurses.h>
#include "files.hpp"

constexpr int default_pos = 1;

static void start_ncurses(){
    initscr();
    cbreak();
    noecho();
    refresh();
}

static files left_window(WINDOW *left_win, std::filesystem::path &window_path){
    int y{default_pos}, x{default_pos};
    int row, col;
    files item;

    getmaxyx(stdscr, row, col);
    left_win = newwin(row - 1, col / 2, 0, 0);

    wmove(left_win, 0, 0);
    box(left_win, 0, 0);
    wprintw(left_win, window_path.c_str());

    std::vector<std::filesystem::directory_entry> temp_files;
    for(auto& p: std::filesystem::directory_iterator(window_path)){
        if(p.is_directory()){
            item.name.push_back(p);
        } else {
            temp_files.push_back(p);
        }
    }

    // Handle empty directory
    if(temp_files.empty() && item.name.empty()){
        item.position = {1};
        item.name = {std::filesystem::directory_entry{""}};
        curs_set(0);
        wrefresh(left_win);
        return item;
    }

    // Reset cursor position
    curs_set(1);

    std::move(temp_files.begin(), temp_files.end(), std::back_inserter(item.name));

    for(auto &p : item.name){
        item.position.push_back(y);
        mvwprintw(left_win, y++, x, p.path().filename().c_str());
    }
    
    wmove(left_win, default_pos, default_pos);
    wrefresh(left_win);
    return item;
}

int main(){
    int ch;
    int selected = 0;
    WINDOW *left_win = nullptr;
    files item;
    std::filesystem::path window_path = std::filesystem::current_path();

    start_ncurses();   

    auto t1 = std::async(std::launch::async, left_window, std::ref(left_win), std::ref(window_path));
    item = t1.get();

    while((ch = getch()) != 'q'){
        if(ch == KEY_RESIZE){
            endwin();
            refresh();
            item = left_window(left_win, window_path);
        }

        switch(ch){
            case 'j': {
                if(item.position[selected] < item.name.size()){
                    ++selected;
                    move(item.position[selected], default_pos);
                    break; 
                }
                break;
            }

            case 'k': {
                if(item.position[selected] > default_pos){
                    --selected;
                    move(item.position[selected], default_pos);
                    break;
                }
                break;
            }

            case 'g': 
                selected = 0;
                move(default_pos, default_pos);
                break;
  
            case 'h': 
                window_path = window_path.parent_path();
                selected = 0;
                item = left_window(left_win, window_path);
                break;
            
            case 'l': {
                if(item.name[selected].is_directory()){
                    window_path = item.name[selected].path();
                    selected = 0;
                    item = left_window(left_win, window_path);
                    break;

                }
                break;
            }                
        }
    }

    delwin(left_win);
    endwin();
}
