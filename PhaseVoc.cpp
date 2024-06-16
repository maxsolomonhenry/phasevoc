# define M_PI 3.14159265358979323846

#include "daisy_field.h"
#include "shy_fft.h"
#include "util.h"
#include "Smoother.h"

using namespace daisy;

DaisyField hw;
CpuLoadMeter meter;

const int kBufferSize = 2048; //8192;
int frameSize = 2048; //8192;
ShyFFT<float, kBufferSize> shyfft;

float knobValue = 0.0;
float knobDb = 0.0;
const float kLowerGainDb = -40.0;
const float kUpperGainDb = 0.0;
const float kThreshold = 0.0316f; // -30dB
Smoother smoother(0.001f);

const float kEps = 1e-4;
int nOverlap = 4;
int hopSize = frameSize / nOverlap;

float window[kBufferSize] = {0.0f};
float xNow[kBufferSize]  = {0.0f};
float xPast[kBufferSize] = {0.0f};
float yNow[kBufferSize] = {0.0f};

float X[kBufferSize] = {0.0f};
float lastX[kBufferSize] = {0.0f};
float Y[kBufferSize] = {kEps};
  
float readBuffer[kBufferSize] = {0.0f};
float writeBuffer[kBufferSize] = {0.0f};
int pBuffer = 0;

Switch button;
bool isButtonOn = false;

int hN = kBufferSize / 2;
float overlapMask = 0.0f;


void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{

	for (size_t i = 0; i < size; i++)
	{
		float gain = smoother.process();

		readBuffer[pBuffer] = in[0][i];
		out[0][i] = (isButtonOn ? writeBuffer[pBuffer] : in[0][i]) * gain;
		out[1][i] = (isButtonOn ? writeBuffer[pBuffer] : in[0][i]) * gain;

		pBuffer = (pBuffer + 1) % frameSize;

		if (pBuffer % hopSize == 0)
		{
			for (int n = 0; n < frameSize; n++)
			{
				xNow[n] = readBuffer[pBuffer];
				pBuffer = (pBuffer + 1) % frameSize;
			}

			for (int n = 0; n < frameSize; n++)
			{
				xNow[n] *= window[n];
				xPast[n] *= window[n];
			}

			shyfft.Direct(xNow, X);
			shyfft.Direct(xPast, lastX);

			// Y = X .* Y ./ (lastX + EPS) ./ (abs(Y ./ X) + EPS);
			for (int n = 0; n < frameSize; n++)
			{
				Y[n] = X[n] * Y[n] / (lastX[n] + kEps);

				if (n % 2 == 0)
					Y[n] /= (sqrt(pow(Y[n], 2) + pow(Y[n + 1], 2)) / sqrt(pow(X[n], 2) + pow(X[n + 1], 2)) + kEps);

				// Clip (only needed when algorithm is broken)...
				if (Y[n] > 1.0f)
					Y[n] = 1.0f;
				if (Y[n] < -1.0f)
					Y[n] = -1.0f;

			}

			shyfft.Inverse(Y, yNow);

			for (int n = 0; n < frameSize; n++)
			{
				yNow[n] *= window[n];
				yNow[n] /= frameSize;
			}

			for (int n = 0; n < frameSize; n++)
			{
				overlapMask = 0.0f;
				if (n < (frameSize - hopSize))
					overlapMask = 1.0f;

				writeBuffer[pBuffer] = writeBuffer[pBuffer] * overlapMask + yNow[n];
				pBuffer = (pBuffer + 1) % frameSize;
			}

			std::copy(xNow, xNow + frameSize, xPast);

		}

	}
}

int main(void)
{

	hw.Init();
	hw.SetAudioBlockSize(4096); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_8KHZ);
	hw.StartAdc();

	hw.display.Fill(false);
	hw.display.Update();

	meter.Init(hw.AudioSampleRate(), hw.AudioBlockSize());
	shyfft.Init();
	hw.seed.StartLog();

	for (int i = 0; i < frameSize; i++)
	{
		window[i] = pow(cos(M_PI * (float(i) - float(hN)) / frameSize), 2.0);
	}

	for (int i = 0; i < kBufferSize; i++)
	{
		xNow[i] = 0.0f;
		xPast[i] = 0.0f;
		yNow[i] = 0.0f;

		X[i] = 0.0f;
		lastX[i] = 0.0f;
		Y[i] = kEps;
		
		readBuffer[i] = 0.0f;
		writeBuffer[i] = 0.0f;
	}

	hw.StartAudio(AudioCallback);

	while(1) 
	{
		hw.ProcessAllControls();

		hw.led_driver.SetLed(DaisyField::LED_KEY_A8, float(isButtonOn));
		hw.led_driver.SwapBuffersAndTransmit();
		
		if (hw.KeyboardRisingEdge(7))
			isButtonOn = true;

		if (hw.KeyboardFallingEdge(7))
			isButtonOn = false;

		knobValue = hw.GetKnobValue(0);
		knobValue = knobValue > 0.999 ? 1.0 : knobValue;

		knobDb = extend(knobValue, kLowerGainDb, kUpperGainDb);
		smoother.setTarget(db2mag(knobDb));

		hw.display.Fill(false);
		hw.display.SetCursor(4, 16);
		hw.display.WriteString("Gain:", Font_6x8, true);
		hw.display.SetCursor(4, 32);

		FixedCapStr<16> str("");

		str.AppendFloat(mag2db(smoother.getValue() > 0.999 ? 1.0 : smoother.getValue()));
		str.Append(" dB");

		if (isButtonOn)
			str.Append("!");

		hw.display.WriteString(str, Font_11x18, true);

		hw.display.Update();

		// System::Delay(1000);

		// FixedCapStr<50> log;
		// log.Append("CPU -- min: ");
		// log.AppendFloat(meter.GetMinCpuLoad());
		// log.Append(", avg: ");
		// log.AppendFloat(meter.GetAvgCpuLoad());
		// log.Append(" max: ");
		// log.AppendFloat(meter.GetMaxCpuLoad());
		// hw.seed.PrintLine(log);

		// meter.Reset();
	}
}
