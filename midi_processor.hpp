#include <stdio.h>
#include <map>
#include "pico/stdlib.h"

using namespace std;

//----------------------------------------------------------------
// MIDIコマンド分類
//----------------------------------------------------------------
enum class MidiCommandType
{
    NoteOn,        // 0x90
    NoteOff,       // 0x80
    ControlChange, // 0xB0
    PitchBend,     // 0xE0
    System,        // 0xF0以上 (クロック, スタート/ストップ等含む)

    // ControlChange
    BankSelect_MSB,
    BankSelect_LSB,
    Modulation,
    Modulation_LSB,
    BreathController,
    BreathController_LSB,
    FootController,
    FootController_LSB,
    PortamentTime,
    PortamentTime_LSB,
    DataEntry,
    DataEntry_LSB,
    ChannelVolume,
    ChannelVolume_LSB,
    Balance,
    Balance_LSB,
    Pan,
    Pan_LSB,
    Expression,
    Expression_LSB,
    EffectControl1,
    EffectControl1_LSB,
    EffectControl2,
    EffectControl2_LSB,
    GeneralPurposeController1,
    GeneralPurposeController2,
    GeneralPurposeController3,
    GeneralPurposeController4,
    GeneralPurposeController5,
    GeneralPurposeController6,
    GeneralPurposeController7,
    GeneralPurposeController8,
    GeneralPurposeController1_LSB,
    GeneralPurposeController2_LSB,
    GeneralPurposeController3_LSB,
    GeneralPurposeController4_LSB,
    GeneralPurposeController5_LSB,
    GeneralPurposeController6_LSB,
    GeneralPurposeController7_LSB,
    GeneralPurposeController8_LSB,
    Hold1,
    Portament,
    Sostenuto,
    SoftPedal,
    LegatoFootswitch,
    Hold2,
    SoundVariation,
    HarmonicIntensity,
    ReleaseTime,
    AttackTime,
    Brightness,
    DecayTime,
    SustainLevel,
    Release,
    VibratoDepth,
    VibratoRate,
    VibratoDecay,
    PortamentoControl,
    Effect1Depth,
    Effect2Depth,
    Effect3Depth,
    Effect4Depth,
    Effect5Depth,
    DataIncrement,
    DataDecrement,
    NRPN_LSB,
    NRPN_MSB,
    RPN_LSB,
    RPN_MSB,
    AllSoundOff,
    ResetAllController,
    LocalControl,
    AllNoteOff,
    OmniOff,
    OmniOn,
    MonoOn,
    PolyOn,
    Undefine,

    // Syntem
    MIDITimecodeQuarterFrame,
    SongPositionPointer,
    SongSelect,
    TuneRequest,
    EndOfExclusive,
    TimingClock,
    Start,
    Continue,
    Stop,
    ActiveSensing,
    SystemReset,

    // Unknoen
    Unknown
};

//----------------------------------------------------------------
// コールバックの型
//   引数: (ch, data1, data2)
//   例: NoteOn => (ch, noteNumber, velocity)
//       ControlChange => (ch, ccNumber, ccValue)
//       System => 0xF8(クロック)などのステータスを data1 や data2 に入れる
//----------------------------------------------------------------
using MidiCallback = void (*)(uint8_t, uint8_t, uint8_t);

class MidiProcessor
{
private:
    // ステータスバイトから大分類の enum を返す
    MidiCommandType getMidiCommandType(uint8_t status, uint8_t data1, uint8_t data2)
    {
        // 0xF0 以上はシステム系 (リアルタイム, SysExなど)
        if (status >= 0xF0)
        {
            // return MidiCommandType::System;
            switch (data1)
            {
            case 0xF8: // MIDI Clock
                return MidiCommandType::TimingClock;
            case 0xFA: // Start
                return MidiCommandType::Start;
            case 0xFB: // Continue
                return MidiCommandType::Continue;
            case 0xFC: // Stop
                return MidiCommandType::Stop;
            case 0xFE: // アクティブ・センシング
                return MidiCommandType::ActiveSensing;
            case 0xFF: // システムリセット
                return MidiCommandType::SystemReset;
            default:
                printf("[Callback] System msg: 0x%02X (data2=%d)\n", status, data2);
                return MidiCommandType::Unknown;
            }
        }

        // 上位4bitがメッセージ種別、下位4bitがチャンネル
        uint8_t messageType = status & 0xF0;

        switch (messageType)
        {
        case 0x80: // ノート・オフ
            return MidiCommandType::NoteOff;
        case 0x90: // ノート・オン
            return MidiCommandType::NoteOn;
        case 0xB0: // コントロールチェンジ
            // return MidiCommandType::ControlChange;
            switch (data1)
            {
            case 0: // バンクセレクト MSB
                return MidiCommandType::BankSelect_MSB;
            case 1: // モジュレーション（1,01h）
                return MidiCommandType::Modulation;
            case 2: // ブレス・コントローラー（2,02h）
                return MidiCommandType::BreathController;
            case 3: // 未定義
                return MidiCommandType::Undefine;
            case 4: // フットコントローラ
                return MidiCommandType::FootController;
            case 5: // ポルタメント・タイム（5,05h）
                return MidiCommandType::PortamentTime;
            case 6: // データエントリー
                return MidiCommandType::DataEntry;
            case 7: // チャンネルボリューム（7,07h）
                return MidiCommandType::ChannelVolume;
            case 8: // バランス（8,08h）
                return MidiCommandType::Balance;
            case 9: // 未定義
                return MidiCommandType::Undefine;
            case 10: // パン（10,0Ah）
                return MidiCommandType::Pan;
            case 11: // エクスプレッション（11,0Bh）
                return MidiCommandType::Expression;
            case 12: // エフェクト・コントロール（12,0Ch）
                return MidiCommandType::EffectControl1;
            case 13: // エフェクト・コントロール（13,0Dh）
                return MidiCommandType::EffectControl2;
            case 14: // 未定義
            case 15: // 未定義
                return MidiCommandType::Undefine;
            case 16: // 汎用コントロ－ラ1
                return MidiCommandType::GeneralPurposeController1;
            case 17: // 汎用コントロ－ラ2
                return MidiCommandType::GeneralPurposeController2;
            case 18: // 汎用コントロ－ラ3
                return MidiCommandType::GeneralPurposeController3;
            case 19: // 汎用コントロ－ラ4
                return MidiCommandType::GeneralPurposeController4;
            case 20: // 未定義
            case 21: // 未定義
            case 22: // 未定義
            case 23: // 未定義
            case 24: // 未定義
            case 25: // 未定義
            case 26: // 未定義
            case 27: // 未定義
            case 28: // 未定義
            case 29: // 未定義
            case 30: // 未定義
            case 31: // 未定義
                return MidiCommandType::Undefine;
            case 32:
                return MidiCommandType::BankSelect_LSB;
            case 33:
                return MidiCommandType::Modulation_LSB;
            case 34:
                return MidiCommandType::BreathController_LSB;
            case 35: // 未定義
                return MidiCommandType::Undefine;
            case 36:
                return MidiCommandType::FootController_LSB;
            case 37:
                return MidiCommandType::PortamentTime_LSB;
            case 38: // データエントリー
                return MidiCommandType::DataEntry_LSB;
            case 39: // チャンネルボリューム（7,07h）
                return MidiCommandType::ChannelVolume_LSB;
            case 40: // バランス（8,08h）
                return MidiCommandType::Balance_LSB;
            case 41: // 未定義
                return MidiCommandType::Undefine;
            case 42: // パン（10,0Ah）
                return MidiCommandType::Pan_LSB;
            case 43: // エクスプレッション（11,0Bh）
                return MidiCommandType::Expression_LSB;
            case 44: // エフェクト・コントロール（12,0Ch）
                return MidiCommandType::EffectControl1_LSB;
            case 45: // エフェクト・コントロール（13,0Dh）
                return MidiCommandType::EffectControl2_LSB;
            case 46: // 未定義
            case 47: // 未定義
                return MidiCommandType::Undefine;
            case 48:
                return MidiCommandType::GeneralPurposeController1_LSB;
            case 49:
                return MidiCommandType::GeneralPurposeController2_LSB;
            case 50:
                return MidiCommandType::GeneralPurposeController3_LSB;
            case 51:
                return MidiCommandType::GeneralPurposeController4_LSB;
            case 52: // 未定義
            case 53: // 未定義
            case 54: // 未定義
            case 55: // 未定義
            case 56: // 未定義
            case 57: // 未定義
            case 58: // 未定義
            case 59: // 未定義
            case 60: // 未定義
            case 61: // 未定義
            case 62: // 未定義
            case 63: // 未定義
                return MidiCommandType::Undefine;
            case 64: // ホールド1（64,40h）
                return MidiCommandType::Hold1;
            case 65: // ポルタメント
                return MidiCommandType::Portament;
            case 66: // ソステヌートペダル
                return MidiCommandType::Sostenuto;
            case 67: // ソフトペダル
                return MidiCommandType::SoftPedal;
            case 68: // レガートフットスイッチ
                return MidiCommandType::LegatoFootswitch;
            case 69: // ホールド2
                return MidiCommandType::Hold2;
            case 70: // サウンドバリエーション
                return MidiCommandType::SoundVariation;
            case 71: // レゾナンス
                return MidiCommandType::HarmonicIntensity;
            case 72: // リリースタイム
                return MidiCommandType::ReleaseTime;
            case 73: // アタックタイム
                return MidiCommandType::AttackTime;
            case 74: // カットオフ
                return MidiCommandType::Brightness;
            case 75: // ディケイタイム
                return MidiCommandType::DecayTime;
            case 76: // ビブラートレート
                return MidiCommandType::VibratoRate;
            case 77: // ビブラートデプス
                return MidiCommandType::VibratoDepth;
            case 78: // ビブラートディケイ
                return MidiCommandType::VibratoDecay;
            case 79: // 未定義：予約済み
                return MidiCommandType::Undefine;
            case 80: // 汎用コントロ－ラ5
                return MidiCommandType::GeneralPurposeController5;
            case 81: // 汎用コントロ－ラ6
                return MidiCommandType::GeneralPurposeController6;
            case 82: // 汎用コントロ－ラ7
                return MidiCommandType::GeneralPurposeController7;
            case 83: // 汎用コントロ－ラ8
                return MidiCommandType::GeneralPurposeController8;
            case 84: // ポルタメントコントロール
                return MidiCommandType::PortamentoControl;
            case 85: // 未定義
            case 86: // 未定義
            case 87: // 未定義
            case 88: // 未定義
            case 89: // 未定義
            case 90: // 未定義
                return MidiCommandType::Undefine;
            case 91: // エフェクト1デプス（リバーブセンドレベル）
                return MidiCommandType::Effect1Depth;
            case 92: // エフェクト2デプス（トレモロデプス）
                return MidiCommandType::Effect2Depth;
            case 93: // エフェクト3デプス（コーラスセンドレベル）
                return MidiCommandType::Effect3Depth;
            case 94: // エフェクト4デプス（セレステデプス）
                return MidiCommandType::Effect4Depth;
            case 95: // エフェクト5デプス（フェイザーデプス）
                return MidiCommandType::Effect5Depth;
            case 96: // データインクリメント
                return MidiCommandType::DataIncrement;
            case 97: // データデクリメント
                return MidiCommandType::DataDecrement;
            case 98: // ノン・レジスタード・パラメータ・ナンバー
                return MidiCommandType::NRPN_LSB;
            case 99: // ノン・レジスタード・パラメータ・ナンバー
                return MidiCommandType::NRPN_MSB;
            case 100: // レジスタード・パラメータ・ナンバー
                return MidiCommandType::RPN_LSB;
            case 101: // レジスタード・パラメータ・ナンバー
                return MidiCommandType::RPN_MSB;
            case 102: // 未定義
            case 103: // 未定義
            case 104: // 未定義
            case 105: // 未定義
            case 106: // 未定義
            case 107: // 未定義
            case 108: // 未定義
            case 109: // 未定義
            case 110: // 未定義
            case 111: // 未定義
            case 112: // 未定義
            case 113: // 未定義
            case 114: // 未定義
            case 115: // 未定義
            case 116: // 未定義
            case 117: // 未定義
            case 118: // 未定義
            case 119: // 未定義
                return MidiCommandType::Undefine;
            case 120: // オールサウンドオフ
                return MidiCommandType::AllSoundOff;
            case 121: // リセットオールコントローラー
                return MidiCommandType::ResetAllController;
            case 122: // ローカルコントロール
                return MidiCommandType::LocalControl;
            case 123: // オールノートオフ
                return MidiCommandType::AllNoteOff;
            case 124: // オムニ・オフ
                return MidiCommandType::OmniOff;
            case 125: // オムニ・オン
                return MidiCommandType::OmniOn;
            case 126: // モノ・オン
                return MidiCommandType::MonoOn;
            case 127: // ポリ・オン
                return MidiCommandType::PolyOn;
            default:
                return MidiCommandType::Unknown;
            }
        case 0xE0:
            return MidiCommandType::PitchBend;
        default:
            return MidiCommandType::Unknown;
        }
    }

public:
    map<MidiCommandType, MidiCallback> callbacks_;

    void registerCallback(MidiCommandType type, MidiCallback cb)
    {
        callbacks_[type] = cb;
    }

    void processMidiMessage(const uint8_t *data, uint32_t size)
    {
        if (size < 3)
            return;

        // データ格納
        uint8_t status = data[0];
        uint8_t d1 = data[1];
        uint8_t d2 = data[2];

        // NoteOn で velocity=0 は NoteOff として扱う
        MidiCommandType cmdType = getMidiCommandType(status, d1, d2);
        if (cmdType == MidiCommandType::NoteOn && d2 == 0)
        {
            cmdType = MidiCommandType::NoteOff;
        }

        if (callbacks_.count(cmdType))
        {
            // 登録済の場合はコールバックを実行
            callbacks_[cmdType](status, d1, d2);
        }
        else
        {
            // 未登録の場合はログを出す
            printf("Unhandled MIDI: cmdType=%d, status=0x%02X, d1=%d, d2=%d\n",
                   static_cast<int>(cmdType), status, d1, d2);
        }
    }
};
