/* amSynth
 * (c) 2001-2003 Nick Dowell
 */

#include "main.h"
#include "../config.h"

#include <iostream>
#include <unistd.h>
#include <sndfile.h>
#include <time.h>

int main( int argc, char *argv[] )
{
	//
	// test parameters
	// 
	int time_seconds = 60;  // seconds of audio to generate
        int num_voices = 10;    // number of simultaneous voices to generate


	
	// set default parameters
	config.audio_driver = "auto";
	config.midi_driver = "auto";
	config.oss_midi_device = "/dev/midi";
	config.midi_channel = 0;
	config.oss_audio_device = "/dev/dsp";
	config.alsa_audio_device = "default";
	config.sample_rate = 44100;
	config.channels = 2;
	config.buffer_size = BUF_SIZE;
	config.polyphony = num_voices;
	
	presetController = new PresetController();
	
	
	vau = new VoiceAllocationUnit( config ); //after were sure of sample_rate
	presetController->loadPresets();
	
	vau->setPreset( presetController->getCurrentPreset() );
	
	presetController->selectPreset( 1 );
	presetController->selectPreset( 0 );

	//
	// prepare sndfile for .wav output
	// 
	SNDFILE	*sndfile;
	SF_INFO	sf_info;

	// set format etc. for output file
	sf_info.samplerate = config.sample_rate;
	sf_info.channels = config.channels;
	sf_info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;

	// open file for output:
	sndfile = sf_open( "ptest-out.wav", SFM_WRITE, &sf_info );
	// specify that floating point data is normalised (between -1.0 and 1.0)
	sf_command( sndfile, SFC_SET_NORM_FLOAT, NULL, SF_TRUE );
	
	
	//
	// now run the test.
	//
	
	long i=0;
	long total_calls = config.sample_rate * time_seconds / BUF_SIZE;
	float *buffer;
	
	// trigger off some notes for amSynth to render.
	for (int v=0; v<num_voices; v++)
	{
		vau->noteOn( 60+v, 127 );
	}
	
	//
	// now we need to get the time at the start of the test
	//
	clock_t clocks_before = clock();
	
	while (i<total_calls)
	{
		buffer = vau->getNFData();
		sf_writef_float( sndfile, buffer, BUF_SIZE );
		i++;
	}
	
	//
	// get the time at the end of test execution, and find the time elapsed
	// 
	clock_t clocks_elapsed = clock() - clocks_before;

	int ms_audio = time_seconds * 1000 * num_voices;
	int ms_elapsed = clocks_elapsed*1000 / CLOCKS_PER_SEC;

	std::cout << "generating " << num_voices << " voices of audio for " << 
		time_seconds << " seconds took " << ms_elapsed << "ms\n";
	std::cout << "***** performance index = " << 
		((float)ms_audio/(float)ms_elapsed) << " *****" << std::endl;
	
	// dont forget to close the output file, else it wont be written!
	sf_close( sndfile );
	
	delete presetController;
	delete vau;
	return 0;
}