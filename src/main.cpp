#include "CImg.h"
#include <iostream>

using namespace cimg_library;

int setColor(int r, int g, int b);
float psnr(CImg<> img_o, CImg<> img_e); // image_original and image_edited


int main(int argc, char const *argv[]) {
    CImg<float> img_read;

    if (argc != 2) {
		printf("ERROR : arguments not complete\n");
		return 1;
	}
    img_read.load(argv[1]);
    int height = img_read.height();
    int width = img_read.width();
    //
    int half_height = height / 2;
    int half_width = width / 2;

    // RGB
    CImg<> red(half_width, half_height, 1,1);
    CImg<> green(half_width, half_height, 1,1);
    CImg<> red_resampled(width, height, 1, 1);
    CImg<> green_resampled(width, height, 1, 1);

    //YCbCr
    CImg<> Y(width, height, 1, 1);
    CImg<> Cb(half_width, half_height, 1, 1);
    CImg<> Cr(half_width, half_height, 1, 1);
    CImg<> Cb_resampled(width, height, 1, 1);
    CImg<> Cr_resampled(width, height, 1, 1);

    // YCbCr
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            Y(i, j, 0) = 0.3 * img_read(i, j, 0, 0) + 0.6 * img_read(i, j, 0, 1) + 0.1 * img_read(i, j, 2);
            if (j < half_height && i < half_width) {
                //Red Green
                red(i, j, 0) = img_read(i*2, j*2, 0, 0);
                green(i, j, 0) = img_read(i*2, j*2, 0, 1);
                //Cb Cr
                Cb(i, j, 0) = -0.1687 * img_read(i*2, j*2, 0, 0) - 0.3313 * img_read(i*2, j*2, 0, 1) + 0.5 * img_read(i*2, j*2, 0, 2) + 128;
                //tab_
                Cr(i, j, 0) = 0.5 * img_read(i*2, j*2, 0, 0) - 0.4187 * img_read(i*2, j*2, 0, 1) - 0.0813 * img_read(i*2, j*2, 0, 2) + 128;
            }
        }
    }


    int line;
    int col;
    float t;
    float u;
    float tmp;
    float d1, d2, d3, d4;
    float p1, p2, p3, p4; /* nearby pixels */

    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            tmp = (float) j / (float) (height - 1) * (half_height - 1);
            line = (int) floor(tmp);

            if (line < 0)
                line = 0;
            else
                if (line >= half_height - 1)
                    line = half_height - 2;
            u = tmp - line;
            tmp = (float) (i) / (float) (width - 1) * (half_width - 1);
            col = (int) floor(tmp);
            if ( col < 0)
                col = 0;
            else
                if (col >= half_width - 1)
                    col = half_width - 2;
            t = tmp - col;

            /* Coefficients */
            d1 = (1 - t) * (1 - u);
            d2 = t * (1 - u);
            d3 = t * u;
            d4 = (1 - t) * u;

            /*neighborhood pixels*/
            p1 = red(col, line, 0);
            p2 = red(col + 1, line, 0);
            p3 = red(col + 1, (line + 1), 0);
            p4 = red(col, (line + 1), 0);
            red_resampled(i, j) = p1 * d1 + p2 * d2 + p3 * d3 + p4 * d4;

            /*neighborhood pixels*/
            p1 = green(col, line, 0);
            p2 = green(col + 1, line, 0);
            p3 = green(col + 1, (line + 1), 0);
            p4 = green(col, (line + 1), 0);
            green_resampled(i, j) = p1 * d1 + p2 * d2 + p3 * d3 + p4 * d4;

            /*neighborhood pixels*/
            p1 = Cb(col, line, 0);
            p2 = Cb(col + 1, line, 0);
            p3 = Cb(col + 1, (line + 1), 0);
            p4 = Cb(col, (line + 1), 0);
            Cb_resampled(i, j) = p1 * d1 + p2 * d2 + p3 * d3 + p4 * d4;

            /*neighborhood pixels*/
            p1 = Cr(col, line, 0);
            p2 = Cr(col + 1, line, 0);
            p3 = Cr(col + 1, (line + 1), 0);
            p4 = Cr(col, (line + 1), 0);
            Cr_resampled(i, j) = p1 * d1 + p2 * d2 + p3 * d3 + p4 * d4;
        }
    }

    red.save_png("../img/halfRedChannel.png");
    green.save_png("../img/halfGreenChannel.png");
    red_resampled.save_png("../img/red_resampled.png");
    green_resampled.save_png("../img/green_resampled.png");

    Y.save_png("../img/Y.png");
    Cb.save_png("../img/Cb.png");
    Cr.save_png("../img/Cr.png");
    Cb_resampled.save_png("../img/Cb_resampled.png");
    Cr_resampled.save_png("../img/Cr_resampled.png");


    CImg<> new_RGB(width, height, 1, 3);
    CImg<> YCbCr(width, height, 1, 3);

    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            // RGB
            new_RGB(i, j, 0, 0) = red_resampled(i, j, 0);
            new_RGB(i, j, 0, 1) = green_resampled(i, j, 0);
            new_RGB(i, j, 0, 2) = img_read(i, j, 0, 2);
            // YCbCr
            float r = abs(Y(i, j, 0) + 1.402 * (Cr_resampled(i, j, 0)));
            float g = Y(i, j, 0) - 0.34414 * (Cb_resampled(i, j, 0) - 128) - 0.71414 * (Cr_resampled(i, j, 0) - 128);
            float b = Y(i, j, 0) + 1.772 * (Cb_resampled(i, j, 0) - 128);
            YCbCr(i, j, 0, 0) = r; //Y(i, j, 0);
            YCbCr(i, j, 0, 1) = g; //Cr_resampled(i, j, 0);
            YCbCr(i, j, 0, 2) = b; //Cb_resampled(i, j, 0);
        }
    }
    new_RGB.save_png("../img/new_RGB.png");
    YCbCr.save_png("../img/YCbCr.png");
//    YCbCr.get_RGBtoYCbCr().save_png("png_test.png");

    double psnr_rgb = psnr(img_read, new_RGB);
    double psnr_ycbcr = psnr(img_read, YCbCr);
    std::cout << "The PSNR of original image and the resampled rgb = " << psnr_rgb << '\n';
    std::cout << "The PSNR of original image and the resampled YCb = " << psnr_ycbcr << '\n';

    return 0;
}

int setColor(int r, int g, int b) {
	return (255<<24) | (r  << 16) | (g << 8) | b;
}

float psnr(CImg<> img_o, CImg<> img_e) {
    int height = img_o.height();
	int width = img_o.width();

	// Erreur quadratique moyenne
	float eqm = 0.0;
    float eqm_r = 0.0, eqm_g = 0.0, eqm_b = 0.0;
	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
            float r = img_o(i, j, 0, 0) - img_e(i, j, 0, 0);
            float g = img_o(i, j, 0, 1) - img_e(i, j, 0, 1);
            float b = img_o(i, j, 0, 2) - img_e(i, j, 0, 2);
            eqm_r += pow(r, 2);
            eqm_g += pow(g, 2);
            eqm_b += pow(b, 2);
		}
	}
	int d = 255;
	eqm_r /= height*width;
    eqm_g /= height*width;
    eqm_b /= height*width;
    eqm = (eqm_r + eqm_g + eqm_b) / 3;
    double psnr = 10 * log10(pow(d, 2) / eqm);
	return psnr;
}
