#ifndef __CODEC_H__
#define __CODEC_H__

extern "C"
{

//	Note that the codec will fail if it is not receiving
//	any clock from the si5351!
//	"vfo" must be initialized before calling this.
int initCodec(void);	//	returns true if succeeds

//	These all use normalized values 0-1.0 for "percent"
float getHpVol(void) ;
void setHpVol(float percent);
void muteHpVol(bool muteOn);
float getLoVol(void);
void setLoVol(float percent);
float getMicVol(void);
void setMicVol(float percent);
float getADCVol(void);
void setADCVol(float percent);
float getDACVol(void);
void setDACVol(float percent);

float getAGCHang(void);
void setAGCHang(float percent);
float getAGCAttack(void);
void setAGCAttack(float percent);
float getAGCLevel(void);
void setAGCLevel(float percent);

//	IMPORTANT: REMEMBER THAT AGC ONLY WORKS WITH INTERNAL AUDIO OUTPUT TO HEADSET
void enableAGC(bool state);

enum cAudioMode { NORMAL_AUDIO, DIGITAL_AUDIO, CW_AUDIO };
int enableTXAudio(bool txon, cAudioMode mode);
}

#endif
