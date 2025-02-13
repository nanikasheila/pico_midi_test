#include <stdio.h>
#include "pico/stdlib.h"

#include "bsp/board.h"
#include "tusb.h"

#include "midi_processor.hpp"

MidiProcessor midi_processor;
/*
void process_midi_message(uint8_t *data, uint32_t count) {
    if (count < 3) return; // MIDI メッセージは通常 3 バイト

    uint8_t status = data[0]; // ステータスバイト (メッセージ種別 + チャンネル)
    uint8_t data1 = data[1];  // データ1（ノート番号、CC番号など）
    uint8_t data2 = data[2];  // データ2（ベロシティ、CC値など）

    uint8_t message_type = status & 0xF0; // 種別 (Note On, Off など)
    uint8_t channel = status & 0x0F;      // チャンネル番号 (0~15)

    switch (message_type) {
        case 0x90: // Note On
            if (data2 > 0) {
                printf("Note On: Channel %d, Note %d, Velocity %d\n", channel, data1, data2);
            } else {
                // ベロシティ 0 の Note On は Note Off 扱い
                printf("Note Off (via Note On): Channel %d, Note %d\n", channel, data1);
            }
            break;

        case 0x80: // Note Off
            printf("Note Off: Channel %d, Note %d\n", channel, data1);
            break;

        case 0xB0: // Control Change (CC)
            printf("Control Change: Channel %d, CC %d, Value %d\n", channel, data1, data2);
            if (data1 == 10) { // CC#10 (Pan)
                printf("Pan: %d\n", data2);
            }
            break;

        case 0xE0: // Pitch Bend
            {
                int pitch_bend = ((data2 << 7) | data1) - 8192; // 14bit 値 (-8192 ~ +8191)
                printf("Pitch Bend: Channel %d, Value %d\n", channel, pitch_bend);
            }
            break;

        case 0xC0: // Program Change
            printf("Program Change: Channel %d, Program %d\n", channel, data1);
            break;

        default:
            printf("Unknown MIDI message: %02X %02X %02X\n", status, data1, data2);
            break;
    }
}
*/

void midi_task()
{
    if (tud_midi_available()) {
        uint8_t packet[4];
        uint32_t count = tud_midi_stream_read(packet, sizeof(packet));

        midi_processor.processMidiMessage(packet,sizeof(packet));
    }
}

void NoteOnCallback(uint8_t status,uint8_t data1,uint8_t data2)
{
    uint8_t channel = status & 0x0F;      // チャンネル番号 (0~15)
    printf("Note On: Channel %d, Note %d, Velocity %d\n", channel, data1, data2);
    printf(":->");
}
void NoteOffCallback(uint8_t status,uint8_t data1,uint8_t /*data2*/)
{
    uint8_t channel = status & 0x0F;      // チャンネル番号 (0~15)
    printf("Note Off: Channel %d, Note %d\n", channel, data1);
}

int main(void)
{
    // stdioの初期化
    stdio_init_all();
    // TinyUSBの初期化
    board_init();

    //tud_disconnect();
    //sleep_ms(1000);
    tusb_init();
    board_led_on();
    sleep_ms(500);
    board_led_off();
    sleep_ms(500);

    midi_processor.registerCallback(MidiCommandType::NoteOn,NoteOnCallback);
    midi_processor.registerCallback(MidiCommandType::NoteOff,NoteOffCallback);

    while (true)
    {
        tud_task();
        midi_task();
    }

    return 0;
}
