#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

struct rgb_img{
    uint8_t *raster;
    size_t height;
    size_t width;
};

void create_img(struct rgb_img **im, size_t height, size_t width){
    *im = (struct rgb_img *)malloc(sizeof(struct rgb_img));
    (*im)->height = height;
    (*im)->width = width;
    (*im)->raster = (uint8_t *)malloc(3 * height * width);
}

int read_2bytes(FILE *fp){
    uint8_t bytes[2];
    fread(bytes, sizeof(uint8_t), 1, fp);
    fread(bytes+1, sizeof(uint8_t), 1, fp);
    return (  ((int)bytes[0]) << 8)  + (int)bytes[1];
}

void write_2bytes(FILE *fp, int num){
    uint8_t bytes[2];
    bytes[0] = (uint8_t)((num & 0XFFFF) >> 8);
    bytes[1] = (uint8_t)(num & 0XFF);
    fwrite(bytes, 1, 1, fp);
    fwrite(bytes+1, 1, 1, fp);
}

void read_in_img(struct rgb_img **im, char *filename){
    FILE *fp = fopen(filename, "rb");
    size_t height = read_2bytes(fp);
    size_t width = read_2bytes(fp);
    create_img(im, height, width);
    fread((*im)->raster, 1, 3*width*height, fp);
    fclose(fp);
}

void write_img(struct rgb_img *im, char *filename){
    FILE *fp = fopen(filename, "wb");
    write_2bytes(fp, im->height);
    write_2bytes(fp, im->width);
    fwrite(im->raster, 1, im->height * im->width * 3, fp);
    fclose(fp);
}

uint8_t get_pixel(struct rgb_img *im, int y, int x, int col){
    return im->raster[3 * (y*(im->width) + x) + col];
}

void set_pixel(struct rgb_img *im, int y, int x, int r, int g, int b){
    im->raster[3 * (y*(im->width) + x) + 0] = r;
    im->raster[3 * (y*(im->width) + x) + 1] = g;
    im->raster[3 * (y*(im->width) + x) + 2] = b;
}

void destroy_image(struct rgb_img *im)
{
    free(im->raster);
    free(im);
}


void print_grad(struct rgb_img *grad){
    int height = grad->height;
    int width = grad->width;
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            printf("%d\t", get_pixel(grad, i, j, 0));
        }
    printf("\n");    
    }
}

void print_best(double *best, int height, int width){
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            printf("%f\t", best[i*width+ j]);
        }
    printf("\n");    
    }
}

#if !defined(SEAMCARVING_H)
#define SEAMCARVING_H

void calc_energy(struct rgb_img *im, struct rgb_img **grad);
void dynamic_seam(struct rgb_img *grad, double **best_arr);
void recover_path(double *best, int height, int width, int **path);
void remove_seam(struct rgb_img *src, struct rgb_img **dest, int *path);

#endif

void print_grad_R(struct rgb_img *grad){
    int height = grad->height;
    int width = grad->width;
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            printf("%d\t", get_pixel(grad, i, j, 0));
        }
    printf("\n");    
    }
}

void print_grad_G(struct rgb_img *grad){
    int height = grad->height;
    int width = grad->width;
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            printf("%d\t", get_pixel(grad, i, j, 1));
        }
    printf("\n");    
    }
}

void print_grad_B(struct rgb_img *grad){
    int height = grad->height;
    int width = grad->width;
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            printf("%d\t", get_pixel(grad, i, j, 2));
        }
    printf("\n");    
    }
}

//COPY FROM HERE

#include "seamcarving.h"

//HELPER FUNCTIONS

//Gets the red value of a specified pixel at row y and column x
uint8_t get_R(struct rgb_img *im, int y, int x){
    return im->raster[(y * im->width + x) * 3];
}

//Gets the green value of a specified pixel at row y and column x
uint8_t get_G(struct rgb_img *im, int y, int x){
    return im->raster[(y * im->width + x) * 3 + 1];
}

//Gets the blue value of a specified pixel at row y and column x
uint8_t get_B(struct rgb_img *im, int y, int x){
    return im->raster[(y * im->width + x) * 3 + 2];
}

double delta_2x(struct rgb_img *im, int y, int x){
    int upper_x = x + 1;
    int lower_x = x - 1;

    //Handling out of bounds pixels
    if (lower_x < 0){lower_x = (im->width - 1);}
    if (upper_x >= im->width){upper_x = 0;}

    //Computing RGB x values
    double Rx = get_R(im, y, upper_x) - get_R(im, y, lower_x);
    double Gx = get_G(im, y, upper_x) - get_G(im, y, lower_x);
    double Bx = get_B(im, y, upper_x) - get_B(im, y, lower_x);

    double result = (Rx*Rx) + (Gx*Gx) + (Bx*Bx);
    return result;
}

double delta_2y(struct rgb_img *im, int y, int x){
    int upper_y = y + 1;
    int lower_y = y - 1;

    //Handling out of bounds pixels
    if (lower_y < 0){lower_y = (im->height - 1);}
    if (upper_y >= im->height){upper_y = 0;}

    //Computing RGB x values
    double Ry = get_R(im, upper_y, x) - get_R(im, lower_y, x);
    double Gy = get_G(im, upper_y, x) - get_G(im, lower_y, x);
    double By = get_B(im, upper_y, x) - get_B(im, lower_y, x);

    return Ry*Ry + Gy*Gy + By*By;
}

//Calculates the energy of a pixel
double get_pixel_energy(struct rgb_img *im, int y, int x){
    double d2x = delta_2x(im, y, x);
    double d2y = delta_2y(im, y, x);

    return sqrt(d2x + d2y);
}


//Calculates energy of image, and puts it into each "pixel"
void calc_energy(struct rgb_img *im, struct rgb_img **grad){
    int height = im->height;
    int width = im->width;
    create_img(grad, height, width); // allocate memory for grad->raster
    for (int i = 0; i < im->height; i++){
        for (int j = 0; j < im->width; j++){
            double energy = get_pixel_energy(im, i, j);
            uint8_t dual_energy = (uint8_t) (energy/10);
            set_pixel(*grad, i, j, dual_energy, dual_energy, dual_energy);
        }
    }
}

//Gets the minimum of 3 values
double min(double a, double b, double c) {
    double min = a;
    if (b < min) {
        min = b;
    }
    if (c < min) {
        min = c;
    }
    return min;
}

void dynamic_seam(struct rgb_img *grad, double **best_arr){
    int h = (int) (grad->height);
    int w = (int) (grad->width);

    *best_arr = malloc(h * w * sizeof(double));

    for (int i = 0; i < h; i++){
        for (int j = 0; j < w; j++){
            (*best_arr)[i * w + j] = (double)(grad->raster[(i * w + j) * 3]);
        }
    }

    for (int i = 1; i < h; i++){
        for (int j = 0; j < w; j++){
            double opt_val;

            //value directly above
            double top = (*best_arr)[((i-1) * w + j)];

            //top left
            double left;
            if (j-1 < 0){
                left = INFINITY;
            }
            else{
                left = (*best_arr)[((i-1) * w + (j-1))];
            }

            //top right
            double right;
            if (j+1 >= w){
                right = INFINITY;
            }
            else{
                right = (*best_arr)[((i-1) * w + (j+1))];
            }

            opt_val = min(top, left, right) + (*best_arr)[i * w + j];

            (*best_arr)[i * w + j] = opt_val;
        }
    }
}

void recover_path(double *best, int height, int width, int **path){
    *path = malloc(height * sizeof(int));
    for (int i = 0; i < height; i++){
        int row_min_ind = i*width;
        for (int j = 0; j < width; j++){
            if ((best)[i*width + j] < (best)[row_min_ind]){
                row_min_ind = i*width + j;
            }
        }
        row_min_ind = row_min_ind%width;
        (*path)[i] = row_min_ind;
    }
}

void remove_seam(struct rgb_img *src, struct rgb_img **dest, int *path){
    int h = src->height;
    int w = src->width;
    int rgb_channels = 3;

    *dest = malloc(sizeof(struct rgb_img));
    (*dest)->height = h;
    (*dest)->width = w;
    (*dest)->raster = malloc(h * w * rgb_channels * sizeof(uint8_t));

    //traverses image, shifts every pixel right by 1, and pixels after seam by 2, which gets rid of seam pixels
    for (int i = 0; i < h; i++){
        for (int j = 0; j < w; j++){

            int src_ind;

            if (j >= path[i]){
                src_ind = (i * (w + 1) + j + 1) * rgb_channels; //pixel index, as pixels have 3 values
                //shifted up by 1 to account for missing seam pixel in row
            }
            else{
                src_ind = (i * (w + 1) + j) * rgb_channels; //w+1 to skip over seam pixels
            }
            int dest_ind = (i * w + j) * rgb_channels; 
            memcpy((*dest)->raster + dest_ind, src->raster + src_ind, rgb_channels * sizeof(uint8_t)); //copies one pixel of size pixel from src to dest
        }
    }

}


int main(){
    struct rgb_img *grad;

    struct rgb_img *im;

    create_img(&im, 5, 6);

    create_img(&grad, 5, 6);

    read_in_img(&im, "6x5.bin");

    double d2x = delta_2x(im, 2, 0);
    printf("%f\n", d2x);

    double d2y = delta_2y(im, 2, 0);
    printf("%f\n", d2y);
  
    calc_energy(im,  &grad);
  
    print_grad(grad);

    double *best;

    dynamic_seam(grad, &best);
    printf("BEST ARRAY:\n");

    print_best(best, 5, 6);

    int *path;

    recover_path(best, 5, 6, &path);

    for (int i = 0; i < 5; i++){
        printf("%d\n", path[i]);
    }

    return 0;
}