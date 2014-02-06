code

/******************************************************************
 *Img2Video : read a sequence of images under a directory (you need to provide a begin and end number of these images, some images
              in between are missing is OK), output a video with smear region marked (in green color)
   Since the exe(64-bit) file was built with OpenCV 2.48 and VS2012, some dll files maybe missing.(I'll include the necessary dll files, 
   based on my knowledge, but I haven't tested it on computer without OpenCV).
 * The main function can run in the following 4 modes:
 * no input: Run with default setting. Read image sequence under "C://sample_drive//sample_drive//cam_2//" or " 
             ~/sample_driver/sample_driver/cam_5"(linux executalbe file), begin from 393408606.jpg, end with 393413167.jpg,
             4500 images used in calculating average gradient image.
 * one input: directory changed as the input
 * three input: directory path, begin image number, end image number
 * four input: directory path, begin image number, end image number, the number of image will be used in calculating average
   gradient image

During running, the program will show the average gradient images, images after morphology opening, estimated smear mask, and 
finally output a video of the original image sequence with green mask for smear region. 

All the output images and video will be wrote to your computer, the images are wrote to the same directory with the executalbe 
file, while the video will be wrote to the directory contains the original image sequence.

For each output window, you need press a key(any) to continue or stop the video.

 
