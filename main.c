#include <windows.h>
#include <commdlg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>

#define END_MARKER "1111111111111110"

HWND hwndEdit;  // Message Input

char selectedFile[MAX_PATH]; // selected PNG

// --- Binary conversion ---
void text_to_binary(const char *text, char *binary) {
    binary[0] = '\0';
    for (int i=0; text[i]; i++) {
        for (int j=7; j>=0; j--) {
            strcat(binary, ((text[i]>>j)&1)?"1":"0");
        }
    }
    strcat(binary, END_MARKER);
}

void binary_to_text(char *binary, char *text) {
    int len = strlen(binary);
    int idx=0;
    for(int i=0;i<len;i+=8){
        char byte[9]; strncpy(byte,&binary[i],8); byte[8]='\0';
        text[idx++] = (char)strtol(byte,NULL,2);
    }
    text[idx]='\0';
}

// --- Encode ---
void encode_image(const char *input_path, const char *output_path, const char *message){
    FILE *fp=fopen(input_path,"rb");
    if(!fp){ MessageBox(NULL,"File couldnt open!","Error",MB_OK); return;}

    png_structp png=png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
    png_infop info=png_create_info_struct(png);
    png_init_io(png,fp);
    png_read_info(png,info);

    int width=png_get_image_width(png,info);
    int height=png_get_image_height(png,info);
    png_byte color_type=png_get_color_type(png,info);
    png_byte bit_depth=png_get_bit_depth(png,info);

    if(bit_depth!=8 || (color_type!=PNG_COLOR_TYPE_RGB && color_type!=PNG_COLOR_TYPE_RGBA)){
        MessageBox(NULL,"Only 8-bit RGB OR RGBA PNG supported!","Error",MB_OK);
        return;
    }

    png_bytep *row_pointers=(png_bytep*)malloc(sizeof(png_bytep)*height);
    for(int y=0;y<height;y++) row_pointers[y]=(png_bytep)malloc(png_get_rowbytes(png,info));
    png_read_image(png,row_pointers);
    fclose(fp);

    // mesaj binary
    char binary[2000000];
    text_to_binary(message,binary);
    int len=strlen(binary);
    int idx=0;

    for(int y=0;y<height && idx<len;y++){
        png_bytep row=row_pointers[y];
        for(int x=0;x<width && idx<len;x++){
            png_bytep px=&(row[x*(color_type==PNG_COLOR_TYPE_RGBA?4:3)]);
            for(int c=0;c<3 && idx<len;c++){
                px[c]=(px[c]&~1) | (binary[idx]=='1'?1:0);
                idx++;
            }
        }
    }

    fp=fopen(output_path,"wb");
    png_structp png_w=png_create_write_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
    png_infop info_w=png_create_info_struct(png_w);
    png_init_io(png_w,fp);
    png_set_IHDR(png_w,info_w,width,height,
                 bit_depth,color_type,PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE,PNG_FILTER_TYPE_BASE);
    png_set_rows(png_w,info_w,row_pointers);
    png_write_png(png_w,info_w,PNG_TRANSFORM_IDENTITY,NULL);
    fclose(fp);

    char msg[200];
    sprintf(msg,"Message Hidden: %s",output_path);
    MessageBox(NULL,msg,"Successfull",MB_OK);

    for(int y=0;y<height;y++) free(row_pointers[y]);
    free(row_pointers);
}

// --- Decode ---
void decode_image(const char *input_path){
    FILE *fp=fopen(input_path,"rb");
    if(!fp){ MessageBox(NULL,"File couldnt open!","Error",MB_OK); return;}

    png_structp png=png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
    png_infop info=png_create_info_struct(png);
    png_init_io(png,fp);
    png_read_info(png,info);

    int width=png_get_image_width(png,info);
    int height=png_get_image_height(png,info);
    png_byte color_type=png_get_color_type(png,info);
    png_byte bit_depth=png_get_bit_depth(png,info);

    png_bytep *row_pointers=(png_bytep*)malloc(sizeof(png_bytep)*height);
    for(int y=0;y<height;y++) row_pointers[y]=(png_bytep)malloc(png_get_rowbytes(png,info));
    png_read_image(png,row_pointers);
    fclose(fp);

    char binary[3000000]="";
    int idx=0;
    for(int y=0;y<height;y++){
        png_bytep row=row_pointers[y];
        for(int x=0;x<width;x++){
            png_bytep px=&(row[x*(color_type==PNG_COLOR_TYPE_RGBA?4:3)]);
            for(int c=0;c<3;c++){
                binary[idx++]=(px[c]&1)?'1':'0';
                binary[idx]='\0';
                if(idx>strlen(END_MARKER)){
                    if(strcmp(&binary[idx-strlen(END_MARKER)],END_MARKER)==0){
                        binary[idx-strlen(END_MARKER)]='\0';
                        char text[1000000];
                        binary_to_text(binary,text);
                        SetWindowText(hwndEdit,text);
                        MessageBox(NULL,"Message Decoded.!","Successfull",MB_OK);
                        goto cleanup;
                    }
                }
            }
        }
    }
    MessageBox(NULL,"Couldnt find message!","Error",MB_OK);

cleanup:
    for(int y=0;y<height;y++) free(row_pointers[y]);
    free(row_pointers);
}

// --- Dosya seç ---
void select_file(HWND hwnd){
    OPENFILENAME ofn;
    ZeroMemory(&ofn,sizeof(ofn));
    ofn.lStructSize=sizeof(ofn);
    ofn.hwndOwner=hwnd;
    ofn.lpstrFilter="PNG Files\0*.png\0";
    ofn.lpstrFile=selectedFile;
    ofn.nMaxFile=MAX_PATH;
    if(GetOpenFileName(&ofn)){
        MessageBox(hwnd,selectedFile,"File selected",MB_OK);
    }
}

// --- WinAPI GUI ---
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
    switch(msg){
        case WM_CREATE:{
            CreateWindow("STATIC", "Message:", WS_VISIBLE|WS_CHILD, 10,10,60,20, hwnd, NULL,NULL,NULL);
            hwndEdit=CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT","",WS_CHILD|WS_VISIBLE|ES_MULTILINE|ES_AUTOVSCROLL,10,35,460,150,hwnd,NULL,NULL,NULL);
            CreateWindow("BUTTON","📂 Select file",WS_VISIBLE|WS_CHILD,10,200,100,30,hwnd,(HMENU)1,NULL,NULL);
            CreateWindow("BUTTON","📥 Hide Message",WS_VISIBLE|WS_CHILD,120,200,120,30,hwnd,(HMENU)2,NULL,NULL);
            CreateWindow("BUTTON","📤 Extract Message",WS_VISIBLE|WS_CHILD,250,200,120,30,hwnd,(HMENU)3,NULL,NULL);
        } break;
        case WM_COMMAND:{
            switch(LOWORD(wParam)){
                case 1: select_file(hwnd); break;
                case 2:{
                    char text[1000000];
                    GetWindowText(hwndEdit,text,1000000);
                    if(strlen(selectedFile)==0){MessageBox(hwnd,"Firstly select a file!","Error",MB_OK); break;}
                    char output[MAX_PATH];
                    sprintf(output,"%s_encoded.png",selectedFile);
                    encode_image(selectedFile,output,text);
                } break;
                case 3:{
                    if(strlen(selectedFile)==0){MessageBox(hwnd,"Firstly select a file!","Error",MB_OK); break;}
                    decode_image(selectedFile);
                } break;
            }
        } break;
        case WM_DESTROY: PostQuitMessage(0); break;
        default: return DefWindowProc(hwnd,msg,wParam,lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nCmdShow){
    WNDCLASS wc={0};
    wc.lpfnWndProc=WndProc;
    wc.hInstance=hInst;
    wc.lpszClassName="StegGUI";
    wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
    RegisterClass(&wc);

    HWND hwnd=CreateWindow("StegGUI","Steganography GUI",WS_OVERLAPPEDWINDOW|WS_VISIBLE,100,100,500,300,NULL,NULL,hInst,NULL);

    MSG msg;
    while(GetMessage(&msg,NULL,0,0)){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}