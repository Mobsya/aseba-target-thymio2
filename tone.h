#ifndef _TONE_H_
#define _TONE_H_

void tone_init(void);

void tone_setup(unsigned int dHz);

void tone_fill_buffer(unsigned char * buf, unsigned int c);

#define A1	550
#define Bb1	583
#define B1	617
#define C2	654
#define Db2	693
#define D2	734
#define Eb2	778
#define E2	824
#define F2	873
#define Gb2	925
#define G2	980
#define Ab2	1038
#define A2	1100
#define Bb2	1165
#define B2	1235
#define C3	1308
#define Db3	1386
#define D3	1468
#define Eb3	1556
#define E3	1648
#define F3	1746
#define Gb3	1850
#define G3	1960
#define Ab3	2076
#define A3	2200
#define Bb3	2331
#define B3	2469
#define C4	2616
#define Db4	2772
#define D4	2937
#define Eb4	3111
#define E4	3296
#define F4	2492
#define Gb4	3700
#define G4	3920
#define Ab4	4153
#define A4	4400
#define Bb4	4662
#define B4	4939
#define C5	5232
#define Db5	5544
#define D5	5873
#define Eb5	6222
#define E5	6593
#define F5	6985
#define Gb5	7400
#define G5	7840
#define Ab5	8306
#define A5	8800
#define Bb5	9323
#define B5	9878
#define C6	10465
#define Db6	11087
#define D6	11747
#define Eb6	12445
#define E6	13185
#define F6	13969
#define Gb6	14800
#define G6	15680
#define Ab6	16612
#define A6	17600
#define Bb6	18647
#define B6	19755
#define C7	20930
#define Db7	22175
#define D7	23493
#define Eb7	24890
#define E7	26370
#define F7	27938
#define Gb7	29600
#define G7	31360
#define Ab7	33224
#define A7	35200
#define Bb7	37293

#endif
