#include "window.h"

int main(){
    int err = create_window("my window", 500, 500);

    if(err){
        return 1;
    }

    while(event_loop()){
    }

    return 0;
}