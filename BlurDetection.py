#!/usr/bin/env python3.11
#########################################################################################
#
#  Title: Blur Detection
#
#  Usage: ./BlurDetection.py -i "<inputfile(s)>" -o <outputfile>
#                 where <inputfile(s)> in quotes if using wildcard
#                       <outputfile> is a text file containing image name and blur value
#
#  References:
#               https://www.pyimagesearch.com/2015/09/07/blur-detection-with-opencv
#               https://pyimagesearch.com/2014/09/29/finding-brightest-spot-image-using-python-opencv/
#
#  Author: Patrick de Perio (pdeperio@ipmu.jp)
#
#########################################################################################

import cv2
import os
import sys, getopt
import glob
import re

def main(argv):
   inputfile = ''
   outputfile = ''
   display = False
   try:
      opts, args = getopt.getopt(argv,"dhi:o:",["input=","output="])
   except getopt.GetoptError:
      print ('test.py -i "<inputfile(s)>" -o <outputfile> [-d]')
      sys.exit(2)
   for opt, arg in opts:
      if opt == '-h':
         print ('test.py -i <inputfile>')
         sys.exit()
      elif opt in ("-i", "--input"):
         inputfile = arg
      elif opt in ("-o", "--output"):
         outputfile = arg
      elif opt in ("-d", "--display"):
         display = True
   print ('Input file(s) are: \n')

   outfile = open(outputfile, "w")

   crop_x1 = 1500
   crop_x2 = 2500
   crop_y1 = 600
   crop_y2 = 1600
   crop_size = 500

   crop_watermark = 250

   for filename in glob.glob(inputfile):
        print (filename)
        
        img = cv2.imread(filename) ##read the image specified in the input

        # Remove camera noise 
        img = cv2.fastNlMeansDenoisingColored(img,None,21,21,21,21)

        # Gaussian blurring requires grayscale only
        gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

        # Remove watermark top bar
        gray = gray[crop_watermark:gray.shape[0], 0:gray.shape[1]]

        # Very broad blur to get approximate bright region, instead of sharp reflections
        gaus_radius = 501
        gray = cv2.GaussianBlur(gray, (gaus_radius, gaus_radius), 0)

        if (display):
           cv2.imshow("gray", gray)

        # Display brightest region
        (minVal, maxVal, minLoc, maxLoc) = cv2.minMaxLoc(gray)
        bright_pos = list(maxLoc)
        bright_pos[1] += crop_watermark  # translate back after watermark crop

        if (display):
           cv2.circle(img, bright_pos, 200, (255, 0, 0), 2)
           cv2.imshow("original", img)

        # Crop around bright region for blur calculation (dark regions will just add noise)
        crop_x1 = bright_pos[0] - crop_size
        crop_x2 = bright_pos[0] + crop_size
        crop_y1 = bright_pos[1] - crop_size
        crop_y2 = bright_pos[1] + crop_size

        # Be careful around edges: keep crop window same size
        if (crop_x1 < 0):
           crop_x1 = 0
           crop_x2 = crop_size*2
        elif (crop_x2 > img.shape[1]):
           crop_x2 = img.shape[1]
           crop_x1 = crop_x2 - crop_size*2

        if (crop_y1 < 0):
           crop_y1 = 0
           crop_y2 = crop_size*2
        elif (crop_y2 > img.shape[0]):
           crop_y2 = img.shape[0]
           crop_y1 = crop_y2 - crop_size*2

        #print (crop_x1, crop_x2, crop_y1, crop_y2)
        
        crop_img = img[crop_y1:crop_y2, crop_x1:crop_x2]

        if (display):
           cv2.imshow("crop", crop_img)
           cv2.waitKey(0)

        crop_img = cv2.Laplacian(img, cv2.CV_64F)
        BlurVal = crop_img.var()
        print ('Blur = %.1f \n' % BlurVal)

        base = os.path.basename(filename)
        imageName = os.path.splitext(base)[0]
        imageName = re.sub("\D", "", imageName)
        
        outfile.write(imageName+" %.02f \n" % BlurVal)

   outfile.close()
   print('Output written:', outputfile)
   
if __name__ == "__main__":
   main(sys.argv[1:])

