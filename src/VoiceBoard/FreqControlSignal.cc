/* amSynth
 * (c) 2002 Nick Dowell
 **/

#include "FreqControlSignal.h"

FreqControlSignal::FreqControlSignal()
{
	lfo = 0;
	pitch_bend_source = key_pitch_source = 0;
	mod_amount = 1.0;
}

void
FreqControlSignal::setLFO( FSource & source )
{
	lfo = &source;
}

void
FreqControlSignal::setPitchBend( FSource & source )
{
	pitch_bend_source = &source;
}

void
FreqControlSignal::setKeyPitch( FSource & source )
{
	key_pitch_source = &source;
}

void
FreqControlSignal::setModAmount( Parameter & param )
{
	mod_amount_param = &param;
	// remember to use addUpdateListener() and NOT setUpdateListener!!
	mod_amount_param->addUpdateListener( *this );
}

void
FreqControlSignal::update()
{
	mod_amount = (mod_amount_param->getControlValue()/2.0)+0.5;
}
/*
float
FreqControlSignal::GetValue()
{
	return pitch_bend_source->getFData()[0] * key_pitch_source->getFData()[0] *
		( mod_amount*(*lfo->getFData()+1.0) + 1.0 - mod_amount );
}
*/
void
FreqControlSignal::process(int nFrames)
{
	lfo_buf = lfo->getFData(nFrames);
	buffer = pitch_bend_source->getFData(nFrames);
	register float pitch_bend = buffer[0];
	register float key_pitch = key_pitch_source->getFData(nFrames)[0];
	register int i;
	
	for( i=0; i<nFrames; i++ ) buffer[i] = pitch_bend*key_pitch *
		( mod_amount*(lfo_buf[i]+1.0) + 1.0 - mod_amount );
}
