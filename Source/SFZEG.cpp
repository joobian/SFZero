#include "SFZEG.h"
#include "SFZDebug.h"


SFZEG::SFZEG()
{
}


void SFZEG::startNote(
	const SFZEGParameters* newParameters, float floatVelocity,
	float newSampleRate,
	const SFZEGParameters* velMod)
{
	parameters = *newParameters;
	if (velMod) {
		parameters.delay += floatVelocity * velMod->delay;
		parameters.attack += floatVelocity * velMod->attack;
		parameters.hold += floatVelocity * velMod->hold;
		parameters.decay += floatVelocity * velMod->decay;
		parameters.sustain += floatVelocity * velMod->sustain;
		if (parameters.sustain < 0.0)
			parameters.sustain = 0.0;
		else if (parameters.sustain > 100.0)
			parameters.sustain = 100.0;
		parameters.release += floatVelocity * velMod->release;
		}
	sampleRate = newSampleRate;

	startDelay();
}


void SFZEG::nextSegment()
{
	switch (segment) {
		case Delay:
			startAttack();
			break;

		case Attack:
			startHold();
			break;

		case Hold:
			startDecay();
			break;

		case Decay:
			startSustain();
			break;

		case Sustain:
			// Shouldn't be called.
			break;

		case Release:
		default:
			segment = Done;
			break;
		}
}


void SFZEG::noteOff()
{
	startRelease();
}


void SFZEG::startDelay()
{
	if (parameters.delay <= 0)
		startAttack();
	else {
		segment = Delay;
		level = 0.0;
		slope = 0.0;
		samplesUntilNextSegment = parameters.delay * sampleRate;
		}
}


void SFZEG::startAttack()
{
	if (parameters.attack <= 0)
		startHold();
	else {
		segment = Attack;
		level = parameters.start / 100.0;
		samplesUntilNextSegment = parameters.attack * sampleRate;
		slope = 1.0 / samplesUntilNextSegment;
		}
}


void SFZEG::startHold()
{
	if (parameters.hold <= 0)
		startDecay();
	else {
		segment = Hold;
		samplesUntilNextSegment = parameters.hold * sampleRate;
		level = 1.0;
		slope = 0.0;
		}
}


void SFZEG::startDecay()
{
	if (parameters.decay <= 0)
		startSustain();
	else {
		segment = Decay;
		samplesUntilNextSegment = parameters.decay * sampleRate;
		level = 1.0;
		slope = (parameters.sustain / 100.0 - 1.0) / samplesUntilNextSegment;
		}
}


void SFZEG::startSustain()
{
	if (parameters.sustain <= 0)
		startRelease();
	else {
		segment = Sustain;
		level = parameters.sustain / 100.0;
		slope = 0.0;
		samplesUntilNextSegment = 0x7FFFFFFF;
		}
}


void SFZEG::startRelease()
{
	float release = parameters.release;
	if (release <= 0) {
		// Enforce a short release, to prevent clicks.
		release = 0.01;
		}

	segment = Release;
	samplesUntilNextSegment = release * sampleRate;
	slope = -level / samplesUntilNextSegment;
}



