#define _USE_MATH_DEFINES

#include <stdio.h>
#include <math.h>

#define SIZE 320

#define HIGHLIGHT_X         -0.25
#define HIGHLIGHT_Y         -0.2
#define HIGHLIGHT_SHININESS  1.2
#define RAYS                  16
#define RAY_SHAPE_FACTOR     1.9
#define CROWN_INNER_RADIUS   0.60
#define CROWN_MIN_THICKNESS  0.10
#define CORE_RADIUS          0.50
#define EDGE_WIDTH           0.04
#define EDGE_STRENGTH        0.3
#define GLOSS_X              0.0
#define GLOSS_Y             -1.95
#define GLOSS_RADIUS         2.0
#define GLOSS_STRENGTH       0.2

typedef unsigned char byte;

typedef struct {
    byte r;
    byte g;
    byte b;
} Pixel;

Pixel COLOR_BASE         = { 255, 128,   0 };
Pixel COLOR_HIGHLIGHT    = { 255, 255,   0 };
Pixel COLOR_GLOSS        = { 255, 255, 255 };
Pixel COLOR_BACKGROUND   = { 255, 255, 255 };

Pixel raster[SIZE][SIZE];

void save_image(const char *filename) {
    FILE *f_out = fopen(filename, "wt+");
    int i, j;
    
    fprintf(f_out, "P3 %d %d 255\n", SIZE, SIZE);
    
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            fprintf(f_out, "%d %d %d ",
                raster[i][j].r, raster[i][j].g, raster[i][j].b);
        }
        fprintf(f_out, "\n");
    }
    
    fclose(f_out);
}

double eucl_dist(double x1, double y1, double x2, double y2)
{
    double dx = x2 - x1;
    double dy = y2 - y1;
    
    return sqrt(dx * dx + dy * dy);
}

double comp_theta(double x1, double y1, double x2, double y2)
{
    double dx = x2 - x1;
    double dy = y2 - y1;
    
    double ang = atan2(dy, dx);
    
    return (ang < 0) ? (ang + 2 * M_PI) : ang;
}

void color_between(Pixel *pix, double at, Pixel *start, Pixel *end)
{
    if (at < 0) {
        at = 0;
    }
    if (at > 1) {
        at = 1;
    }
    
    pix->r = (int)(start->r * (1-at) + end->r * at);
    pix->g = (int)(start->g * (1-at) + end->g * at);
    pix->b = (int)(start->b * (1-at) + end->b * at);
}

// How close we are to the center of a ray
double comp_ray_theta(double theta)
{
    return 1 - fabs(fmod(theta * RAYS, 2 * M_PI) - M_PI) / M_PI;
}

int crown_mask(double x, double y)
{
    double ro = eucl_dist(x, y, 0, 0);
    
    if ((ro > 1) || (ro < CROWN_INNER_RADIUS)) {
        return 0;
    }
    
    if (ro < CROWN_INNER_RADIUS + CROWN_MIN_THICKNESS) {
        return 1;
    }
    
    // normalize
    ro = (ro - CROWN_INNER_RADIUS - CROWN_MIN_THICKNESS) /
        (1 - CROWN_INNER_RADIUS - CROWN_MIN_THICKNESS);
    
    double theta = comp_theta(x, y, 0, 0);
    double ray_theta = comp_ray_theta(theta);

    ray_theta = pow(ray_theta, RAY_SHAPE_FACTOR);

    return (ro <= ray_theta);
}

int crown_edge(double x, double y)
{
    if (!crown_mask(x, y)) {
        return 0;
    }
    
    double samp = 0.01;
    
    int sx,sy;
    double dx,dy;
    for (sx = -1; sx <= 1; sx += 2)
        for (sy = -1; sy <= 1; sy += 2)
            for (dx = 0; dx < EDGE_WIDTH; dx += samp)
                for (dy = 0; dy < EDGE_WIDTH; dy += samp)
                    if (!crown_mask(x + dx*sx, y + dy*sy))
                        return 1;
    
    return 0;
}

int core_mask(double x, double y)
{
    double ro = eucl_dist(x, y, 0, 0);
    
    return ro <= CORE_RADIUS;
}

int core_edge(double x, double y)
{
    double ro = eucl_dist(x, y, 0, 0);
    
    return (ro <= CORE_RADIUS) && (ro >= CORE_RADIUS - EDGE_WIDTH);
}

void get_pixel(double x, double y, Pixel *pix)
{
    if (crown_mask(x,y) || core_mask(x,y)) {
        double ro = eucl_dist(x, y, HIGHLIGHT_X, HIGHLIGHT_Y);
        
        color_between(pix, pow(ro, HIGHLIGHT_SHININESS),
            &COLOR_HIGHLIGHT, &COLOR_BASE);
        
        if (eucl_dist(x, y, GLOSS_X, GLOSS_Y) < GLOSS_RADIUS) {
            color_between(pix, GLOSS_STRENGTH, pix, &COLOR_GLOSS);
        }
        
        if (crown_edge(x,y) || core_edge(x,y)) {
            color_between(pix, EDGE_STRENGTH, pix, &COLOR_BASE);
        }
            
        return;
    }
    
    *pix = COLOR_BACKGROUND;
}

void gen_image() {
    int i,j;
    double x, y;
    
    for (i = 0; i < SIZE; i++)
        for (j = 0; j < SIZE; j++) {
            // normalize coords to -1.0..1.0
            x = (j + 0.5 - (SIZE / 2.0)) / (SIZE / 2.0);
            y = (i + 0.5 - (SIZE / 2.0)) / (SIZE / 2.0);
            get_pixel(x, y, raster[i]+j);
        }
}

int main(int argc, char **argv) {
    gen_image();
    save_image("sun.ppm");
    
    return 0;
}
