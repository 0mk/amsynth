/* amSynth
 * (c) 2001,2002 Nick Dowell
 **/

#include "FValue.h"

FValue::FValue(float *buf)
{
	_value = 0.0;
	_buffer = buf;
	getvalfunc = &FValue::getIntVal;
}

FValue::~FValue()
{
  delete[]_buffer;
}

void
 FValue::setValue(float value)
{
  _value = value;
}

float FValue::getIntVal()
{
  return _value;
}

float FValue::getValue()
{
  return (this->*getvalfunc) ();
}

void FValue::setParameter(Parameter & param)
{
#ifdef _DEBUG
  cout << "<FValue> using Parameter: '" << param.
	getName() << "'" << endl;
#endif
  _param = &param;
  getvalfunc = &FValue::getParamVal;
}

float FValue::getParamVal()
{
  return _param->getControlValue();
}

float *FValue::getFData(int nFrames)
{
	float tmp = getValue();
	register int i;
	for (i = 0; i < nFrames; i++) 
		_buffer[i] = tmp;
	return _buffer;
}
