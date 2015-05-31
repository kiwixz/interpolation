/*
 * Copyright (c) 2015 kiwixz
 *
 * This file is part of interpolation.
 *
 * interpolation is free software : you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * interpolation is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with interpolation. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdio.h>
#include <opencv2/core/core_c.h>
#include <opencv2/highgui/highgui_c.h>

#define INFO(s, ...) \
  fprintf(stderr, " \x1b[32m"s "\x1b[0m\n", ## __VA_ARGS__)

#define ERROR(s, ...)                                      \
  fprintf(stderr, __FILE__ ":%d: \x1b[31;1m"s "\x1b[0m\n", \
          __LINE__, ## __VA_ARGS__)

#define CODEC CV_FOURCC('X', 'V', 'I', 'D')
static const double target = 60.0;

static void blend(IplImage *p, IplImage *n, float nfactor, IplImage *r)
{
  int   i;
  float pfactor;

  pfactor = 1 - nfactor;

  for (i = 0; i < r->imageSize; ++i)
    r->imageData[i] = (unsigned char)p->imageData[i] * pfactor
      + (unsigned char)n->imageData[i] * nfactor;
}

static int interpolate(double fps, CvVideoWriter *out,
                       IplImage *p, IplImage *n)
{
  static double late;

  int      i, times;
  IplImage *img;

  img = cvCreateImage(cvSize(p->width, p->height), p->depth, p->nChannels);
  if (!img)
    {
      ERROR("Failed to create container for interpolated frame");
      return -1;
    }

  late += target / fps;
  times = late;
  late -= times;

  for (i = 0; i < times; ++i)
    {
      blend(p, n, (float)i / times, img);
      cvWriteFrame(out, img);
    }

  cvReleaseImage(&img);

  return 0;
}

static void progressbar(int p)
{
  static int lastp;

  int i;

  if (p != lastp)
    {
      if (p)
        {
          printf("[");
          for (i = 0; i < p; ++i)
            printf("#");

          for (i = 100; i > p; --i)
            printf("-");

          printf("] %d%%\r", p);
        }
      else
        {
          for (i = 0; i < 107; ++i)
            printf(" ");
          printf("\r");
        }

      fflush(stdout);
      lastp = p;
    }
}

int main(int argc, char *argv[])
{
  double        fps;
  int           i, width, height, len;
  CvCapture     *in;
  CvVideoWriter *out;
  IplImage      *bufimg;

  if (argc != 3)
    {
      ERROR("Usage: interpolation source destination");
      return EXIT_FAILURE;
    }

  in = cvCaptureFromFile(argv[1]);
  if (!in)
    {
      ERROR("Failed to open for reading '%s'", argv[1]);
      return EXIT_FAILURE;
    }

  fps = cvGetCaptureProperty(in, CV_CAP_PROP_FPS);
  if (fps > target)
    {
      ERROR("Video already has a framerate of %.2f (target was %.2f)",
            fps, target);
      return EXIT_FAILURE;
    }

  width = cvGetCaptureProperty(in, CV_CAP_PROP_FRAME_WIDTH);
  height = cvGetCaptureProperty(in, CV_CAP_PROP_FRAME_HEIGHT);
  len = cvGetCaptureProperty(in, CV_CAP_PROP_FRAME_COUNT);

  out =
    cvCreateVideoWriter(argv[2], CODEC, target, cvSize(width, height), 1);
  if (!out)
    {
      ERROR("Failed to open for writing '%s'", argv[2]);
      return EXIT_FAILURE;
    }

  bufimg = cvQueryFrame(in);
  for (i = 1; i < len; ++i)
    {
      IplImage *img;

      img = cvCloneImage(bufimg);
      bufimg = cvQueryFrame(in);
      progressbar(100 * i / len);

      if (interpolate(fps, out, img, bufimg))
        return EXIT_FAILURE;

      cvReleaseImage(&img);
    }
  cvWriteFrame(out, bufimg);
  progressbar(0);

  cvReleaseVideoWriter(&out);
  cvReleaseCapture(&in);

  return EXIT_SUCCESS;
}
