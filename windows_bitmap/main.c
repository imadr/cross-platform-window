#include "window.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int main(){
    int width, height, channels;
    unsigned char* data = stbi_load("peppers.png", &width, &height, &channels, 3);

    if(!data){
        return 0;
    }

    init_buffer(width, height);
    for(int i = 0; i < width*height; i++){
        buffer[i] = (0xff<<24)+(data[i*3]<<16)+(data[i*3+1]<<8)+data[i*3+2];
    }

    int err = create_window("my window", 512, 512);

    if(err){
        return 1;
    }

    while(event_loop()){
    }

    return 0;
}