#!/usr/bin/env python3
#########################################################################################
#
#  Title: Blur Detection
#
#  Usage: ./BlurDetection.py -i "<inputfile(s)>" -o <outputfile>
#                 where <inputfile(s)> in quotes if using wildcard
#                       <outputfile> is a text file containing image name and blur value
#
#  Reference: https://www.pyimagesearch.com/2015/09/07/blur-detection-with-opencv
#
#  Author: Patrick de Perio (pdeperio@triumf.ca)
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
   try:
      opts, args = getopt.getopt(argv,"hi:o:",["input=","output="])
   except getopt.GetoptError:
      print ('test.py -i "<inputfile(s)>" -o <outputfile>')
      sys.exit(2)
   for opt, arg in opts:
      if opt == '-h':
         print ('test.py -i <inputfile>')
         sys.exit()
      elif opt in ("-i", "--input"):
         inputfile = arg
      elif opt in ("-o", "--output"):
         outputfile = arg
   print ('Input file(s) are: \n', inputfile)

   outfile = open(outputfile, "w")

   crop_x1 = 1500
   crop_x2 = 2500
   crop_y1 = 600
   crop_y2 = 1600
   #crop_x1 = 1600
   #crop_x2 = 2200
   #crop_y1 = 900
   #crop_y2 = 1400
   
   for filename in sorted(glob.glob(inputfile)):
        print (filename)
       
        img = cv2.imread(filename) ##read the image specified in the input

        img_denoise = cv2.fastNlMeansDenoisingColored(img,None,10,10,7,21)

        crop_img = img_denoise[crop_y1:crop_y2, crop_x1:crop_x2]

        
        #cv2.imshow("cropped", crop_img)
        #cv2.waitKey(0)

        #BlurVal = cv2.Laplacian(img, cv2.CV_64F).var()
        BlurVal = cv2.Laplacian(crop_img, cv2.CV_64F).var()
        print ('Blur = %.1f \n' % BlurVal)

        base = os.path.basename(filename)
        imageName = os.path.splitext(base)[0]
        imageName = re.sub("\D", "", imageName)

        outfile.write(imageName+" %.02f \n" % BlurVal)

   outfile.close()
   print('Output written:', outputfile)
   
if __name__ == "__main__":
   main(sys.argv[1:])
