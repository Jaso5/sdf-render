#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <stdint.h>
#include <math.h>

#include "help.h"

#define BUFFER_WIDTH 720
#define BUFFER_HEIGHT 576

#define PTS_COUNT 1

static const int buffer_width = BUFFER_WIDTH;
static const int buffer_height = BUFFER_HEIGHT;
static uint16_t (*mem)[BUFFER_WIDTH];
static const float tolerance = 0.2;

// static float pts[1][4];
// static float cam[3];
// static float target[3];

float dst(float v1[3], float v2[3]) {
	float sum[3] = {
		pow(v1[0] - v2[0], 2),
                pow(v1[1] - v2[1], 2),
                pow(v1[2] - v2[2], 2),
	};

	// printf("sum = %f \n", sum[0] + sum[1] + sum[2]);

	return sqrtf(sum[0] + sum[1] + sum[2]);
}


int sdf(float _pos[3], float diff[3], float normal[3], float pts[PTS_COUNT][4]) {
	int collided = 0;
	float travelled = 0.0;

	float pos[3] = {_pos[0], _pos[1], _pos[2]};

	while (!collided) {
		float dist = dst(pos, pts[0]) - pts[0][4];

		// printf("dist = %f \n", dist);

		if (dist <= tolerance) {
			collided = 1;
			break;
		} else {
			pos[0] += diff[0] * dist;
			pos[1] += diff[1] * dist;
			pos[2] += diff[2] * dist;
			travelled += dist;
			if (travelled > 50.0) {
				// printf("Went too far! travelled = %f \n", travelled);
				return 0;
			}
		}
	}

	normal[0] = pos[0];
	normal[1] = pos[1];
	normal[2] = pos[2];

	return collided;
}

uint16_t conv(float color[3]) {
	uint16_t r = (uint16_t) color[0] * 16;
	uint16_t g = (uint16_t) color[1] * 32;
	uint16_t b = (uint16_t) color[2] * 16;

	uint16_t res = 0;

	res |= b;
	res |= (g << 5);
	res |= (r << 11);

	return res;
}

void screen(float pts[PTS_COUNT][4]) {
	for (int x = 0; x < BUFFER_WIDTH; x++) {
                for (int y = 0; y < BUFFER_HEIGHT; y++) {
                        float pos[3] = {
				((float)x / BUFFER_WIDTH) - 0.5,
				((float)y / BUFFER_HEIGHT) - 0.5,
				-10.0
			};

			float diff[3] = { // Must be unit vector
				0.0,
				0.0,
				1.0,
			};

			float normal[3] = {0.0, 0.0, 0.0};

			int res = sdf(pos, diff, normal, pts);

			if (res == 1) {
				mem[y][x] = 0xFFFF;
			} else {
				mem[y][x] = 0x0000;
			}
                }
        }
}

int main() {
	int fd = open("/dev/fb0", O_RDWR);
  	mem = (uint16_t (*)[buffer_width]) mmap(NULL, buffer_width * buffer_height * 2, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

	// cam = {10.0, 10.0, 10.0};
	// float target[3] = {1.0, 0.0, 0.0};

	float pts[PTS_COUNT][4] = { 0.0, 0.0, 0.0, 2.0 };

	// while (1) {
		screen(pts);
	// }

	float pos[3] = {1.0, 0.0, -10.0};
	float diff[3] = {0.0, 0.0,1.0};
	float normal[3] = {0.0, 0.0, 0.0};

	// int a = sdf(pos, diff, normal, pts);

	// printf("a = %i \n", a);

	munmap(mem, buffer_width * buffer_height * 2);

	return 0;
}


