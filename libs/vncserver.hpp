/*
    vncserver.hpp - source code

    =================================
      vnc class for PIwebVNC in C++
    =================================

    Free to use, free to modify, free to redistribute.
    Created by : Jishan Ali Mondal

    This is a header-only library.
    created for only PIwebVNC
    * This code was created entirely for the most optimized performance for PIwebVNC *
    * May not be suitable for other projects *
    version 1.0.1
*/

#ifndef WEBVNC_HPP
#define WEBVNC_HPP

#include <iostream>
#include <unistd.h>
#include <string>
#include <lz4.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xdamage.h>
#include <X11/extensions/XTest.h>
#include <jpeglib.h>

#include "display.hpp"
#include "input.hpp"
#include "xOptimizer.hpp"

class VNCServer
{
    public:
        XDisplay xdisplay;
        XInputs *inputs;
        VNCServer();
        ~VNCServer();
        void start_service(Websocket &ws);
        void stop_service();
        void send_first_frame(int sd); // send the first frame to the client
    private:
        void threadSleep();
        unsigned char *compress_image_to_jpeg(char *input_image_data, int width, int height, int *out_size, int quality);
        Display * display;
        Damage damage;
        ScreenInfo screenInfo;
        char config[500] = {0}; // 500 byte not too much
        bool isRunning = true;
        bool sendFirstFrame = false;
        int clientSD = 0;
        int sleepDelay = 1000000 / FPS;
        int sleepLoop = (1000000 / FPS)/this->sleepDelay;
        int bufferSize = 1000000;
        char *buffer;
};

VNCServer::VNCServer()
{
    int damage_event, damage_error;   
    this->display = this->xdisplay.getDisplay();
    this->screenInfo = this->xdisplay.getScreenInfo();
    strcpy(this->config,this->xdisplay.getDisplayConfig().c_str());

    XDamageQueryExtension(display, &damage_event, &damage_error);
    this->damage = XDamageCreate(display, this->screenInfo.root, XDamageReportNonEmpty);

    this->bufferSize = (this->screenInfo.height * this->xdisplay.getBitPerLine()) + (2 * this->screenInfo.width);
    this->buffer = (char *)malloc(this->bufferSize * sizeof(char));
    if (this->buffer == NULL)
    {
        printf("[ERROR] Memory not allocated for VNC buffer.\n");
        exit(0);
    }
}

VNCServer::~VNCServer()
{
    free(this->buffer);
    printf("[INFO] Memory released for VNC buffer.\n");
    XDamageDestroy(display, damage);
    this->xdisplay.close();
}

void VNCServer::stop_service()
{
    this->isRunning = false;
}
void VNCServer::send_first_frame(int sd)
{
    this->clientSD = sd;
    this->sendFirstFrame = true;
}
void VNCServer::start_service(Websocket &ws)
{
    register XImage *image;
    const XserverRegion xregion = XFixesCreateRegion(this->display, NULL, 0);
    bool doCompress = 0;
    while(this->isRunning)
    {
        threadSleep();
        if (ws.clients > 0)
        {
            if (this->sendFirstFrame)
            {
                image = XGetImage(this->display, this->screenInfo.root, 0, 0, this->screenInfo.width
                    , this->screenInfo.height, AllPlanes, ZPixmap);
                int frameSize = ((this->screenInfo.height - 1) * image->bytes_per_line + (this->screenInfo.width - 1));
                int compressedSize = LZ4_compress_default(image->data, this->buffer, frameSize, this->bufferSize);
                std::string data = "UPD" + std::to_string(0) + " " + std::to_string(0) + " " 
                    + std::to_string(this->screenInfo.width) + " " + std::to_string(this->screenInfo.height) + " " 
                    + std::to_string(image->bytes_per_line) + " " + std::to_string(compressedSize) + " \n";
                char *info = (char *)data.c_str();
                int infoSize = strlen(info);
                XDestroyImage(image);
                ws.sendText(this->config, this->clientSD);
                ws.sendFrame(info, buffer, infoSize, compressedSize, this->clientSD);
                this->sendFirstFrame = false;
                usleep(100000);
            }
            else
            {
                int partCounts = 0;
                XDamageSubtract(this->display, this->damage, None, xregion);
                XRectangle *rect = XFixesFetchRegion(this->display, xregion, &partCounts);
                //check if same frame is getting changed
                for (int i = 0; i < partCounts; i++)
                {
                    image = XGetImage(display, this->screenInfo.root, rect[i].x, rect[i].y, rect[i].width, rect[i].height, AllPlanes, ZPixmap);
                    int frameSize = (rect[i].height * image->bytes_per_line);
                    int optimization = optimizationManager.checkOptimization(rect[i]);
                    if(optimization < 100)
                        {
                            // send JPEG compressed frame
                            int compressedSize = 0;
                            int cord = 0, cordb = 0;
                            for (int y = 0; y < image->height; y++)
                            {
                                for (int x = 0; x < image->width; x++)
                                {
                                    unsigned long pixel = XGetPixel(image, x, y);
                                    unsigned char r = (pixel) >> 16;
                                    unsigned char g = (pixel) >> 8;
                                    unsigned char b = pixel;

                                    this->buffer[cordb++] = r; // R
                                    this->buffer[cordb++] = g; // G
                                    this->buffer[cordb++] = b; // B
                                }
                            }
                            char *jpeg_data = (char *)this->compress_image_to_jpeg(this->buffer, rect[i].width, rect[i].height, &compressedSize, optimization);
                            std::string data = "VPD" + std::to_string(rect[i].x) + " " + std::to_string(rect[i].y) + " " + std::to_string(rect[i].width) + " " + std::to_string(rect[i].height) + " " + std::to_string(image->bytes_per_line) + " " + std::to_string(compressedSize) + " \n";
                            char *info = (char *)data.c_str();
                            int infoSize = strlen(info);
                            if (!XDestroyImage(image))
                                free(image);
                            ws.sendFrame(info, jpeg_data, infoSize, compressedSize);
                            delete jpeg_data;
                        }
                    else
                    {
                        int compressedSize = LZ4_compress_default(image->data, this->buffer, frameSize, this->bufferSize);
                        std::string data = "UPD" + std::to_string(rect[i].x) + " " + std::to_string(rect[i].y) + " " + std::to_string(rect[i].width) + " " + std::to_string(rect[i].height) + " " + std::to_string(image->bytes_per_line) + " " + std::to_string(compressedSize) + " \n";
                        char *info = (char *)data.c_str();
                        int infoSize = strlen(info);
                        if (!XDestroyImage(image))
                            free(image);
                        ws.sendFrame(info, this->buffer, infoSize, compressedSize);
                    }
                }
                XFree(rect);
            }
        }
        else{
            while(ws.clients == 0){
                sleep(1);
            }
        }
    }
}

void VNCServer::threadSleep()
{
    // loop and sleep for 50ms
    int i = this->sleepLoop;
    while(i--)
    {
        this->inputs->dispatchEvents();
        usleep(this->sleepDelay);
    }
}

unsigned char *VNCServer::compress_image_to_jpeg(char *input_image_data, int width, int height, int *out_size, int quality)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    // Create a memory destination for the compressed image
    unsigned char *jpeg_data = NULL;
    unsigned long jpeg_size = 0;

    // Set up the error handler
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    // Memory buffer to hold the compressed data
    jpeg_mem_dest(&cinfo, &jpeg_data, &jpeg_size);

    // Set the compression parameters
    cinfo.image_width = width;      // Image width
    cinfo.image_height = height;    // Image height
    cinfo.input_components = 3;     // RGB, so 3 components
    cinfo.in_color_space = JCS_RGB; // Color space is RGB

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE); // Set the quality (0-100)

    // Start compression
    jpeg_start_compress(&cinfo, TRUE);

    // Write the image data
    while (cinfo.next_scanline < cinfo.image_height)
    {
        unsigned char *row_pointer = (unsigned char *)&input_image_data[cinfo.next_scanline * width * 3];
        jpeg_write_scanlines(&cinfo, &row_pointer, 1);
    }

    // Finish the compression
    jpeg_finish_compress(&cinfo);

    // Set the output size
    *out_size = jpeg_size;

    // Clean up
    jpeg_destroy_compress(&cinfo);

    // Return the compressed image data (JPEG in memory)
    return jpeg_data;
}



#endif