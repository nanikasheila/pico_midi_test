#include <stdio.h>
#include "pico/stdlib.h"

#include "bsp/board.h"
#include "tusb.h"

#include "midi_processor.hpp"

MidiProcessor midi_processor;

void midi_task()
{
    if (tud_midi_available()) {
        uint8_t packet[4];
        uint32_t count = tud_midi_stream_read(packet, sizeof(packet));
        midi_processor.processMidiMessage(packet,count);
    }
}

void NoteOnCallback(uint8_t status,uint8_t data1,uint8_t data2)
{
    uint8_t channel = status & 0x0F;      // チャンネル番号 (0~15)
    printf("Note On: Channel %d, Note %d, Velocity %d\n", channel, data1, data2);
    board_led_on();
}
void NoteOffCallback(uint8_t status,uint8_t data1,uint8_t data2)
{
    uint8_t channel = status & 0x0F;      // チャンネル番号 (0~15)
    printf("Note Off: Channel %d, Note %d\n", channel, data1);
    board_led_off();
}

void PanCallback(uint8_t status,uint8_t data1,uint8_t data2)
{
    uint8_t channel = status & 0x0F;      // チャンネル番号 (0~15)
    printf("Pan: Channel %d, Value %d\n", channel, data2);
}


int main(void)
{
    // stdioの初期化
    stdio_init_all();
    // TinyUSBの初期化
    board_init();
    tusb_init();

    // 実行確認用オンボードLEDの点滅
    board_led_on();
    sleep_ms(500);
    board_led_off();
    sleep_ms(500);

    midi_processor.registerCallback(MidiCommandType::NoteOn,NoteOnCallback);
    midi_processor.registerCallback(MidiCommandType::NoteOff,NoteOffCallback);
    midi_processor.registerCallback(MidiCommandType::Pan,PanCallback);

    while (true)
    {
        tud_task();
        midi_task();
    }

    return 0;
}
