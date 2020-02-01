#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#define LED 13

#define MAX_POLY 16



//Voices

struct Voice{
  AudioSynthWaveform osc;
  AudioEffectEnvelope env;
  byte note = 0;
  byte velocity = 0;
};

Voice voices[MAX_POLY];
AudioMixer4 fourChannelMixer[MAX_POLY/4];
AudioMixer4 masterMixer;
AudioOutputAnalog dac;

AudioConnection          patchOscToEnv0(voices[0].osc, voices[0].env);
AudioConnection          patchOscToEnv1(voices[1].osc, voices[1].env);
AudioConnection          patchOscToEnv2(voices[2].osc, voices[2].env);
AudioConnection          patchOscToEnv3(voices[3].osc, voices[3].env);
AudioConnection          patchOscToEnv4(voices[4].osc, voices[4].env);
AudioConnection          patchOscToEnv5(voices[5].osc, voices[5].env);
AudioConnection          patchOscToEnv6(voices[6].osc, voices[6].env);
AudioConnection          patchOscToEnv7(voices[7].osc, voices[7].env);
AudioConnection          patchOscToEnv8(voices[8].osc, voices[8].env);
AudioConnection          patchOscToEnv9(voices[9].osc, voices[9].env);
AudioConnection          patchOscToEnv10(voices[10].osc, voices[10].env);
AudioConnection          patchOscToEnv11(voices[11].osc, voices[11].env);
AudioConnection          patchOscToEnv12(voices[12].osc, voices[12].env);
AudioConnection          patchOscToEnv13(voices[13].osc, voices[13].env);
AudioConnection          patchOscToEnv14(voices[14].osc, voices[14].env);
AudioConnection          patchOscToEnv15(voices[15].osc, voices[15].env);

AudioConnection          patchEnvToMix0(voices[0].env, 0, fourChannelMixer[0], 0);
AudioConnection          patchEnvToMix1(voices[1].env, 0, fourChannelMixer[0], 1);
AudioConnection          patchEnvToMix2(voices[2].env, 0, fourChannelMixer[0], 2);
AudioConnection          patchEnvToMix3(voices[3].env, 0, fourChannelMixer[0], 3);
AudioConnection          patchEnvToMix4(voices[4].env, 0, fourChannelMixer[1], 0);
AudioConnection          patchEnvToMix5(voices[5].env, 0, fourChannelMixer[1], 1);
AudioConnection          patchEnvToMix6(voices[6].env, 0, fourChannelMixer[1], 2);
AudioConnection          patchEnvToMix7(voices[7].env, 0, fourChannelMixer[1], 3);
AudioConnection          patchEnvToMix8(voices[8].env, 0, fourChannelMixer[2], 0);
AudioConnection          patchEnvToMix9(voices[9].env, 0, fourChannelMixer[2], 1);
AudioConnection          patchEnvToMix10(voices[10].env, 0, fourChannelMixer[2], 2);
AudioConnection          patchEnvToMix11(voices[11].env, 0, fourChannelMixer[2], 3);
AudioConnection          patchEnvToMix12(voices[12].env, 0, fourChannelMixer[3], 0);
AudioConnection          patchEnvToMix13(voices[13].env, 0, fourChannelMixer[3], 1);
AudioConnection          patchEnvToMix14(voices[14].env, 0, fourChannelMixer[3], 2);
AudioConnection          patchEnvToMix15(voices[15].env, 0, fourChannelMixer[3], 3);

AudioConnection          patchMixToMaster0(fourChannelMixer[0], 0, masterMixer, 0);
AudioConnection          patchMixToMaster1(fourChannelMixer[1], 0, masterMixer, 1);
AudioConnection          patchMixToMaster2(fourChannelMixer[2], 0, masterMixer, 2);
AudioConnection          patchMixToMaster3(fourChannelMixer[3], 0, masterMixer, 3);

AudioConnection          patchMasterToDAC(masterMixer, 0, dac, 0);





float noteToFrequency(byte note) {
      return (float) 440.0 * powf(2.0, (float)(note - 69) * 0.08333333);
}
float velocityToLevel(byte velocity) {
      return (float) velocity/255;
}

void setup() {
  pinMode(LED, OUTPUT);
  
  AudioMemory(20);
  
  usbMIDI.setHandleNoteOff(HandleNoteOff);
  usbMIDI.setHandleNoteOn(HandleNoteOn);


  for(int i = 0 ; i < MAX_POLY/4 ; i++){
    for(int j = 0 ; j < MAX_POLY/4 ; j++){
      fourChannelMixer[i].gain(j, 0.25);
    }
    masterMixer.gain(i, 1);
  }

  for (int i = 0; i < MAX_POLY; i++) {
    voices[i].env.attack(0);
    voices[i].env.decay(0);
    voices[i].env.sustain(1);
    voices[i].env.release(500);
    voices[i].osc.begin(WAVEFORM_TRIANGLE);
  }

}

void HandleNoteOn(byte channel, byte note, byte velocity) {
  if (velocity > 0) {

    int activeVoice = 0;
    int voiceToSteal = 0;
    int lowestVelocity = 128;

    for (int i = 0 ; i < MAX_POLY; i++) {
      if(!voices[i].env.isActive()){
        voices[i].env.noteOff();
        voices[i].osc.frequency(noteToFrequency(note));
        voices[i].osc.amplitude(velocityToLevel(velocity));
        voices[i].env.noteOn();
        voices[i].note = note;
        voices[i].velocity = velocity;
        break;
      }else{
        activeVoice++;
        if(lowestVelocity >= voices[i].velocity){
          lowestVelocity = voices[i].velocity;
          voiceToSteal = i;
        }
      }
    }

    if(activeVoice == MAX_POLY){
        voices[voiceToSteal].env.noteOff();
        voices[voiceToSteal].osc.frequency(noteToFrequency(note));
        voices[voiceToSteal].osc.amplitude(velocityToLevel(velocity));
        voices[voiceToSteal].env.noteOn();
        voices[voiceToSteal].note = note;
        voices[voiceToSteal].velocity = velocity;
    }
    digitalWrite(LED, HIGH);

  } else {
//    MIDI.sendNoteOff(note, velocity, channel);
  }
}


void HandleNoteOff(byte channel, byte note, byte velocity) {
  byte handsOffChecker = 0;
  for (int i = 0; i < MAX_POLY; i++) {
    if (note == voices[i].note) {
      voices[i].env.noteOff();
      voices[i].note = 0;
      voices[i].velocity = 0;
    }
    handsOffChecker += voices[i].note;
  }

  if (handsOffChecker == 0) {
    digitalWrite(LED, LOW);
  }
}




void loop() {
  usbMIDI.read();
}
