#ifndef _SMA_H_
#define _SMA_H_

void SMAGetData(char* sensor, char** output, int *len);
int RGB_LEDControl(int color);
int RGB_LEDStatus(void);

#endif//_SMA_H_
