#ifndef ADV_7611_H
#define ADV_7611_H


#define ADV_7611_V_LOCKED_SET_INTERRUPT_POS                    (0x1)
#define ADV_7611_V_LOCKED_SET_INTERRUPT_MASK                   (0x1)
#define ADV_7611_V_LOCKED_CLEAR_INTERRUPT_POS                  (0x1)
#define ADV_7611_V_LOCKED_CLEAR_INTERRUPT_MASK                 (0x1)

#define ADV_7611_DE_REGEN_LCK_SET_INTERRUPT_POS                (0x0)
#define ADV_7611_DE_REGEN_LCK_SET_INTERRUPT_MASK               (0x1)
#define ADV_7611_DE_REGEN_LCK_CLEAR_INTERRUPT_POS              (0x0)
#define ADV_7611_DE_REGEN_LCK_CLEAR_INTERRUPT_MASK             (0x1)

void ADV_7611_Initial(uint8_t index);
void ADV_7611_DumpOutEdidData(uint8_t index);
void ADV_7611_DumpOutDefaultSettings(uint8_t index);
void ADV_7611_GetVideoFormat(uint8_t index, uint16_t* widthPtr, uint16_t* hightPtr, uint8_t* framteratePtr);
void ADV_7611_GetAudioSampleRate(uint8_t index, uint32_t* sampleRate);
uint8_t ADV_7611_IrqHandler0(void);
uint8_t ADV_7611_IrqHandler1(void);

#endif

