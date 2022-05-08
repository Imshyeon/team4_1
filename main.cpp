#include "./include/camLib.hpp"

int main(int argc, char* argv[])
{
  // Create a camera instance...
  Camera cam1;
  int number_of_changes = 0;
  const int frameBuffer = 50;	        // Frame buffer around motion ...
  const int lengthThreshold = 5;	// How many frames in a sequence need to show motion...
	
  // if more than 'there_is_motion' pixels are changed, we say there is motion...
  // and store the image...
  int there_is_motion = 5;
  int frameCounter  = 0;        // Counter for frames...
  int frameSequence = 0;	// Count sequence of frames...
  int framesTotal   = 0;	// Total number fa frames containing motion -- for saving...
  int saveVid       = 0;	// Flag to save keep video if changes found...
  vector*frameStack;	        // Buffer to store frames before saving to SD card...
  Mat frame;			// Captured single frames...
  int framesize;		// size of a frame...
	
  // Create file name for detected motion...
  time_t seconds;
  struct tm * timeinfo;
  char detectionfile[80];
	
  // Allocate memory for the frames to store and start the camera...
  frameStack = new vector [frameBuffer*sizeof(cam1.captureVideo())];		
  if(frameStack == NULL) {
	cerr << " Could not allocate enough memory..." << endl;
	return 0;
  }
	
  // Start endless loop to capture frames...
  // This endless loop is stopped by user pressing the ESC key...
  // Generate new file name with a time-stamp right after the sequence that was captures
  while(1){
       // Display the resulting frames...
       frame = cam1.captureVideo();			// Capture a frame from the live stream of camera...
       framesize = sizeof(frame);
		     
       // 'frameBuffer' times frames are the min amount of frames being captured per SEQUENCE...
       if(frameCounter < frameBuffer) { frameCounter++; // Count frames... // This line puts frame-by-frame on a stack...: frameStack->push_back(frame);	        // Put new frame on stack on the computer's RAM...
		framesTotal++;
		// Only if there are more than 'there_is_motion' in the image
		// is it accepted that there is motion..
		number_of_changes = cam1.detectMotion();
		if(number_of_changes >= there_is_motion)
		{
			cout << "...Motion Detected... number_of_changes= " << number_of_changes << " ... FramesTotal = " << framesTotal << endl; frameSequence++; // Need a minimum amount of frames in a sequence showing motion... // Was motion detected over multiple frames...? if(frameSequence > lengthThreshold)  {
				saveVid = 1;		// Set flag to keep this video, as motion was detected
				frameCounter = 0;	// Reset the frame-counter, so that more frames are captured after the motion detection...
			}			
		} else{					// If not enough motion detected, then reset the counter...
			// Reset...
			frameSequence = 0;
		}
		   
	}
	else if(saveVid == 1)  {			// frameCounter >= frameBuffer and motion was detected (Flag 'saveVid' set)...
		// Reset the parameters for the next SEQUENCE...
		saveVid = 0;
		frameCounter = 0;
		frameSequence = 0;
		cout <<"......Save Video ....." << endl;			
		// Take frame-by-frame off the stack and save to avi video file on the permanent memory (e.g. SD-card)...
		cout << "Frames of the Video = " << ((frameStack->size())-1) << endl;
			
		// Get the current time...
		time (&seconds);
		timeinfo = localtime (&seconds);
		strftime(detectionfile, 80, "./detections/%d%h%Y_%H%M%S.avi", timeinfo);
		cam1.openVideoFile(detectionfile);
			
		for(unsigned i=0; i<((frameStack->size())-1); i++){
			frame = frameStack->at(i);
			cam1.saveVideo(frame);		
		}
		cout << "\n" << endl; // Release (close) the writer... cam1.closeVideo(); // Cleasr the stack, ready for the next SEQUENCE... frameStack->clear();
		framesTotal = 0;

	}
	else  {						// In case no motion was detected, discard the images collected on the stack...
		frameCounter = 0;
		frameSequence = 0;
		saveVid = 0;
		cout <<"----- Nothing to save -------" << endl; frameStack->clear();
	}
		
	if (waitKey(30) == 27) 		               //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
	{
		cout << "esc key is pressed by user" << endl; frameStack->clear();
                break; 
	}
  }

  return 0;
}
