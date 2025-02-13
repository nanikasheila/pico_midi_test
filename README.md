# Raspberry Pi PicoでUSB-MIDI受信

RaspberryPi PicoでUSB-MIDIを受信し、コマンドに応じて任意の関数を実行するコードです。

## 使い方

`MidiProcessor`を宣言して、

```c++
MidiProcessor midi_processor;
```

`registerCallback()`を使用し、
どのコマンドを受信したときに、どの関数を実行するかを登録する。
対応しているコマンドはMidiCommandTypeにまとめられている。

```c++
midi_processor.registerCallback(MidiCommandType::NoteOn,NoteOnCallback);
```

MIDIパケットを受信したら、
`processMidiMessage()`を実行してやる。

```c++
void midi_task()
{
    if (tud_midi_available()) {
        uint8_t packet[4];
        uint32_t count = tud_midi_stream_read(packet, sizeof(packet));
        midi_processor.processMidiMessage(packet,count);
    }
}
```

登録したコマンドとコールバック関数の組み合わせがあれば、実行されます。

※RP2350でテストしていますが、RP2040でも同じはずです。

tusb_config.h ,usb_descriptors.cはTinyUSBのExampleを参照。
ただし、下記だけ追記してます。（これがないとPC側でUSBデバイスとして認識しなかった）

```h
//--------------------------------------------------------------------
// DEVICE CONFIGURATION
//--------------------------------------------------------------------
#define CFG_TUSB_MCU                OPT_MCU_RP2040
#define CFG_TUSB_RHPORT0_MODE       OPT_MODE_DEVICE  // USB デバイスモードに設定

```

## 参考

[MIDI1.0 規格書](https://amei.or.jp/midistandardcommittee/MIDI1.0.pdf)

[tinyusb/examples/device/midi_test at master · hathach/tinyusb](https://github.com/hathach/tinyusb/tree/master/examples/device/midi_test)
