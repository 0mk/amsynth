/* amSynth
 * (c) 2001,2002 Nick Dowell
 **/

#include <math.h>
#include <stdlib.h>
#include <iostream>
#include "Oscillator.h"

Oscillator::Oscillator(int rate, float *buf)
{
    outBuffer = buf;
    pulseWidth = 0;
    rads = 0.0;
    waveform = 1;
    waveParam = 0;
    random = 0;
    random_count = 0;
    this->rate = rate;
    twopi_rate = TWO_PI / rate;
    syncParam = 0;
    sync = period = 0;
	input = 0;
}

Oscillator::~Oscillator()
{
}

void
Oscillator::setInput(FSource &fsource)
{
    input = &fsource;
}

void 
Oscillator::setWaveform(Parameter & param)
{
    waveParam = &param;
    param.addUpdateListener(*this);
    update();
}

void 
Oscillator::setSync(Parameter & param, Oscillator & osc)
{
    syncParam = &param;
    syncOsc = &osc;
    param.addUpdateListener(*this);
    update();
}

void 
Oscillator::setPulseWidth(FSource & source)
{
    pulseWidth = &source;
}

void 
Oscillator::reset()
{
    rads = 0.0;
}

void 
Oscillator::reset(int offset, int period)
{
	reset_offset = offset;
	reset_period = period;
}

void 
Oscillator::update()
{
    if (waveParam)
		waveform = (int) waveParam->getValue();
	sync = 0;
    if (syncParam)
		sync = (int) syncParam->getValue();
	if(sync==0){
		reset_period = 4096;
		reset_offset = 4096;
		if(syncOsc)
			syncOsc->reset( 4096, 4096 );
	}
}

float *
Oscillator::getNFData(int nFrames)
{
    // do we really need to track the frequency _every_ sample??
	if (input)
	{
	    inBuffer = input->getFData(nFrames);
		freq = inBuffer[0];
	}
    else
	{
	}
	sync_c = 0;
    sync_offset = nFrames + 1;
	
    reset_cd = reset_offset;
    
	if (pulseWidth)	mPulseWidth = pulseWidth->getFData(nFrames)[0];
	else		mPulseWidth = 0;

    // any decision statements are BAD in real-time code...
    switch (waveform) {
	case 0:
		doSine(outBuffer, nFrames);
		break;
	case 1:
		doSquare(outBuffer, nFrames);
		break;
	case 2:
		doSaw(outBuffer, nFrames);
		break;
	case 3:
		doNoise(outBuffer, nFrames);
		break;
	case 4:
		doRandom(outBuffer, nFrames);
		break;
	default:
		break;
	}
	if(sync)
		syncOsc->reset( sync_offset, (int)(rate/freq) );
    return outBuffer;
}

void
Oscillator::Process64Samples	(float *buffer, float freq_hz, float pw)
{
	freq = freq_hz;
	mPulseWidth = pw;
	
	sync_c = 0;
	sync_offset = 65;
	
	reset_cd = reset_offset;
	
	switch (waveform)
	{
	case Waveform_Sine:	doSine (buffer, 64);	break;
	case Waveform_Pulse:	doSquare (buffer, 64);	break;
	case Waveform_Saw:	doSaw (buffer, 64);	break;
	case Waveform_Noise:	doNoise (buffer, 64);	break;
	case Waveform_Random:	doRandom (buffer, 64);	break;
	default: break;
	}
	
	if (sync) syncOsc->reset (sync_offset, (int)(rate/freq));
}

void 
Oscillator::doSine(float *buffer, int nFrames)
{
    for (int i = 0; i < nFrames; i++) {
		buffer[i] = sin(rads += (twopi_rate * freq));
		//-- sync to other oscillator --
		if (reset_cd-- == 0){
			rads = 0.0;					// reset the oscillator
			reset_cd = reset_period-1;	// start counting down again
		}
		if ( sync_offset > nFrames)	// then we havent already found the offset
			if( rads > TWO_PI )			// then weve completed a circle
				sync_offset = i;		// remember the offset
	}
	rads = fmod((float)rads, (float)TWO_PI);			// overflows are bad!
}

float 
Oscillator::sqr(float foo)
{
    if ((fmod((float)foo, (float)TWO_PI)) < (mPulseWidth + 1) * PI)
	return 1.0;
    else
	return -1.0;
}

void 
Oscillator::doSquare(float *buffer, int nFrames)
{
    for (int i = 0; i < nFrames; i++) {
		buffer[i] = sqr(rads += (twopi_rate * freq));
		//-- sync to other oscillator --
		if (reset_cd-- == 0){
			rads = 0.0;					// reset the oscillator
			reset_cd = reset_period-1;	// start counting down again
		}
		if ( sync_offset > nFrames)	// then we havent already found the offset
			if( rads > TWO_PI )			// then weve completed a circle
				sync_offset = i;		// remember the offset
	}
    rads = fmod((float)rads, (float)TWO_PI);
}


float 
Oscillator::saw(float foo)
{
    foo = fmod((float)foo, (float)TWO_PI);
    register float t = (foo / (2 * PI));
    register float a = (mPulseWidth + 1) / 2;
    if (t < a / 2)
	return 2 * t / a;
    else if (t > (1 - (a / 2)))
	return (2 * t - 2) / a;
    else
	return (1 - 2 * t) / (1 - a);
//    return 1.0 - (fmod((float)foo, (float)TWO_PI) / PI);
}

void 
Oscillator::doSaw(float *buffer, int nFrames)
{
    for (int i = 0; i < nFrames; i++) {
		buffer[i] = saw(rads += (twopi_rate * freq));
		//-- sync to other oscillator --
		if (reset_cd-- == 0){
			rads = 0.0;					// reset the oscillator
			reset_cd = reset_period-1;	// start counting down again
		}
		if ( sync_offset > nFrames)	// then we havent already found the offset
			if( rads > TWO_PI )			// then weve completed a circle
				sync_offset = i;		// remember the offset
	}
    rads = fmod((float)rads, (float)TWO_PI);
}

void 
Oscillator::doRandom(float *buffer, int nFrames)
{
    register int period = (int) (rate / freq);
    for (int i = 0; i < nFrames; i++) {
	if (random_count > period) {
	    random_count = 0;
	    random = ((float)::random() / (RAND_MAX / 2)) - 1.0;
	}
	random_count++;
	buffer[i] = random;
    }
}

void 
Oscillator::doNoise(float *buffer, int nFrames)
{
    for (int i = 0; i < nFrames; i++)
	buffer[i] = ((float)::random() / (RAND_MAX / 2)) - 1.0;
}
